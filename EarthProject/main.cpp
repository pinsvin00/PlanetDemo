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

// TODO
// general refactor
// Fix rendering bottom of the ocean, include the real data, to show shallow seas
// Fix the paths, they're currently based of the hardcoded string
// Create the github repo
// Disallow scrolling into the middle of the planet
// Fix coloring of the provinces
// Create the map for provinces, and the outlines for them
// Fix the mWaterLandTexture of the water, it's currently too big, and should show much less details in the general
// Fix the resizing window, make sure that if we resize to the FULL HD the screen would be ok, same as the mouse
// Clean up the code

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
}

int main()
{
    srand(time(NULL));
    SetupOpenGL();

    std::shared_ptr<Demo> earthDemo = Demo::GetInstance();
    std::shared_ptr<WindowContext> context = WindowContext::GetInstance();
    earthDemo->Init();

    {
       //unsigned int cubeVBO, cubeVAO;
       //glGenVertexArrays(1, &cubeVAO);
       //glGenBuffers(1, &cubeVBO);

       //glBindVertexArray(cubeVAO);

       //glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
       //glBufferData(GL_ARRAY_BUFFER, sizeof(Utils::Render::CubeVertices), Utils::Render::CubeVertices, GL_STATIC_DRAW);

       //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
       //glEnableVertexAttribArray(0);
       //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
       //glEnableVertexAttribArray(1);


       //// load and create a mWaterLandTexture 
       //// -------------------------
       //unsigned int texture1, texture2, textureEarthWhiteBlack;
       //// mWaterLandTexture 1
       //// ---------
       //glGenTextures(1, &texture1);
       //glBindTexture(GL_TEXTURE_2D, texture1);
       //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
       //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
       //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       //// load image, create mWaterLandTexture and generate mipmaps
       //int width, height, nrChannels;
       //stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded mWaterLandTexture'xf on theta y-axis.
       //unsigned char* data = stbi_load(
       //    (Utils::Paths::ProjDir + "assets/textures/container.jpg").c_str(), &width, &height, &nrChannels, 0
       //);

       //if (data)
       //{
       //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
       //    glGenerateMipmap(GL_TEXTURE_2D);
       //}
       //else
       //{
       //    std::cout << "Failed to load mWaterLandTexture" << std::endl;
       //}
       //stbi_image_free(data);

       //glGenTextures(1, &texture2);
       //glBindTexture(GL_TEXTURE_2D, texture2);
       //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
       //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
       //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

       //data = stbi_load((Utils::Paths::ProjDir + "assets/textures/awesomeface.png").c_str(), &width, &height, &nrChannels, 0);
       //if (data)
       //{
       //    // note that theta awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL theta data type is of GL_RGBA
       //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
       //    glGenerateMipmap(GL_TEXTURE_2D);
       //}
       //else
       //{
       //    std::cout << "Failed to load mWaterLandTexture" << std::endl;
       //}
       //stbi_image_free(data);

       //ourShader.use();
       //ourShader.setInt("texture1", 0);
       //ourShader.setInt("texture2", 1);
    }
   
    std::shared_ptr<WindowContext> ctx = WindowContext::GetInstance();
    while (!glfwWindowShouldClose(ctx->mWindow))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        earthDemo->OnTick();

        glfwSwapBuffers(ctx->mWindow);
        glfwPollEvents();
    }


    return 0;
}