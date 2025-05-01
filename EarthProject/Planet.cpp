#include "Planet.h"

void Planet::TryToCreateFloodFillMap(Utils::ImageData& imgDataIn, Utils::ImageData& imgDataOut, glm::vec2 startPoint, glm::vec3 color)
{
    std::queue<glm::vec2> queueOfPoints;
    std::vector<std::vector<bool>> visited(imgDataIn.h, std::vector<bool>(imgDataIn.w, false));
    queueOfPoints.push(startPoint);
    
    auto getPixelAt = [&](const Utils::ImageData& imgData, int x, int y) {
        return &imgData.data[x * imgData.nrChannels + (y * imgData.nrChannels * imgData.w)];
    };
    unsigned char* ptr = getPixelAt(imgDataIn, (int)startPoint.x, (int)startPoint.y);
    unsigned char startR = ptr[0];

    auto evaluatePixelForProvincesColoredMap = [startR](unsigned char* pixel) {
        return *pixel > 200;
    };
    auto isInsideTheTexutre = [](const Utils::ImageData& data, int x, int y) {
        return x >= 0 && x < data.w && y >= 0 && y < data.h;
    };

    auto setPixel = [&](unsigned char* pixel, glm::vec3 color) {
        pixel[0] = static_cast<int>(color.x);
    };

    glm::vec2 dirs[] = {
        glm::vec2(1,0),
        glm::vec2(-1,0),
        glm::vec2(0,1),
        glm::vec2(0,-1),
    };
    
    int i = 0;

    while (!queueOfPoints.empty())
    {
        glm::vec2 point = queueOfPoints.front();
        queueOfPoints.pop();

        unsigned char* data = getPixelAt(imgDataOut, point.x, point.y);
        setPixel(data, color);

        for (const glm::vec2& dir : dirs)
        {
            glm::vec2 pointNew = point + dir;
            unsigned char* pixel = getPixelAt(imgDataIn, pointNew.x, pointNew.y);
            if (isInsideTheTexutre(imgDataIn, pointNew.x, pointNew.y) && !visited[pointNew.y][pointNew.x] && evaluatePixelForProvincesColoredMap(pixel))
            {
                visited[pointNew.y][pointNew.x] = true;
                queueOfPoints.push(pointNew);
                i++;
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, this->provinceTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, imgDataOut.w, imgDataOut.h, 0, GL_RED, GL_UNSIGNED_BYTE, imgDataOut.data);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        switch (error) {
        case GL_INVALID_ENUM:
            std::cerr << "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument." << std::endl;
            break;
        case GL_INVALID_VALUE:
            std::cerr << "GL_INVALID_VALUE: A numeric argument is out of range." << std::endl;
            break;
        case GL_INVALID_OPERATION:
            std::cerr << "GL_INVALID_OPERATION: The specified operation is not allowed in the current state." << std::endl;
            break;
        case GL_OUT_OF_MEMORY:
            std::cerr << "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command." << std::endl;
            break;
        default:
            std::cerr << "Unknown error occurred." << std::endl;
            break;
        }
    }

    glGenerateMipmap(GL_TEXTURE_2D);
}

bool IsPointInsidePolygon(const glm::vec2& point, const std::vector<glm::vec2>& polygon) {
    int intersections = 0;
    size_t n = polygon.size();

    for (size_t i = 0; i < n; i++) {
        glm::vec2 v1 = polygon[i];
        glm::vec2 v2 = polygon[(i + 1) % n];  // Wrap around to the first vertex

        // Check if point lies between the y-range of the edge
        if ((point.y > v1.y) != (point.y > v2.y)) {
            // Calculate x-coordinate of the intersection point
            float intersectX = (v2.x - v1.x) * (point.y - v1.y) / (v2.y - v1.y) + v1.x;

            // Check if point is to the left of the intersection point
            if (point.x < intersectX) {
                intersections++;
            }
        }
    }

    // Odd number of intersections means the point is inside
    return (intersections % 2) == 1;
}

void Planet::SetupRenderData()
{
    mWaterLandTexture = Utils::Render::LoadTexture((Utils::Paths::ProjDir + "assets/textures/earthmap_bw.png").c_str(), false, &mLandMassImgData);
    provinceTexture = Utils::Render::LoadTexture((Utils::Paths::ProjDir + "assets/textures/mapka.png").c_str(), false, &mStatesImgData);
    textureBottom = Utils::Render::LoadTexture((Utils::Paths::ProjDir + "assets/textures/dry_riverbed.jpg").c_str(), false);
    heightMapTexture = Utils::Render::LoadTexture((Utils::Paths::ProjDir + "assets/textures/gebco_08_rev_elev_low_res.png").c_str(), false);
    terrianMapTexture = Utils::Render::LoadTexture((Utils::Paths::ProjDir + "assets/textures/earthmap_terrain.jpg").c_str(), false);
    dudvMapTexture = Utils::Render::LoadTexture((Utils::Paths::ProjDir + "assets/textures/dudv.png").c_str(), false);
    waterTexture = Utils::Render::LoadTexture((Utils::Paths::ProjDir + "assets/textures/water.png").c_str(), false);


    //bind VAO, VBO, send the data to VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mVerts.size() * sizeof(float), mVerts.data(), GL_STATIC_DRAW);

    //bind EBO & transfer the data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(int), mIndices.data(), GL_STATIC_DRAW);

    //vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //mWaterLandTexture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Planet::GenerateSphere(float radius, int sectorCount, int stackCount, std::vector<float>& vertices, std::vector<int>& indices) {

    const float M_PI = 3.14f;

    float x, y, z, xy; // vertex position
    float s, t; // vertex mWaterLandTexture coordinates
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
            x = radius * cosf(phi) * cosf(theta); // r * cos(u) * cos(v)
            y = radius * cosf(phi) * sinf(theta); // r * cos(u) * sin(v)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            float nx = x / radius;
            float ny = y / radius;
            float nz = z / radius;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            // mWaterLandTexture coordinates (s, t)
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
