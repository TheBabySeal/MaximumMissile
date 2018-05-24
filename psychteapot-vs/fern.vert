#version 150

in  vec3 inPosition;
in  vec3 inNormal;
in vec2 inTexCoord;
out vec2 texCoord;
out vec3 Normal;
out vec3 position;
out vec3 vert_pos;
uniform mat4 model_to_world;
uniform mat4 world_to_view;

// NY
uniform mat4 projMatrix;
//uniform mat4 mdlMatrix;

void main(void)
{
	mat3 normalMatrix1 = mat3(model_to_world);
	Normal = normalMatrix1*inNormal;
	texCoord = inTexCoord;
	vert_pos = vec3(model_to_world * vec4(inPosition, 1.0));
	gl_Position = projMatrix * world_to_view * model_to_world * vec4(inPosition, 1.0);
	position = vec3(gl_Position);
}
