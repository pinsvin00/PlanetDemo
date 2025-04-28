#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;  

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform vec3 viewPos;
uniform Light light; 

void main()
{
    float specularStrength = 0.5;
    vec3 lightDir = normalize(light.position - FragPos);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);  
    float spec = pow(max(1.0 - dot(viewDir, reflectDir), 0.0), 32);
    
    vec3 specular = specularStrength * (1.0 - spec) * vec3(1.0);  
    vec4 atmosphereColor = vec4(123.0/255.0, 148.0/255.0, 209.0/255.0, 0.4);
    
    if(specular.x < 0.1)
    {
        discard;
    }

	FragColor = mix(vec4(specular, 1.0), atmosphereColor, 0.8);
    FragColor.w = 0.8;
    
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.2)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

}