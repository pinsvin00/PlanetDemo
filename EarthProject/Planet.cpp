#include "Planet.h"

void Planet::TryToCreateFloodFillMap(Utils::ImageData & imgDataIn, Utils::ImageData & imgDataOut, glm::vec2 startPoint, glm::vec3 color)
{
    std::queue<glm::vec2> queueOfPoints;
    std::vector<std::vector<bool>> visited(imgDataIn.h, std::vector<bool>(imgDataIn.w, false));
    queueOfPoints.push(startPoint);
    
    auto getPixelAt = [&](const Utils::ImageData& imgData, int x, int y) {
        return &imgData.data[x * imgData.nrChannels + (y * imgData.nrChannels * imgData.w)];
    };

    auto evalPixel = [&](unsigned char* pixel) {
        return *pixel <= 5;
    };

    auto setPixel = [&](unsigned char* pixel, glm::vec3 color) {
        pixel[0] = static_cast<int>(color.x);
    };

    uint8_t* data = imgDataIn.data;

    glm::vec2 dirs[] = {
        glm::vec2(1,0),
        glm::vec2(-1,0),
        glm::vec2(0,1),
        glm::vec2(0,-1),
    };


    while (!queueOfPoints.empty())
    {
        glm::vec2 point = queueOfPoints.front();
        queueOfPoints.pop();

        unsigned char * data = getPixelAt(imgDataOut, point.x, point.y);
        setPixel(data, color);

        for (const glm::vec2& dir : dirs)
        {
            glm::vec2 pointNew = point + dir;
            unsigned char* pixel = getPixelAt(imgDataIn, pointNew.x, pointNew.y);
            if (!visited[pointNew.y][pointNew.x] && evalPixel(pixel))
            {
                visited[pointNew.y][pointNew.x] = true;
                queueOfPoints.push(pointNew);
            }
        }

    }
}

void Planet::SetupRenderData()
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load((Utils::Paths::ProjDir + "assets/textures/earthmap_bw2.jpg").c_str(), &width, &height, &nrChannels, 0);

    mLandMassImgData.data = data;
    mLandMassImgData.w = width;
    mLandMassImgData.h = height;
    mLandMassImgData.nrChannels = nrChannels;

    
    mStatesImgData.data = (unsigned char *) malloc(width * height * nrChannels);
    if (mStatesImgData.data == nullptr)
    {
        std::cout << "No memory to alloc" << std::endl;
        exit(0);
    }

    memcpy(mStatesImgData.data, mLandMassImgData.data, width * height * nrChannels);
    mStatesImgData.w = mLandMassImgData.w;
    mStatesImgData.h = mLandMassImgData.h;
    mStatesImgData.nrChannels = nrChannels;

    TryToCreateFloodFillMap(mLandMassImgData, mStatesImgData, glm::vec2(1939, 533), glm::vec3(129));
    TryToCreateFloodFillMap(mLandMassImgData, mStatesImgData, glm::vec2(2011, 480), glm::vec3(200));


    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, mStatesImgData.data);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }



    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    //bind VAO, VBO, send the data to VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mVerts.size() * sizeof(float), mVerts.data(), GL_STATIC_DRAW);

    //bind EBO & transfer the data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(int), mIndices.data(), GL_STATIC_DRAW);

    //vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    stbi_image_free(data);
}

void Planet::GenerateSphere(float radius, int sectorCount, int stackCount, std::vector<float>& vertices, std::vector<int>& indices) {

    const double M_PI = 3.14;

    float x, y, z, xy; // vertex position
    float nx, ny, nz; // vertex normal
    float s, t; // vertex texture coordinates
    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float theta, phi;

    for (int i = 0; i <= stackCount; ++i) {
        phi = M_PI / 2 - i * stackStep; // from pi/2 to -pi/2
        xy = radius * cosf(phi); // r * cos(u)
        z = radius * sinf(phi); // r * sin(u)

        for (int j = 0; j <= sectorCount; ++j) {
            theta = j * sectorStep; // from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(theta); // r * cos(u) * cos(v)
            y = xy * sinf(theta); // r * cos(u) * sin(v)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // texture coordinates (s, t)
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }

    // indices
    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1); // current vertex
        int k2 = k1 + sectorCount + 1; // next vertex

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k2);
                indices.push_back(k2 + 1);
                indices.push_back(k1 + 1);
            }
        }
    }

}
