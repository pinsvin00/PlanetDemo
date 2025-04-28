#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;


in vec2 TexCoord;

// texture samplers
uniform sampler2D texture;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = vec4(1,1,0,1);
	BrightColor = vec4(1.0, 1.0, 0.0, 1.0);
}