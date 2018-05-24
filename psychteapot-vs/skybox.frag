#version 150

out vec4 outColor;
in vec2 texCoord;
uniform sampler2D tex;

in vec3 Normal;
in vec3 vert_pos;
uniform vec3 skyColor;

uniform vec3 cam_pos;

const float density = 0.0014;
const float gradient = 0.5;
const float lowerLimit = 30.0;
const float upperLimit = 70.0;

void main(void)
{	
	/*float factor = (vert_pos.y - lowerLimit) / (upperLimit - lowerLimit);
	factor = clamp(factor, 0.0, 1.0);
	vec4 totalColor = texture(tex, texCoord);
	outColor = mix(vec4(skyColor, 1.0), totalColor, factor);*/
	outColor = texture(tex, texCoord);
}
