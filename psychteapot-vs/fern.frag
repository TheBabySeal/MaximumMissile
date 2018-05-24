#version 150

out vec4 outColor;
in vec2 texCoord;
uniform sampler2D tex;
in vec3 Normal;
in vec3 position;
in vec3 vert_pos;

//Light
vec3 lightSourcesDirPosArr = vec3(0.0, 0.5, -0.5);
vec3 lightSourcesDirPosArr2 = vec3(0.0, 0.5, 0.5);
vec3 lightSourcesDirPosArr3 = vec3(1.0, 1.0, 0.0);
vec3 lightSourcesDirPosArr4 = vec3(-1.0, 1.0, 0.0);

vec3 lightSourcesColorArr = vec3(1.0f, 1.0f, 1.0f);
float specularExponent = 128.0;
uniform vec3 cam_pos;

//Fog
uniform vec3 skyColor;
const float density = 0.0014;
const float gradient = 0.8;

float calcDirLight(vec3 lightPos)
{
	vec3 s;
	s = normalize(lightPos);
	vec3 n = normalize(Normal);

	float NdotS= max(dot(n,s), 0.0);
	return NdotS;
}


float calcSpec(vec3 lightPos)
{
	if(calcDirLight(lightPos) > 0.0)
	{
		vec3 s = normalize(lightPos - vert_pos);
		vec3 n = normalize(Normal);
		vec3 r = normalize(2*(dot(s, n) * n - s));
		vec3 eye = normalize(cam_pos - vert_pos);

		float spec = max(pow(dot(r, eye), specularExponent)*128, 0.0);
		return spec;
	}
	else
	{
		return 0.0;
	}
}


void main(void)
{
	float distance = length(vert_pos - cam_pos);
	float visibility = exp(-pow((distance*density), gradient));
	vec3 currLight = lightSourcesDirPosArr;

	vec3 currColor = lightSourcesColorArr;

	vec4 totalColor = texture(tex, texCoord);
	if(totalColor.a < 0.5)
	{
		discard;
	}
	outColor = 0.5*(calcDirLight(currLight) + calcDirLight(lightSourcesDirPosArr2))*totalColor; //  + calcDirLight(lightSourcesDirPosArr3) + calcDirLight(lightSourcesDirPosArr4)
	//outColor = mix(vec4(skyColor, 1.0), outColor, visibility);
}
