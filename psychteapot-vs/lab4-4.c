// Lab 4, terrain generation


// RIP, byte från model trans till kamera trans.

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	// Linking hint for Lightweight IDE
	// uses framework Cocoa
#endif
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"

mat4 projectionMatrix, model_trans, model_rot;

mat4 total_terrain, trans_terrain, rot_terrain;

// vertex array object
Model *m, *m2, *tm, *octagon;
// Reference to shader program
GLuint program;
GLuint tex1, tex2;
TextureData ttex; // terrain

mat4 total, camMatrix, modelView, cam_rot, cam_total, rot_x, rot_y, cam_trans, oct_trans, oct_total;
vec3 oct_position = {100, 45, 100};
vec3 cam = {0, 20, 0};
vec3 lookAtPoint = {0, 20, 1};


vec3 get_vertex(int x, int z, int width, GLfloat *vertexArray)
{
	vec3 temp;
	temp.x = vertexArray[(x + z * width)*3 + 0];
	temp.y = vertexArray[(x + z * width)*3 + 1];
	temp.z = vertexArray[(x + z * width)*3 + 2];

	return temp;
}

Model* GenerateTerrain(TextureData *tex)
{
	int vertexCount = tex->width * tex->height;
	int triangleCount = (tex->width-1) * (tex->height-1) * 2;
	int x, z;

	GLfloat *vertexArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *normalArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *texCoordArray = malloc(sizeof(GLfloat) * 2 * vertexCount);
	GLuint *indexArray = malloc(sizeof(GLuint) * triangleCount*3);

	printf("bpp %d\n", tex->bpp);
	for (x = 0; x < tex->width; x++)
		for (z = 0; z < tex->height; z++)
		{
// Vertex array. You need to scale this properly
			vertexArray[(x + z * tex->width)*3 + 0] = x / 1.0;
			vertexArray[(x + z * tex->width)*3 + 1] = tex->imageData[(x + z * tex->width) * (tex->bpp/8)] / 20.0;
			vertexArray[(x + z * tex->width)*3 + 2] = z / 1.0;
// // Normal vectors. You need to calculate these.
// 			normalArray[(x + z * tex->width)*3 + 0] = 0.0;
// 			normalArray[(x + z * tex->width)*3 + 1] = 1.0;
// 			normalArray[(x + z * tex->width)*3 + 2] = 0.0;
// Texture coordinates. You may want to scale them.
			texCoordArray[(x + z * tex->width)*2 + 0] = x; // (float)x / tex->width;
			texCoordArray[(x + z * tex->width)*2 + 1] = z; // (float)z / tex->height;
		}
	for (x = 0; x < tex->width; x++)
		for (z = 0; z < tex->height; z++)
		{
			if (x == 0 || z == 0 || x == tex->width || z == tex->width*3)
			{
				// Use a default normal (Tricky to calc)
				normalArray[(x + z * tex->width)*3 + 0] = 0.0;
				normalArray[(x + z * tex->width)*3 + 1] = 1.0;
				normalArray[(x + z * tex->width)*3 + 2] = 0.0;
			}
			else
			{
				vec3 top = get_vertex(x, z - 1, tex->width, vertexArray);
				vec3 left = get_vertex(x - 1, z + 1, tex->width, vertexArray);
				vec3 right = get_vertex(x + 1, z + 1, tex->width, vertexArray);

				vec3 vec1 = VectorSub(left, top);
				vec3 vec2 = VectorSub(left, right);

				vec3 final = CrossProduct(vec1, vec2);

				normalArray[(x + z * tex->width)*3 + 0] = final.x;
				normalArray[(x + z * tex->width)*3 + 1] = final.y;
				normalArray[(x + z * tex->width)*3 + 2] = final.z;
			}


	// Normal vectors. You need to calculate these.

		}

	for (x = 0; x < tex->width-1; x++)
		for (z = 0; z < tex->height-1; z++)
		{
		// Triangle 1
			indexArray[(x + z * (tex->width-1))*6 + 0] = x + z * tex->width;
			indexArray[(x + z * (tex->width-1))*6 + 1] = x + (z+1) * tex->width;
			indexArray[(x + z * (tex->width-1))*6 + 2] = x+1 + z * tex->width;
		// Triangle 2
			indexArray[(x + z * (tex->width-1))*6 + 3] = x+1 + z * tex->width;
			indexArray[(x + z * (tex->width-1))*6 + 4] = x + (z+1) * tex->width;
			indexArray[(x + z * (tex->width-1))*6 + 5] = x+1 + (z+1) * tex->width;
		}

	// End of terrain generation

	// Create Model and upload to GPU:

	Model* model = LoadDataToModel(
			vertexArray,
			normalArray,
			texCoordArray,
			NULL,
			indexArray,
			vertexCount,
			triangleCount*3);

	return model;
}

