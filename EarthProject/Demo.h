#pragma once
#include "MathUtils.h"
#include "ShaderUtil.h"
#include "Planet.h"
#include "parser.h"
#include "util.h"

class Demo
{
public:
	Demo();
	static std::shared_ptr<Demo> GetInstance();

	void Init();
	void SetupOpenGL();
	void OnTick();

	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	static void ProcessInput(GLFWwindow* window);
	static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

private:
	double mMaxZoomValue;
	double mMinZoomValue;

	//json related stuff
	std::string mFileName = "settings.json";
	Asago::Parser mJsonParser;
	Asago::Value* mGameSettings;

	std::shared_ptr<WindowContext> windowCtx;

	Planet mEarthPlanet;
	//context of the rendering, proj, view etc.
	glm::mat4 mProjection;
	glm::mat4 mView;

	glm::vec3 mCameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 mCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 mCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float lastTimeMousePressedToRecolor = 0.0f;

	ShaderUtil cubeShader;


	void ProcessCamera();
	void Terminate();

	std::vector<glm::vec2> mColorChangePolygonPoints;
	std::optional<glm::vec2> TrackMousePositionFromSphereToTexture(GLFWwindow* window, float planetRadius, glm::mat4 projection, glm::mat4 view);

public:
	bool firstMouse = true;
	float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to theta right so we initially rotate a bit to theta left.
	float pitch = 0.0f;
	float lastX = 800.0f / 2.0;
	float lastY = 600.0 / 2.0;
	float fov = 90.0f;
	bool placed = false;
	bool rightMouseButtonPressed = false;
	double lastTimePlaced = 0.0;
	double swipeCameraRadius = 15.0;
	double xAngleValue = 0.0;
	double yAngleValue = 0.0;
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float phi_place = 0.0f;
	float theta_place = 0.0f;
	int currentCountryId = 129;

};

