#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ShaderUtil.h"
#include "Utils.h"
#include <iostream>
#include "Planet.h"
#include <cmath>
#include <queue>
#include <optional>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback_camera_swipe(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to theta right so we initially rotate a bit to theta left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 90.0f;

double thetaPlace = 0.0f;
double phiPlace = 0.0f;
bool placed = false;

bool rightMouseButtonPressed = false;

double lastTimePlaced = 0.0;

double swipeCameraRadius = 15.0;
double xAngleValue = 0.0;
double yAngleValue = 0.0;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


glm::vec2 sphericalToTexture(double theta, double phi) {
    double s = theta / glm::pi<float>();
    double t = phi / (2 * glm::pi<float>());
    return glm::vec2(t, s);
}

void sphericalVectorToAngularPosition(glm::vec3 point, double& theta, double& phi) {
    float x = point.x;
    float y = point.y;
    float z = point.z;

    float r = std::sqrt(x * x + y * y + z * z);

    // Azimuth (theta)
    theta = std::atan2(y, x);
    if (theta < 0) theta += 2.0f * glm::pi<float>();  // Ensure theta is in [0, 2*pi]

    // Elevation (phi)
    phi = std::acos(z / r);  // acos returns value in [0, pi]
    phi = std::fmax(0.0f, phi);    // Clamp to avoid rare negative results
}

glm::vec3 GetMouseWorldPosition(float mouseX, float mouseY,
    glm::mat4 projection, glm::mat4 view,
    glm::vec2 screenSize) {
    // Step 1: Normalize mouse coordinates to [-1, 1]
    float x = (2.0f * mouseX) / screenSize.x - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenSize.y;  // Invert Y axis
    float z = 1.0f;  // Assume the mouse is at the far plane initially

    glm::vec3 ndcPos = glm::vec3(x, y, z);

    // Step 2: Create homogeneous clip coordinates
    glm::vec4 clipCoords = glm::vec4(ndcPos.x, ndcPos.y, -1.0f, 1.0f);

    // Step 3: Unproject by multiplying with the inverse of the combined matrix
    glm::mat4 invViewProj = glm::inverse(projection * view);
    glm::vec4 worldPos = invViewProj * clipCoords;

    // Step 4: Perspective divide to get actual world position
    worldPos /= worldPos.w;

    return glm::vec3(worldPos);
}


std::optional<glm::vec3> mousePositionToThePlanet(GLFWwindow* window, float planetRadius, glm::mat4 projection, glm::mat4 view)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    glm::vec3 rayOrigin = cameraPos;
    glm::vec3 mousePosition = GetMouseWorldPosition(mouseX, mouseY, projection, view, glm::vec2(SCR_WIDTH, SCR_HEIGHT));
    glm::vec3 rayDirection = glm::normalize(mousePosition-rayOrigin);

    glm::vec3 sphereCenter = glm::vec3(0.0f, 0.0f, 0.0f); // Sphere center position
    glm::vec3 startPosition = rayOrigin - sphereCenter;

    float a = glm::dot(rayDirection, rayDirection);
    float b = 2.0f * glm::dot(startPosition, rayDirection);
    float c = glm::dot(startPosition, startPosition) - planetRadius * planetRadius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
        return {};

    float distsqrt = sqrt(discriminant);
    float q;

    if (b < 0)
        q = (-b - distsqrt) / 2.0;
    else
        q = (-b + distsqrt) / 2.0;

    float t0 = q / a;
    float t1 = c / q;

    //if t0 > t1 then swap them.
    if (t0 > t1)
    {
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    //the ray missed the sphere
    if (t1 < 0)
        return {};
    //if t0<0 intersection is at t1
    if (t0 < 0)
        return cameraPos + rayDirection * t1;
    return cameraPos + rayDirection * t0;
}

