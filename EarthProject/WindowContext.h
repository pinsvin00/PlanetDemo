#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

class WindowContext
{
public:
	static std::shared_ptr<WindowContext> GetInstance();
	const int mXSize = 800;
	const int mYSize = 600;
	GLFWwindow* mWindow = nullptr;
};

