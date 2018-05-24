#version 150

out vec4 outColor;
in vec2 texCoord;
uniform sampler2D tex;
in vec3 Normal;
in vec3 position;
in vec3 vert_pos;
uniform vec3 inColor;

void main(void)
{
	outColor = vec4(inColor, 1.0);

}
