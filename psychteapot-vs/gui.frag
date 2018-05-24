#version 150

out vec4 outColor;
in vec2 texCoord;
uniform sampler2D tex;
in vec3 Normal;
in vec3 position;
in vec3 vert_pos;

void main(void)
{
	vec4 totalColor = texture(tex, texCoord);

	if(totalColor.r > 0.9 && totalColor.g > 0.9 && totalColor.b > 0.9)
	{
		discard;
	}
	outColor = texture(tex, texCoord);
}
