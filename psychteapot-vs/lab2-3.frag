#version 150

out vec4 out_Color;
in vec3 OutColor;
in vec3 out_Normal;
in vec2 outTexCoord;
uniform sampler2D texUnit;

void main(void)
{
	float a = outTexCoord.s;
	float b = outTexCoord.t;
	//out_Color = vec4(0.0, a, 0.0, 0.0);
	out_Color = texture(texUnit, outTexCoord);
}
