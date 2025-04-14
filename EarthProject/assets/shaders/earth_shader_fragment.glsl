#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

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

uniform vec3 viewPos;
uniform Light light; 

void main()
{
    // ambient
    vec3 ambient = light.ambient * vec3(1,1,1);
  	
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

	vec4 textureRGBA = texture(redPlanetMap, TexCoord);
	vec4 textureTerrainRGBA = texture(terrainMapTexture, TexCoord);

	vec4 halfResult;

	bool isWater = false;
	int saturation = int(textureRGBA.x * 255);
	if(saturation == 129)
	{
		halfResult = vec4(1,0.5,0,1);
	}
	else if(saturation == 200)
	{
		halfResult = vec4(0,0.2,1,1);
	}
	else if(textureRGBA.x > 0.1)
	{
		//water basin
		isWater = true;
		halfResult = vec4(0,0,1,0.82);
	}
	else
	{
		halfResult = vec4(0,1,0,1);
	}

	if(!isWater)
	{
		FragColor = vec4(ambient + diffuse, 1.0) * mix(halfResult, textureTerrainRGBA, 0.5);
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