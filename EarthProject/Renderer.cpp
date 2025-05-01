#include "Renderer.h"
#include "Utils.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <format>

Renderer::Renderer() : 
    mEarthPlanet(400.0f),
    cubeShader(
        (Utils::Paths::ProjDir + "assets\\shaders\\cube_vertex.glsl").c_str(),
        (Utils::Paths::ProjDir + "assets\\shaders\\cube_fragment.glsl").c_str()
    ),
    mAtmosphereShader(
        (Utils::Paths::ProjDir + "assets\\shaders\\atmosphere_vertex.glsl").c_str(),
        (Utils::Paths::ProjDir + "assets\\shaders\\atmosphere_fragment.glsl").c_str()
    ),
    finalMixShader(
        (Utils::Paths::ProjDir + "assets\\shaders\\quad_mix_final_vs.glsl").c_str(),
        (Utils::Paths::ProjDir + "assets\\shaders\\quad_mix_final_fs.glsl").c_str()
    ),
    mBlurShader(
        (Utils::Paths::ProjDir + "assets\\shaders\\quad_blur_vs.glsl").c_str(),
        (Utils::Paths::ProjDir + "assets\\shaders\\quad_blur_fs.glsl").c_str()
    )
{
    std::shared_ptr<WindowContext> ctx = WindowContext::GetInstance();
    glfwMakeContextCurrent(ctx->mWindow);
    glfwSetFramebufferSizeCallback(ctx->mWindow, &Renderer::FrameBufferSizeCallback);
    glfwSetCursorPosCallback(ctx->mWindow, &Renderer::MouseCallback);
    glfwSetScrollCallback(ctx->mWindow, &Renderer::ScrollCallback);
    glfwSetInputMode(ctx->mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

std::shared_ptr<Renderer> Renderer::GetInstance()
{
    static std::shared_ptr<Renderer> instance(new Renderer());
    return instance;
}

void Renderer::Init()
{
    std::shared_ptr<WindowContext> windowContext = WindowContext::GetInstance();

    mEarthPlanet.GenerateSphere(1.0f, 144, 72, mEarthPlanet.mVerts, mEarthPlanet.mIndices);
    mEarthPlanet.SetupRenderData();
    windowCtx = WindowContext::GetInstance();
    //mGameSettings = mJsonParser.load_from_file(Utils::Paths::ProjDir + mFileName);
    //mMaxZoomValue = 20.0f; //asago_bytes_to_double((*mGameSettings->mapped_values)["MAX_ZOOM"]->result);
    //mMinZoomValue = 10.0f; //asago_bytes_to_double((*mGameSettings->mapped_values)["MIN_ZOOM"]->result);
    SetupFrameBuffers();
}

void Renderer::SetupOpenGL()
{

}

void Renderer::RenderCurrentScene()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Text(std::format("max zoom = {}", mMaxZoomValue).c_str());
    ImGui::Text(std::format("min zoom = {}", mMinZoomValue).c_str());
    ImGui::InputInt("Graphics mode", &mDisplayMode);
    ImGui::Checkbox("Debug render sun", &this->debugRenderSunPlaceholder);
    mRightMouseButtonPressed = glfwGetMouseButton(windowCtx->mWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    //auto val = TrackMousePositionFromSphereToTexture(windowCtx->mWindow, mEarthPlanet.mRadius, mProjection, mView);
    //ImGui::Text(std::format("x texture = {}", texturePosition.x).c_str());
    //ImGui::Text(std::format("y texture = {}", texturePosition.y).c_str());

    //dt
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    //Determine the view & the projection matrices
    mCameraPosition = glm::vec3(
        cos(glm::radians(mXAngleValue)) * cos(glm::radians(mYAngleValue)),
        sin(glm::radians(mYAngleValue)),
        sin(glm::radians(mXAngleValue)) * cos(glm::radians(mYAngleValue))
    );
    mCameraPosition *= swipeCameraRadius;
    mCameraFront = -glm::normalize(mCameraPosition);
    mView = glm::lookAt(mCameraPosition, mCameraPosition + mCameraFront, mCameraUp);    
    mProjection = glm::perspective(glm::radians(fov), (float)windowCtx->mXSize / (float)windowCtx->mYSize, 0.1f, 400.0f);


    if (mRightMouseButtonPressed && currentFrame - lastTimeMousePressedToRecolor >= 0.2f)
    {
        std::optional<glm::vec2> textureCoordinates = TrackMousePositionFromSphereToTexture(windowCtx->mWindow, mEarthPlanet.mRadius, mProjection, mView);
        if (textureCoordinates.has_value())
        {
            lastTimeMousePressedToRecolor = currentFrame;
            std::cout << textureCoordinates->x << " " << textureCoordinates->y << std::endl;
            mEarthPlanet.TryToCreateFloodFillMap(mEarthPlanet.mStatesImgData, mEarthPlanet.mStatesImgData,
                glm::vec2((int)textureCoordinates->x, (int)textureCoordinates->y), glm::vec3(__debug_countryId)
            );
        }
    }

    //Input processing
    ProcessInput(windowCtx->mWindow);

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    glDepthMask(GL_FALSE);
    RenderAtmosphere();
    glDepthMask(GL_TRUE);

    RenderPlanet();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    RunPostProcessEffects();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::ProcessCamera()
{
    mCameraPosition = glm::vec3(
        cos(glm::radians(mXAngleValue)) * cos(glm::radians(mYAngleValue)),
        sin(glm::radians(mYAngleValue)),
        sin(glm::radians(mXAngleValue)) * cos(glm::radians(mYAngleValue))
    );

    mCameraPosition *= swipeCameraRadius;
    mCameraFront = -glm::normalize(mCameraPosition);
    glm::mat4 view = glm::lookAt(mCameraPosition, mCameraPosition + mCameraFront, mCameraUp);
}

void Renderer::Terminate()
{
    glfwTerminate();
}

void Renderer::RenderAtmosphere()
{
    ShaderUtil& atmosphereShader = mAtmosphereShader;
    atmosphereShader.use();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(mEarthPlanet.mRadius + 0.05f));
    model = glm::rotate(model,
        glm::pi<float>() * 1.5f,
        glm::vec3(1, 0, 0)
    );

    float time = static_cast<float>(glfwGetTime() / 30.0);
    constexpr float theta = glm::radians(180.0f);
    float phi = time * 2.0f;

    const float radiusAdd = 20.0f;
    float x = (mEarthPlanet.mRadius + radiusAdd) * cosf(phi) * cosf(theta); // r * cos(u) * cos(v)
    float y = (mEarthPlanet.mRadius + radiusAdd) * cosf(phi) * sinf(theta); // r * cos(u) * sin(v)
    float z = (mEarthPlanet.mRadius + radiusAdd) * sinf(phi); // r * sin(u)

    float nx, ny, nz;
    nx = x / mEarthPlanet.mRadius;
    ny = y / mEarthPlanet.mRadius;
    nz = z / mEarthPlanet.mRadius;

    atmosphereShader.setVec3("viewPos", mCameraPosition);
    atmosphereShader.setVec3("light.position", x, y, z);
    atmosphereShader.setVec3("light.direction", nx, ny, nz);
    atmosphereShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);    
    atmosphereShader.setVec3("light.diffuse", .3f, .3f, .3f);
    atmosphereShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    atmosphereShader.setMat4("projection", mProjection);
    atmosphereShader.setMat4("view", mView);
    atmosphereShader.setMat4("model", model);

    glBindVertexArray(mEarthPlanet.VAO);
    glDrawElements(GL_TRIANGLES, mEarthPlanet.mIndices.size(), GL_UNSIGNED_INT, 0);
}

void Renderer::SetupFrameBuffers()
{
    // configure (floating point) framebuffers
    // ---------------------------------------
    const int SCR_WIDTH = WindowContext::GetInstance()->mXSize;
    const int SCR_HEIGHT = WindowContext::GetInstance()->mYSize;

    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }


    finalMixShader.use();
    finalMixShader.setInt("scene", 0);
    finalMixShader.setInt("bloomBlur", 1);
    std::cout << glGetError() << std::endl;
}

