#include "Demo.h"
#include "Utils.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <format>

Demo::Demo() : 
    mEarthPlanet(10.0f),
    cubeShader(
        (Utils::Paths::ProjDir + "assets\\shaders\\cube_vertex.glsl").c_str(),
        (Utils::Paths::ProjDir + "assets\\shaders\\cube_fragment.glsl").c_str()
    )
{

}

std::shared_ptr<Demo> Demo::GetInstance()
{
    static std::shared_ptr<Demo> instance(new Demo());
    return instance;
}

void Demo::Init()
{
    mEarthPlanet.GenerateSphere(1.0f, 144, 72, mEarthPlanet.mVerts, mEarthPlanet.mIndices);
    mEarthPlanet.SetupRenderData();
    windowCtx = WindowContext::GetInstance();
    mGameSettings = mJsonParser.load_from_file(Utils::Paths::ProjDir + mFileName);
    mMaxZoomValue = asago_bytes_to_double((*mGameSettings->mapped_values)["MAX_ZOOM"]->result);
    mMinZoomValue = asago_bytes_to_double((*mGameSettings->mapped_values)["MIN_ZOOM"]->result);
}

void Demo::SetupOpenGL()
{

}

void Demo::OnTick()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Text(std::format("max zoom = {}", mMaxZoomValue).c_str());
    ImGui::Text(std::format("min zoom = {}", mMinZoomValue).c_str());

    //dt
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    //Determine the view & the projection matrices
    mCameraPosition = glm::vec3(
        cos(glm::radians(xAngleValue)) * cos(glm::radians(yAngleValue)),
        sin(glm::radians(yAngleValue)),
        sin(glm::radians(xAngleValue)) * cos(glm::radians(yAngleValue))
    );
    mCameraPosition *= swipeCameraRadius;
    mCameraFront = -glm::normalize(mCameraPosition);
    mView = glm::lookAt(mCameraPosition, mCameraPosition + mCameraFront, mCameraUp);
    mProjection = glm::perspective(glm::radians(fov), (float)windowCtx->mXSize / (float)windowCtx->mYSize, 0.1f, 100.0f);


    if (rightMouseButtonPressed && currentFrame - lastTimeMousePressedToRecolor >= 0.2f)
    {
        std::optional<glm::vec2> textureCoordinates = TrackMousePositionFromSphereToTexture(windowCtx->mWindow, mEarthPlanet.mRadius, mProjection, mView);
        if (textureCoordinates.has_value())
        {
            lastTimeMousePressedToRecolor = currentFrame;
            mEarthPlanet.TryToCreateFloodFillMap(mEarthPlanet.mStatesImgData, mEarthPlanet.mLandMassImgData, 
                glm::vec2((int)textureCoordinates->x, (int)textureCoordinates->y), glm::vec3(currentCountryId));
        }
    }

    //Input processing
    ProcessInput(windowCtx->mWindow);

    //Render the bottom of the sea
    {
        glBindVertexArray(mEarthPlanet.VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mEarthPlanet.textureBottom);

        mEarthPlanet.planetBottomShader.use();
        mEarthPlanet.planetBottomShader.setMat4("projection", mProjection);
        mEarthPlanet.planetBottomShader.setMat4("view", mView);
        mEarthPlanet.planetBottomShader.setInt("planetBottomMap", 0);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(mEarthPlanet.mRadius - 0.20f));
        mEarthPlanet.planetBottomShader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, mEarthPlanet.mIndices.size(), GL_UNSIGNED_INT, 0);
    }

    //Render planet
    {
        ShaderUtil& planetShader = mEarthPlanet.planetShader;
        mEarthPlanet.planetShader.use();

        glBindVertexArray(mEarthPlanet.VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mEarthPlanet.mWaterLandTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mEarthPlanet.heightMapTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mEarthPlanet.terrianMapTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mEarthPlanet.dudvMapTexture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, mEarthPlanet.waterTexture);
        planetShader.setMat4("projection", mProjection);
        planetShader.setMat4("view", mView);
        planetShader.setInt("redPlanetMap", 0);
        planetShader.setInt("heightMap", 1);
        planetShader.setInt("terrainMapTexture", 2);
        planetShader.setInt("dudvMap", 3);
        planetShader.setInt("waterTexture", 4);

        float time = static_cast<float>(glfwGetTime());
        float theta = glm::radians(180.0f);
        float phi = time * 2.0f;

        const float radiusAdd = 20.0f;
        float x = (mEarthPlanet.mRadius + radiusAdd) * cosf(phi) * cosf(theta); // r * cos(u) * cos(v)
        float y = (mEarthPlanet.mRadius + radiusAdd) * cosf(phi) * sinf(theta); // r * cos(u) * sin(v)
        float z = (mEarthPlanet.mRadius + radiusAdd) * sinf(phi); // r * sin(u)

        float nx, ny, nz;
        nx = x / mEarthPlanet.mRadius;
        ny = y / mEarthPlanet.mRadius;
        nz = z / mEarthPlanet.mRadius;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(mEarthPlanet.mRadius));
        model = glm::rotate(model,
            glm::pi<float>() * 1.5f,
            glm::vec3(1, 0, 0)
        );


        planetShader.setFloat("time", static_cast<float>(glfwGetTime()));
        planetShader.setVec3("viewPos", glm::vec3(1.0f, 2.0f, 5.0f));
        planetShader.setVec3("light.position", x,y,z);
        planetShader.setVec3("light.direction", nx,ny,nz);
        planetShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        planetShader.setVec3("light.diffuse", .9f, .9f, .9f);
        planetShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        planetShader.setMat4("model", model);

        glDrawElements(GL_TRIANGLES, mEarthPlanet.mIndices.size(), GL_UNSIGNED_INT, 0);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Demo::ProcessCamera()
{
    mCameraPosition = glm::vec3(
        cos(glm::radians(xAngleValue)) * cos(glm::radians(yAngleValue)),
        sin(glm::radians(yAngleValue)),
        sin(glm::radians(xAngleValue)) * cos(glm::radians(yAngleValue))
    );

    mCameraPosition *= swipeCameraRadius;
    mCameraFront = -glm::normalize(mCameraPosition);
    glm::mat4 view = glm::lookAt(mCameraPosition, mCameraPosition + mCameraFront, mCameraUp);
}

