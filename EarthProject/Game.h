#pragma once
#include <memory>
#include "Render.h"
#include "Renderer.h"

class Game
{
	//Backend and other game logics responsible class
	//self
	//Code excluesively for the GPU, and the window interaction
	std::shared_ptr<Renderer> renderer = nullptr;

public:
	Game()
	{
		renderer = Renderer::GetInstance();
		renderer->Init();
	}

	void OnTick()
	{

	}

};