void Renderer::RenderPlanet()
{
    ShaderUtil& planetShader = mEarthPlanet.planetShader;
    mEarthPlanet.planetShader.use();
    
    glBindVertexArray(mEarthPlanet.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mEarthPlanet.provinceTexture);
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
    planetShader.setInt("displayMode", mDisplayMode);
    
    float time = static_cast<float>(glfwGetTime() / 30.0);
    constexpr float theta = glm::radians(180.0f);
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
    planetShader.setVec3("viewPos", mCameraPosition);
    planetShader.setVec3("light.position", x, y, z);
    planetShader.setVec3("light.direction", nx, ny, nz);
    planetShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    planetShader.setVec3("light.diffuse", .3f, .3f, .3f);
    planetShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    planetShader.setMat4("model", model);

    glDrawElements(GL_TRIANGLES, mEarthPlanet.mIndices.size(), GL_UNSIGNED_INT, 0);

    if (debugRenderSunPlaceholder)
    {
        cubeShader.use();
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(x, y, z));
        model = glm::scale(model, glm::vec3(4.0f));
        cubeShader.setMat4("model", model);
        cubeShader.setMat4("projection", mProjection);
        cubeShader.setMat4("view", mView);

        Utils::Render::renderCube();
    }

}

void Renderer::RunPostProcessEffects()
{
    //The "bright colors" are stored inside the the framebuffer, blur it a couple of times
    bool horizontal = true, first_iteration = true;
    unsigned int amount = 10;
    mBlurShader.use();
    mBlurShader.setInt("image", 4);
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        mBlurShader.setInt("horizontal", horizontal);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
        Utils::Render::renderQuad();
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //We got our blur framebuffer rendered, now let's just render it!
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    finalMixShader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

    finalMixShader.setInt("bloom", 1);
    finalMixShader.setFloat("exposure", 1.5f);
    Utils::Render::renderQuad();
}

