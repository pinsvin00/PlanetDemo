#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D heightMap;

void main()
{
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);

	Normal = aNormal;

	vec4 heightData = texture(heightMap, aTexCoord);
	float r = heightData.x * 0.05;
	vec3 res = aPos + aNormal * r;
	FragPos = vec3(model * vec4(res, 1.0));
	gl_Position = projection * view * model * vec4(res, 1.0f);

}