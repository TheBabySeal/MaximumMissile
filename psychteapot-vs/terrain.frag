#version 150

out vec4 outColor;
in vec2 texCoord;
//uniform sampler2D tex;
uniform sampler2D blendMap;
uniform sampler2D background;
uniform sampler2D rtex;
uniform sampler2D gtex;
uniform sampler2D btex;

in vec3 Normal;
in vec3 position;
in vec3 vert_pos;
uniform float terrain_scale;
uniform vec3 skyColor;
//Light
vec3 lightSourcesDirPosArr = vec3(0.0, -1.0, 1.0);

vec3 lightSourcesColorArr = vec3(1.0f, 1.0f, 1.0f);
float specularExponent = 16.0;
uniform vec3 cam_pos;

float calcDirLight(vec3 lightPos)
{
	vec3 s;
	s = normalize(lightPos);
	vec3 n = normalize(Normal);

	float NdotS= max(dot(n,s),0.0);
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

		float spec = max(pow(dot(r, eye), specularExponent)*64, 0.0);
		return spec;
	}
	else
	{
		return 0.0;
	}
}

const float density = 0.0014;
const float gradient = 0.8;

void main(void)
{

	vec4 blendMapColor = texture(blendMap, 0.005*texCoord);
	float backTextureAmount = 1 - (blendMapColor.r + blendMapColor.g + blendMapColor.b);
	vec2 tiledCoord = texCoord * 0.2;
	vec4 backgroundColor = texture(background, tiledCoord) * backTextureAmount;
	vec4 rColor = texture(rtex, 1.0*tiledCoord) * blendMapColor.r;
	vec4 gColor = texture(gtex, tiledCoord) * blendMapColor.g;
	vec4 bColor = texture(btex, 0.8*tiledCoord) * blendMapColor.b;
	
	vec4 totalColor = backgroundColor + rColor + gColor + bColor;
	
	float distance = length(vert_pos - cam_pos);
	float visibility = exp(-pow((distance*density), gradient));
	vec3 currLight = lightSourcesDirPosArr;

	vec3 currColor = lightSourcesColorArr;
	
	outColor = 0.5*calcDirLight(currLight)*totalColor;
	//outColor = mix(vec4(skyColor, 1.0), outColor, visibility);

}