std::vector<glm::vec2> colorChangePolygonPoints;
void TrackMousePolygon(GLFWwindow* window, float planetRadius, glm::mat4 projection, glm::mat4 view)
{
    auto position3DSphere = mousePositionToThePlanet(window, planetRadius, projection, view);
    if (!position3DSphere.has_value())
    {
        return;
    }
    double theta, phi;
    sphericalVectorToAngularPosition(*position3DSphere, theta, phi);
    glm::vec2 colorChangePolygonPoint = sphericalToTexture(theta, phi);
    colorChangePolygonPoints.push_back(colorChangePolygonPoint);
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
   
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback_camera_swipe);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);


    ShaderUtil ourShader(
        (Utils::Paths::ProjDir + "assets\\shaders\\cube_vertex.glsl").c_str(),
        (Utils::Paths::ProjDir + "assets\\shaders\\cube_fragment.glsl").c_str()
    );

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // world space colorChangePolygonPoints of our cubes
    std::vector <glm::vec3> cubePositions;
    std::vector <glm::vec3> transformedPositions(3);
    std::vector <glm::vec2> sphereTextureCoords;

    srand(time(NULL));



    unsigned int cubeVBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Utils::Render::CubeVertices), Utils::Render::CubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // load and create a texture 
    // -------------------------
    unsigned int texture1, texture2, textureEarthWhiteBlack;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on theta y-axis.
    unsigned char* data = stbi_load(
        (Utils::Paths::ProjDir + "assets/textures/container.jpg").c_str(), &width, &height, &nrChannels, 0
    );

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    // texture 2
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load((Utils::Paths::ProjDir + "assets/textures/awesomeface.png").c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that theta awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL theta data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    Planet p(10.0f);
    p.GenerateSphere(1.0f, 72, 18*2, p.mVerts, p.mIndices);
    p.SetupRenderData();


    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        ourShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        cameraPos = glm::vec3(
            cos(glm::radians(xAngleValue)) * cos(glm::radians(yAngleValue)),
            sin(glm::radians(yAngleValue)),
            sin(glm::radians(xAngleValue)) * cos(glm::radians(yAngleValue))
        );

        cameraPos *= swipeCameraRadius;

        cameraFront = -glm::normalize(cameraPos);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("view", view);


        for(int i = 0; i < cubePositions.size(); i++)
        {
            glBindVertexArray(cubeVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);

            ourShader.use();

            glm::mat4 model = glm::mat4(1.0f);
            glm::vec3 position = glm::vec3(1.0f, 0.0f, 0.0f);
            position *= p.mRadius;

            //model = glm::rotate(model, 90.0f, glm::vec3(1, 1, 1));
            model = glm::translate(model, cubePositions[i]);
            model = glm::scale(model, glm::vec3(0.2, 0.5, 0.2));

            ourShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        if (rightMouseButtonPressed)
        {
            TrackMousePolygon(window, p.mRadius, projection, view);
        }
        else
        {
            if (!colorChangePolygonPoints.empty())
            {
                p.TryToCreateFloodFillMapTO_DELETE(p.mLandMassImgData, p.mStatesImgData, colorChangePolygonPoints, glm::vec3(129));
            }
            colorChangePolygonPoints.clear();
        }


        if (placed && currentFrame - lastTimePlaced > 0.5f)
        {
            glm::vec3 position(
                p.mRadius * glm::sin(thetaPlace) * glm::cos(phiPlace),
                p.mRadius * glm::sin(thetaPlace) * glm::sin(phiPlace),
                p.mRadius * glm::cos(thetaPlace)
            );
            cubePositions.push_back(position);
            sphereTextureCoords.push_back(sphericalToTexture(thetaPlace, phiPlace));
            placed = false;
            lastTimePlaced = currentFrame;
        }
        else
        {
            placed = false;
        }

        {
            glBindVertexArray(cubeVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);

            ourShader.use();

            //glm::mat4 model = glm::mat4(1.0f);
            //glm::vec3 position(
            //    p.mRadius * glm::sin(thetaPlace) * glm::cos(phiPlace),
            //    p.mRadius * glm::sin(thetaPlace) * glm::sin(phiPlace),
            //    p.mRadius * glm::cos(thetaPlace)
            //);

            //auto val = mousePositionToThePlanet(window, p.mRadius, projection, view);
            //if (val.has_value())
            //{
            //    model = glm::translate(model, *val);
            //    model = glm::scale(model, glm::vec3(0.2, 0.5, 0.2));

            //    ourShader.setMat4("model", model);
            //    glDrawArrays(GL_TRIANGLES, 0, 36);
            //}
        }

        {
            glBindVertexArray(p.VAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, p.texture);

            p.planetShader.use();
            p.planetShader.setMat4("projection", projection);
            p.planetShader.setMat4("view", view);
            p.planetShader.setInt("redPlanetMap", 0);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(p.mRadius));
           // model = glm::rotate(model, glm::pi<float>() * 1.5f, glm::vec3(1, 0, 0));

            p.planetShader.setMat4("model", model);
            p.planetShader.setInt("numVectors", sphereTextureCoords.size());
            GLint location = glGetUniformLocation(p.planetShader.ID, "myVectors");
            if (sphereTextureCoords.size() > 0)
            {
                glUniform2fv(location, sphereTextureCoords.size(), &sphereTextureCoords[0].x);
            }
            glDrawElements(GL_TRIANGLES, p.mIndices.size(), GL_UNSIGNED_INT, 0);

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(7.5 * deltaTime);
    //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //    cameraPos += cameraSpeed * cameraFront;
    //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //    cameraPos -= cameraSpeed * cameraFront;
    //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    //placing shits
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        thetaPlace += 0.1 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        thetaPlace -= 0.1 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        phiPlace += 1.0 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        phiPlace -= 1.0 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        placed = true;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void mouse_callback_camera_swipe(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        rightMouseButtonPressed = true;
    else
        rightMouseButtonPressed = false;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        yaw += xoffset;
        pitch += yoffset;

        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        xAngleValue -= xoffset;
        yAngleValue += yoffset;
    }
    else
    {
        lastX = xpos;
        lastY = ypos;
    }
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    swipeCameraRadius += 0.2 * yoffset;
}