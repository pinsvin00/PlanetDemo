#pragma once
#include <string>

namespace Utils
{
    struct ImageData {
        int nrChannels, w, h;
        unsigned char* data;
    };

    namespace Paths
    {
        const std::string ProjDir = "C:\\Users\\pnsv0\\source\\repos\\PlanetDemo\\EarthProject\\";
    }
    namespace Render
    {
        uint32_t LoadTexture(const char* pathToTexture, bool shouldImageBeFlipped, ImageData* out = nullptr);
        static inline unsigned int cubeVAO = 0;
        static inline unsigned int quadVAO = 0;
        static inline unsigned int quadVBO = 0;
        static inline unsigned int cubeVbO = 0;
        void renderQuad();
        void renderCube();
        void SetupOpenGL();
    }
}

