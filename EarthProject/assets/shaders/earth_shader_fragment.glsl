#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 SpherePos;
uniform vec3 countryIdToColorMap[255];

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform sampler2D redPlanetMap;
uniform sampler2D terrainMapTexture;
uniform sampler2D dudvMap;
uniform sampler2D waterTexture;
uniform float time;
uniform int displayMode;

uniform vec3 viewPos;
uniform Light light; 

float near = 0.1; 
float far  = 100.0; 
  
void main()
{
    // ambient
    vec3 ambient = light.ambient * vec3(1,1,1);
  	
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;    
	
	vec4 textureRGBA = texture(redPlanetMap, TexCoord);
	vec4 textureTerrainRGBA = texture(terrainMapTexture, TexCoord);

	vec4 halfResult = vec4(0,1,0,1);
	bool isWater = false;
	bool isBorder = false;
	int saturation = int(textureRGBA.x * 255);
	if(saturation == 129)
	{
		halfResult = vec4(1,0.5,0,1);
	}
	else if(saturation == 200)
	{
		halfResult = vec4(0,0.2,1,1);
	}
	else if(saturation <= 33)
	{
		isBorder = true;
	}
	else if(saturation == 255)
	{
		halfResult = vec4(0,1,0,1);
	}
	else if(saturation == 71)
	{
		isWater = true;
		halfResult = vec4(0,0,1,0.82);
	}

	if(displayMode == 0)
	{
		if(isBorder)
		{
			FragColor = vec4(0,0,0,1);
		}
		else if(!isWater)
		{
			FragColor = vec4(ambient + diffuse, 1.0) * mix(halfResult, textureTerrainRGBA, 0.0);
		}
		else
		{
			float scaledTime = time * 0.001;
			vec2 distortion = texture(dudvMap, TexCoord + vec2(scaledTime, 0.0)).rg * 2.0 - 1.0;
			distortion *= 0.02;
			vec2 distortedTexCoords = TexCoord + distortion;
			vec4 color = texture(waterTexture, distortedTexCoords);
			FragColor = vec4(ambient + diffuse, 1.0) * color;
		}
	}

	BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}