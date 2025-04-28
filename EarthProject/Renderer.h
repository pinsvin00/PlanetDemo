#pragma once
#include "MathUtils.h"
#include "ShaderUtil.h"
#include "Planet.h"
#include "parser.h"
#include "util.h"

class Renderer
{
public:
	Renderer();
	static std::shared_ptr<Renderer> GetInstance();

	void Init();
	void SetupOpenGL();
	void RenderCurrentScene();

	//Window responsible
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	static void ProcessInput(GLFWwindow* window);
	static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

private:
	int mDisplayMode = 0;
	bool debugRenderSunPlaceholder = false;

	//Settings related
	std::string mFileName = "settings.json";
	Asago::Parser mJsonParser;
	Asago::Value* mGameSettings;

	std::shared_ptr<WindowContext> windowCtx;

	Planet mEarthPlanet;

	//Projection and view matrices
	glm::mat4 mProjection;
	glm::mat4 mView;

	//camera data
	bool firstMouse = true;
	float mYaw = -90.0f;	// mYaw is initialized to -90.0 degrees since a mYaw of 0.0 results in a direction vector pointing to theta right so we initially rotate a bit to theta left.
	float mPitch = 0.0f;
	float mLastX = 800.0f / 2.0;
	float mLastY = 600.0 / 2.0;
	float fov = 90.0f;
	
	double mMaxZoomValue = mEarthPlanet.mRadius + 100.0f;
	double mMinZoomValue = mEarthPlanet.mRadius;
	double swipeCameraRadius = mMaxZoomValue;
	double swipeSpeed = 3.0f;

	//
	double mXAngleValue = 0.0;
	double mYAngleValue = 0.0;

	glm::vec3 mCameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 mCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 mCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	//Frame buffers
	float lastTimeMousePressedToRecolor = 0.0f;

	//HDR and Bloom
	GLuint pingpongFBO[2] = { 0,0 };
	GLuint pingpongColorbuffers[2] = { 0,0 };
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	GLuint colorBuffers[2] = { 0,0 };
	GLuint hdrFBO = 0;
	GLuint rboDepth = 0;
	GLuint mFrameBuffer = 0;
	GLuint mHdrFrameBuffer = 0;
	GLuint mDepthTexture = 0;

	//Shaders
	ShaderUtil cubeShader;
	ShaderUtil mAtmosphereShader;
	ShaderUtil finalMixShader;
	ShaderUtil mBlurShader;


	void ProcessCamera();
	void Terminate();
	void RenderAtmosphere();
	void SetupFrameBuffers();
	void RenderPlanet();
	void RunPostProcessEffects();

	std::optional<glm::vec2> TrackMousePositionFromSphereToTexture(GLFWwindow* window, float planetRadius, glm::mat4 projection, glm::mat4 view);

public:
	bool mRightMouseButtonPressed = false;
	double lastTimePlaced = 0.0;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	//Debug data
	int __debug_Bloom = 1;
	int __debug_countryId = 129;

	float exposure;
};

