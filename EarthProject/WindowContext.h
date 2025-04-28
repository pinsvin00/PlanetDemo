#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

class WindowContext
{
public:
	static std::shared_ptr<WindowContext> GetInstance();
	int mXSize = 1600;
	int mYSize = 900;
	GLFWwindow* mWindow = nullptr;
};

