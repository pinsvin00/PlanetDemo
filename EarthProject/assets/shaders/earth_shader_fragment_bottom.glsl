#version 330 core
out vec4 FragColor;
in vec2 TexCoord;


uniform sampler2D planetBottomMap;

void main()
{
	FragColor = texture(planetBottomMap, TexCoord);
}