vec3* findQuad(GLfloat x, GLfloat z, TextureData *tex)
{
	vec3 *quad = malloc(sizeof(GLfloat) * 3 * 4);

	if (x == 0 || z == 0 || x == tex->width || z == tex->width*3)
	{

	}
	else
	{
		quad[0] = get_vertex((int)floor(x), (int)floor(z), tex->width, tm->vertexArray);
		quad[1] = get_vertex((int)floor(x + 1), (int)floor(z), tex->width, tm->vertexArray);
		quad[2] = get_vertex((int)floor(x), (int)floor(z + 1), tex->width, tm->vertexArray);
		quad[3] = get_vertex((int)floor(x + 1), (int)floor(z + 1), tex->width, tm->vertexArray);
	}

	return quad;
}

vec3* findTriangle(vec3* quad, GLfloat x, GLfloat z)
{
	vec3* triangle = malloc(sizeof(GLfloat) * 3 * 3);
	vec3 lowerleft = quad[0];
	vec3 upperright = quad[3];
	GLfloat distLL = sqrt(pow(lowerleft.x-x,2) + pow(lowerleft.z-z,2));
	GLfloat distUR = sqrt(pow(upperright.x-x,2) + pow(upperright.z-z,2));
	if(distLL < distUR)
	{
		triangle[0] = quad[0];
		triangle[1] = quad[1];
		triangle[2] = quad[2];
 	}
	else
	{
		triangle[0] = quad[3];
		triangle[1] = quad[1];
		triangle[2] = quad[2];
	}
	return triangle;
}

GLfloat findHeight(vec3* triangle, GLfloat x, GLfloat z)
{
	vec3 normal_temp = CrossProduct(VectorSub(triangle[1], triangle[0]), VectorSub(triangle[2], triangle[0]));
	GLfloat a = normal_temp.x;
	GLfloat b = normal_temp.y;
	GLfloat c = normal_temp.z;
	GLfloat d = a*triangle[0].x + b*triangle[0].y + c*triangle[0].z;

	GLfloat height = (d - a * x - c * z) / b;

	return height;
}



void init(void)
{
	// GL inits
	glClearColor(0.2,0.2,0.5,0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	printError("GL inits");

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 500.0);

	model_rot = Ry(0);
	model_trans = T(0,0,0);

	octagon = LoadModelPlus("octagon.obj");

	// Load and compile shader
	program = loadShaders("terrain.vert", "terrain.frag");
	glUseProgram(program);
	printError("init shader");

	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(program, "tex"), 0); // Texture unit 0
	LoadTGATextureSimple("maskros512.tga", &tex1);

	// Load terrain data
	LoadTGATextureData("fft-terrain.tga", &ttex);
	tm = GenerateTerrain(&ttex);
	printError("init terrain");

	camMatrix = lookAt(cam.x, cam.y, cam.z,
				lookAtPoint.x, lookAtPoint.y, lookAtPoint.z,
				0.0, 1.0, 0.0);

	vec3* temp_quad;
	vec3* temp_triangle;

	temp_quad = findQuad(oct_position.x, oct_position.z, &ttex);
	temp_triangle = findTriangle(temp_quad, oct_position.x, oct_position.z);
	oct_position.y = findHeight(temp_triangle, oct_position.x, oct_position.z);
	oct_trans = T(oct_position.x, oct_position.y, oct_position.z);

}

