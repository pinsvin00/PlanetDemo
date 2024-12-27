#pragma once

#include <iostream>
#include <vector>
#include <math.h>
#include <glm/common.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Utils.h"
#include <stb_image.h>
#include "ShaderUtil.h"
#include "Planet.h"

class PlanetSoldier
{
	uint32_t VAO, VBO;
	uint32_t texture;

	std::shared_ptr<Planet> mPlanet;
	ShaderUtil mSoldierShader;

	PlanetSoldier(std::shared_ptr<Planet> planet)
		: mSoldierShader(
			(Utils::Paths::ProjDir + "assets\\shaders\\cube_vertex.glsl").c_str(),
			(Utils::Paths::ProjDir + "assets\\shaders\\cube_fragment.glsl").c_str()
		),
		mPlanet(planet)
	{

	}

	void SetupRenderData();

};