std::optional<glm::vec2> Renderer::TrackMousePositionFromSphereToTexture(GLFWwindow* window, float planetRadius, glm::mat4 projection, glm::mat4 view)
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

    return glm::vec2(xf * mEarthPlanet.mStatesImgData.w, yf * mEarthPlanet.mStatesImgData.h);
}

//STATIC FUNCTIONS, RELATED TO GLFW
void Renderer::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    std::shared_ptr<Renderer> demo = Renderer::GetInstance();
    demo->swipeCameraRadius += demo->swipeSpeed * yoffset;
    demo->swipeCameraRadius = glm::clamp(demo->swipeCameraRadius, demo->mMinZoomValue, demo->mMaxZoomValue);
}

void Renderer::MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    std::shared_ptr<Renderer> demo = Renderer::GetInstance();

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (demo->firstMouse)
        {
            demo->mLastX = xpos;
            demo->mLastY = ypos;
            demo->firstMouse = false;
        }

        float xoffset = xpos - demo->mLastX;
        float yoffset = demo->mLastY - ypos;
        demo->mLastX = xpos;
        demo->mLastY = ypos;

        demo->mYaw += xoffset;
        demo->mPitch += yoffset;

        float sensitivity = 0.1f; 
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        demo->mXAngleValue -= xoffset;
        demo->mYAngleValue += yoffset;
    }
    else
    {
        demo->mLastX = xpos;
        demo->mLastY = ypos;
    }
}

void Renderer::ProcessInput(GLFWwindow* window)
{
    std::shared_ptr<Renderer> demo = Renderer::GetInstance();

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
        demo->__debug_countryId = countryId;
    }
}

void Renderer::FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    std::shared_ptr<WindowContext> ctx = WindowContext::GetInstance();
    ctx->mXSize = width;
    ctx->mYSize = height;
    glViewport(0, 0, width, height);
}