void key_press()
{
	float step_size = 0.3;

	// Calculate vectors
	vec3 go_dir = Normalize(VectorSub(lookAtPoint, cam));
	vec3 up_vec = {0.0, 1.0, 0.0};
	vec3 lookAtVec = VectorSub(lookAtPoint, cam);
	vec3 left_vec = CrossProduct(go_dir, up_vec);
	// Movement and Rotation
	if(glutKeyIsDown('w'))
	{
		cam.x = cam.x + go_dir.x*step_size;
		cam.y = cam.y + go_dir.y*step_size;
		cam.z = cam.z + go_dir.z*step_size;
		lookAtPoint.x = lookAtPoint.x + go_dir.x*step_size;
		lookAtPoint.y = lookAtPoint.y + go_dir.y*step_size;
		lookAtPoint.z = lookAtPoint.z + go_dir.z*step_size;
	}
	if(glutKeyIsDown('a'))
	{
		cam_rot = ArbRotate(up_vec, 0.2*step_size);

		lookAtVec = MultVec3(cam_rot, lookAtVec);

		lookAtPoint = VectorAdd(lookAtVec, cam);
	}
	if(glutKeyIsDown('s'))
	{
		cam.x = cam.x - go_dir.x*step_size;
		cam.y = cam.y - go_dir.y*step_size;
		cam.z = cam.z - go_dir.z*step_size;
		lookAtPoint.x = lookAtPoint.x - go_dir.x*step_size;
		lookAtPoint.y = lookAtPoint.y - go_dir.y*step_size;
		lookAtPoint.z = lookAtPoint.z - go_dir.z*step_size;
	}
	if(glutKeyIsDown('d'))
	{
		cam_rot = ArbRotate(up_vec, -0.2*step_size);

		lookAtVec = MultVec3(cam_rot, lookAtVec);

		lookAtPoint = VectorAdd(lookAtVec, cam);
	}

	if(glutKeyIsDown('e'))
	{
		cam_rot = ArbRotate(left_vec, 0.2*step_size);

		lookAtVec = MultVec3(cam_rot, lookAtVec);

		lookAtPoint = VectorAdd(lookAtVec, cam);
	}

	if(glutKeyIsDown('f'))
	{
		cam_rot = ArbRotate(left_vec, -0.2*step_size);

		lookAtVec = MultVec3(cam_rot, lookAtVec);

		lookAtPoint = VectorAdd(lookAtVec, cam);
	}

	// Move object
	if(glutKeyIsDown('i'))
	{
		vec3* temp_quad;
		vec3* temp_triangle;
		GLfloat y;
		oct_position.z += 0.5*step_size;
		temp_quad = findQuad(oct_position.x, oct_position.z, &ttex);
		temp_triangle = findTriangle(temp_quad, oct_position.x, oct_position.z);
		y = findHeight(temp_triangle, oct_position.x, oct_position.z);

		oct_position.y = y;
		oct_trans = T(oct_position.x, oct_position.y, oct_position.z);
	}

	if(glutKeyIsDown('k'))
	{
		vec3* temp_quad;
		vec3* temp_triangle;
		GLfloat y;
		oct_position.z -= 0.5*step_size;
		temp_quad = findQuad(oct_position.x, oct_position.z, &ttex);
		temp_triangle = findTriangle(temp_quad, oct_position.x, oct_position.z);
		y = findHeight(temp_triangle, oct_position.x, oct_position.z);

		oct_position.y = y;
		oct_trans = T(oct_position.x, oct_position.y, oct_position.z);
	}
	if(glutKeyIsDown('j'))
	{
		vec3* temp_quad;
		vec3* temp_triangle;
		GLfloat y;
		oct_position.x -= 0.5*step_size;
		temp_quad = findQuad(oct_position.x, oct_position.z, &ttex);
		temp_triangle = findTriangle(temp_quad, oct_position.x, oct_position.z);
		y = findHeight(temp_triangle, oct_position.x, oct_position.z);

		oct_position.y = y;
		oct_trans = T(oct_position.x, oct_position.y, oct_position.z);
	}
	if(glutKeyIsDown('l'))
	{
		vec3* temp_quad;
		vec3* temp_triangle;
		GLfloat y;
		oct_position.x += 0.5*step_size;
		temp_quad = findQuad(oct_position.x, oct_position.z, &ttex);
		temp_triangle = findTriangle(temp_quad, oct_position.x, oct_position.z);
		y = findHeight(temp_triangle, oct_position.x, oct_position.z);

		oct_position.y = y;
		oct_trans = T(oct_position.x, oct_position.y, oct_position.z);
	}

	// Update Cam matrix
	camMatrix = lookAt(cam.x, cam.y, cam.z,
				lookAtPoint.x, lookAtPoint.y, lookAtPoint.z,
				0.0, 1.0, 0.0);
}

void display(void)
{
	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	modelView = IdentityMatrix();

	printError("pre display");

	glUseProgram(program);

	// Build matrix

	// vec3 cam = {0, 5, 8};
	// vec3 lookAtPoint = {2, 0, 2};


	key_press();

	oct_total = oct_trans;

	modelView = Mult(model_rot, model_trans);
	glUniformMatrix4fv(glGetUniformLocation(program, "model_to_world"), 1, GL_TRUE, modelView.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "world_to_view"), 1, GL_TRUE, camMatrix.m);

	glUniform3f(glGetUniformLocation(program, "cam_pos"), cam.x, cam.y, cam.z);

	glBindTexture(GL_TEXTURE_2D, tex1);		// Bind Our Texture tex1
	DrawModel(tm, program, "inPosition", "inNormal", "inTexCoord");

	glUniformMatrix4fv(glGetUniformLocation(program, "model_to_world"), 1, GL_TRUE, oct_total.m);
	DrawModel(octagon, program, "inPosition", "inNormal", "inTexCoord");

	printError("display 2");

	glutSwapBuffers();
}

void timer(int i)
{
	glutTimerFunc(20, &timer, i);
	glutPostRedisplay();
}

void mouse(int x, int y)
{
	//printf("%d %d\n", x, y);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2);
	glutInitWindowSize (600, 600);
	glutCreateWindow ("TSBK07 Lab 4");
	glutDisplayFunc(display);
	init ();
	glutTimerFunc(20, &timer, 0);

	glutPassiveMotionFunc(mouse);

	glutMainLoop();
	exit(0);
}
