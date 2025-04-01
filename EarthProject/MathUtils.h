#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <glad/glad.h>
#include "WindowContext.h"
#include <GLFW/glfw3.h>

namespace MathUtils {
    void SphericalVectorToAngularPosition(glm::vec3 point, double& theta, double& phi);
    glm::vec3 GetMouseWorldPosition(
        float mouseX,
        float mouseY,
        glm::mat4 projection, glm::mat4 view,
        glm::vec2 screenSize
    );

    std::optional<glm::vec3> MousePositionToSphere(GLFWwindow* window, const glm::vec3& cameraPos, float planetRadius, glm::mat4 projection, glm::mat4 view);
}
