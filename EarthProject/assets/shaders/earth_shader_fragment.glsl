#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform int numVectors;
uniform vec2 myVectors[256];

uniform sampler2D redPlanetMap;


bool isPointInPolygon(vec2 point) {
    bool inside = false;

    for (int i = 0; i < numVectors; i++) {
        vec2 v1 = myVectors[i];
        vec2 v2 = myVectors[(i + 1) % numVectors]; // Wrap around to the first vertex

        // Check if the point is within the y-range of the edge
        if (((v1.y > point.y) != (v2.y > point.y)) && 
            (point.x < (v2.x - v1.x) * (point.y - v1.y) / (v2.y - v1.y) + v1.x)) {
            inside = !inside; // Toggle the inside state
        }
    }

    return inside;
}

void main()
{
	vec4 textureRGBA = texture(redPlanetMap, TexCoord);
	//we got some land
	int saturation = int(textureRGBA.x * 255);

	if(saturation == 129)
	{
		FragColor = vec4(1,0.5,0,1);
	}
	else if(saturation == 200)
	{
		FragColor = vec4(0,0.2,1,1);
	}
	else if(textureRGBA.x > 0.1)
	{
		FragColor = vec4(0,0,1,1);
	}
	else
	{
		FragColor = vec4(0,1,0,1);
	}
}