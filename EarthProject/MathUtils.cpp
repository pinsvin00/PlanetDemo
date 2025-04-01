#include "MathUtils.h"

void MathUtils::SphericalVectorToAngularPosition(glm::vec3 point, double& theta, double& phi) {
    float x = point.x;
    float y = point.y;
    float z = point.z;

    float r = std::sqrt(x * x + y * y + z * z);

    // Azimuth (theta)
    theta = std::atan2(z, x);
    if (theta < 0) theta += 2.0f * glm::pi<float>();  // Ensure theta is in [0, 2*pi]
    // Elevation (phi)
    phi = std::acos(y / r);
    phi = std::fmax(0.0f, phi);    // Clamp to avoid rare negative results
}

glm::vec3 MathUtils::GetMouseWorldPosition(float mouseX, float mouseY, glm::mat4 projection, glm::mat4 view, glm::vec2 screenSize) {
    // Step 1: Normalize mouse coordinates to [-1, 1]
    float x = (2.0f * mouseX) / screenSize.x - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenSize.y;  // Invert Y axis
    float z = 1.0f;  // Assume the mouse is at the far plane initially

    glm::vec3 ndcPos = glm::vec3(x, y, z);

    // Step 2: Create homogeneous clip coordinates
    glm::vec4 clipCoords = glm::vec4(ndcPos.x, ndcPos.y, -1.0f, 1.0f);

    // Step 3: Unproject by multiplying with the inverse of the combined matrix
    glm::mat4 invViewProj = glm::inverse(projection * view);
    glm::vec4 worldPos = invViewProj * clipCoords;

    // Step 4: Perspective divide to get actual world position
    worldPos /= worldPos.w;

    return glm::vec3(worldPos);
}

std::optional<glm::vec3> MathUtils::MousePositionToSphere(GLFWwindow* window, const glm::vec3& cameraPos, float planetRadius, glm::mat4 projection, glm::mat4 view)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    glm::vec3 rayOrigin = cameraPos;

    std::shared_ptr<WindowContext> ctx = WindowContext::GetInstance();
    glm::vec3 mousePosition = GetMouseWorldPosition(mouseX, mouseY, projection, view, glm::vec2(ctx->mXSize, ctx->mYSize));

    glm::vec3 rayDirection = glm::normalize(mousePosition - rayOrigin);

    glm::vec3 sphereCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 startPosition = rayOrigin - sphereCenter;

    float a = glm::dot(rayDirection, rayDirection);
    float b = 2.0f * glm::dot(startPosition, rayDirection);
    float c = glm::dot(startPosition, startPosition) - planetRadius * planetRadius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
        return {};

    float distsqrt = sqrt(discriminant);
    float q;

    if (b < 0)
        q = (-b - distsqrt) / 2.0f;
    else
        q = (-b + distsqrt) / 2.0f;

    float t0 = q / a;
    float t1 = c / q;

    //if t0 > t1 then swap them.
    if (t0 > t1)
    {
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    //the ray missed the sphere
    if (t1 < 0)
        return {};
    //if t0<0 intersection is at t1
    if (t0 < 0)
        return cameraPos + rayDirection * t1;

    return cameraPos + rayDirection * t0;
}
