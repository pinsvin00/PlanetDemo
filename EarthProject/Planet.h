#pragma once

#include <iostream>
#include <vector>
#include <math.h>
#include <glm/common.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Utils.h"
#include <queue>
#include <stb_image.h>
#include "ShaderUtil.h"

class Planet
{


public:
    uint32_t texture, VAO, VBO, EBO;
    uint32_t textureBottom, heightMapTexture, terrianMapTexture, dudvMapTexture, waterTexture;
    uint32_t vertCount = 0;
    float mRadius = 0.0f;

    Planet(float radius) :
        planetShader(
            (Utils::Paths::ProjDir + "assets\\shaders\\earth_shader_vertex.glsl").c_str(),
            (Utils::Paths::ProjDir + "assets\\shaders\\earth_shader_fragment.glsl").c_str()
        ),
        planetBottomShader(
            (Utils::Paths::ProjDir + "assets\\shaders\\earth_shader_vertex.glsl").c_str(),
            (Utils::Paths::ProjDir + "assets\\shaders\\earth_shader_fragment_bottom.glsl").c_str()
        ),
        mRadius(radius)
    {
    }

  public:

    void TryToCreateFloodFillMap(Utils::ImageData& imgData, Utils::ImageData& imgDataOut, glm::vec2 startPoint, glm::vec3 color);
    void TryToCreateFloodFillMapTO_DELETE(Utils::ImageData& imgDataIn, Utils::ImageData& imgDataOut, std::vector<glm::vec2> vecs, glm::vec3 color);
    void SetupRenderData();
    void GenerateSphere(float radius, int sectorCount, int stackCount, std::vector<float>& vertices, std::vector<int>& indices);

    std::vector<float> mVerts;
    std::vector<int> mIndices;
    ShaderUtil planetShader;
    ShaderUtil planetBottomShader;
    Utils::ImageData mLandMassImgData;
    Utils::ImageData mStatesImgData;

};

