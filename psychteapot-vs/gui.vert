#version 150

in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexCoord;
out vec2 texCoord;
out vec3 Normal;
out vec3 position;
out vec3 vert_pos;
uniform mat4 model_to_world;
uniform mat4 world_to_view;


void main(void)
{
	texCoord = inTexCoord;
	gl_Position = model_to_world * vec4(inPosition, 1.0);
}
