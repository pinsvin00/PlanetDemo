#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Utils.h"
#include "Renderer.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Game.h"
#include <thread>

// TODO
// Fix rendering bottom of the ocean, include the real data, to show shallow seas
// Fix the paths, they're currently based of the hardcoded string
// Fix the mWaterLandTexture of the water, it's currently too big, and should show much less details in the general
// Clean up the code
// Fix the borders between provinces
// Move some code from the plane to the Renderer class,  or some other, the planet class doesn't seem like the right thing, it's singleton
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
    glfwSetFramebufferSizeCallback(ctx->mWindow, &Renderer::FrameBufferSizeCallback);
    glfwSetCursorPosCallback(ctx->mWindow, &Renderer::MouseCallback);
    glfwSetScrollCallback(ctx->mWindow, &Renderer::ScrollCallback);
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


void gameThread(std::shared_ptr<Game> gameInstance)
{
    std::shared_ptr<WindowContext> ctx = WindowContext::GetInstance();
    while (!glfwWindowShouldClose(ctx->mWindow))
    {
        gameInstance->OnTick();
    }
}

int main()
{
    srand(time(NULL));
    SetupOpenGL();

    std::shared_ptr<WindowContext> ctx = WindowContext::GetInstance();
    std::shared_ptr<Game> game = std::make_shared<Game>();
    std::shared_ptr<Renderer> renderer = Renderer::GetInstance();
    std::thread threadOfGame(gameThread, game);

    while (!glfwWindowShouldClose(ctx->mWindow))
    {
        glClearColor(0.00f, 0.00f, 0.00f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer->RenderCurrentScene();
        glfwSwapBuffers(ctx->mWindow);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    threadOfGame.join();

    return 0;
}