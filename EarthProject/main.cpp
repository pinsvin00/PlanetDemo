#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "MathUtils.h"
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
#include "Demo.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// TODO
// Fix rendering bottom of the ocean, include the real data, to show shallow seas
// Fix the paths, they're currently based of the hardcoded string
// Fix coloring of the provinces
// Fix the mWaterLandTexture of the water, it's currently too big, and should show much less details in the general
// Clean up the code
// Fix the borders between provinces
// Add some cli stuff to change the current color
// Move some code from the plane to the Demo class,  or some other, the planet class doesn't seem like the right thing, it's singleton
// settings

void SetupOpenGL()
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
    std::shared_ptr<WindowContext> ctx = WindowContext::GetInstance();
    ctx->mWindow = glfwCreateWindow(ctx->mXSize, ctx->mYSize, "LearnOpenGL", NULL, NULL);
    if (ctx->mWindow == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(ctx->mWindow);
    glfwSetFramebufferSizeCallback(ctx->mWindow, &Demo::FrameBufferSizeCallback);
    glfwSetCursorPosCallback(ctx->mWindow, &Demo::MouseCallback);
    glfwSetScrollCallback(ctx->mWindow, &Demo::ScrollCallback);
    glfwSetInputMode(ctx->mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(WindowContext::GetInstance()->mWindow, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

int main()
{
    srand(time(NULL));
    SetupOpenGL();

    std::shared_ptr<Demo> earthDemo = Demo::GetInstance();
    std::shared_ptr<WindowContext> ctx = WindowContext::GetInstance();
    earthDemo->Init();
   
    while (!glfwWindowShouldClose(ctx->mWindow))
    {
        glClearColor(0.05, 0.05, 0.05, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        earthDemo->OnTick();

        glfwSwapBuffers(ctx->mWindow);
        glfwPollEvents();
    }


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}