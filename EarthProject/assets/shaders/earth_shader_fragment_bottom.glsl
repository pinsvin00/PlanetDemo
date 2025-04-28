#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
in vec2 TexCoord;
uniform sampler2D planetBottomMap;

void main()
{
	FragColor = texture(planetBottomMap, TexCoord);
	BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}