void Demo::Terminate()
{
    glfwTerminate();
}

std::optional<glm::vec2> Demo::TrackMousePositionFromSphereToTexture(GLFWwindow* window, float planetRadius, glm::mat4 projection, glm::mat4 view)
{
    std::optional<glm::vec3> position3DSphere = MathUtils::MousePositionToSphere(window, mCameraPosition, mEarthPlanet.mRadius, mProjection, mView);
    //We don't need to have the mouse on the planet, dont track anything if mouse doesn't point to the planet.
    if (!position3DSphere.has_value())
    {
        return std::nullopt;
    }

    double theta, phi;
    MathUtils::SphericalVectorToAngularPosition(*position3DSphere, theta, phi);
    //Convert spherical coordinates to the x,y both ranging (0.0f,1.0f)
    double xf = 1.0f - ((theta) / (2 * glm::pi<float>()));
    double yf = (phi) / glm::pi<float>();
    return glm::vec2(xf * mEarthPlanet.mLandMassImgData.w, yf * mEarthPlanet.mLandMassImgData.h);
}

//STATIC FUNCTIONS, RELATED TO GLFW
void Demo::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    std::shared_ptr<Demo> demo = Demo::GetInstance();
    demo->swipeCameraRadius += 0.2 * yoffset;
    demo->swipeCameraRadius = glm::clamp(demo->swipeCameraRadius, demo->mMinZoomValue, demo->mMaxZoomValue);
}

void Demo::MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    std::shared_ptr<Demo> demo = Demo::GetInstance();

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    demo->rightMouseButtonPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (demo->firstMouse)
        {
            demo->lastX = xpos;
            demo->lastY = ypos;
            demo->firstMouse = false;
        }

        float xoffset = xpos - demo->lastX;
        float yoffset = demo->lastY - ypos;
        demo->lastX = xpos;
        demo->lastY = ypos;

        demo->yaw += xoffset;
        demo->pitch += yoffset;

        float sensitivity = 0.1f; 
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        demo->xAngleValue -= xoffset;
        demo->yAngleValue += yoffset;
    }
    else
    {
        demo->lastX = xpos;
        demo->lastY = ypos;
    }
}

void Demo::ProcessInput(GLFWwindow* window)
{
    std::shared_ptr<Demo> demo = Demo::GetInstance();

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3& cameraFront = demo->mCameraFront;
    glm::vec3& cameraUp = demo->mCameraUp;
    float deltaTime = demo->deltaTime;

    float cameraSpeed = static_cast<float>(7.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        demo->mCameraPosition += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        demo->mCameraPosition -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        demo->mCameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        demo->mCameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        int countryId;
        std::cout << "Please enter the country id you wish to edit.";
        std::cin >> countryId;
        demo->currentCountryId = countryId;
    }
}

void Demo::FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    std::shared_ptr<WindowContext> ctx = WindowContext::GetInstance();
    ctx->mXSize = width;
    ctx->mYSize = height;
    glViewport(0, 0, width, height);
}

