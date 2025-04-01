#include "Utils.h"
#include <glm/common.hpp>
#include <stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

uint32_t Utils::Render::LoadTexture(const char* pathToTexture, bool shouldImageBeFlipped, ImageData* out)
{
    uint32_t textureId;
    int width, height, nrChannels;

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(shouldImageBeFlipped);
    unsigned char* data = stbi_load(pathToTexture, &width, &height, &nrChannels, 0);

    int glChannels;
    switch (nrChannels)
    {
        case 1:
        {
            glChannels = GL_RED;
            break;
        }
        case 2:
        {
            glChannels = GL_RG;
            break;
        }
        case 3:
        {
            glChannels = GL_RGB;
            break;
        }
        case 4:
        {
            glChannels = GL_RGBA;
            break;
        }
        default:
        {
            std::cout << "Invalid channels value?" << std::endl;
            exit(1);
        }
    }

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, glChannels, width, height, 0, glChannels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "ERR: Failed to load textureId at " << pathToTexture << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    if (out == nullptr)
    {
        stbi_image_free(data);
    }
    else
    {
        out->data = data;
        out->w = width;
        out->h = height;
        out->nrChannels = nrChannels;
    }

    return textureId;
}
