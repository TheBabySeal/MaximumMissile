
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "MicroGlut.h"
// uses framework Cocoa
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "extrafunctions.h"

#define map_size 255

bool check_proximity(GLfloat a, GLfloat b, GLfloat prox)
{
	if (a < b - prox || a > b + prox)
	{
		return false;
	}
	else return true;
}

// the random function
int RandomNumber(const int nMin, const int nMax)
{
	int Number = rand() % (nMax - nMin) + nMin;
	return Number;
}

bool outside_map(int x, int z, GLfloat terrain_scale, vec3 terrain_pos)
{
	return ((x <= terrain_pos.x) ||
		(z <= terrain_pos.z) ||
		(x - terrain_pos.x >= map_size * terrain_scale) ||
		(z - terrain_pos.z >= map_size * terrain_scale));
}

vec3 get_vertex(int x, int z, int width, GLfloat *vertexArray)
{
	vec3* temp = malloc(sizeof(GLfloat) * 3);
	temp[0].x = vertexArray[(x + z * width) * 3 + 0];
	temp[0].y = vertexArray[(x + z * width) * 3 + 1];
	temp[0].z = vertexArray[(x + z * width) * 3 + 2];

	return temp[0];
}

// Find the Vertices surrounding the current vertix
vec3* findQuad(GLfloat x, GLfloat z, TextureData *tex, GLfloat *vertexArray)
{
	vec3 *quad = malloc(sizeof(GLfloat) * 3 * 4);

	if (x == 0 || z == 0 || x == tex->width || z == tex->width * 3)
	{

	}
	else
	{
		quad[0] = get_vertex((int)floor(x), (int)floor(z), tex->width, vertexArray);
		quad[1] = get_vertex((int)floor(x + 1), (int)floor(z), tex->width, vertexArray);
		quad[2] = get_vertex((int)floor(x), (int)floor(z + 1), tex->width, vertexArray);
		quad[3] = get_vertex((int)floor(x + 1), (int)floor(z + 1), tex->width, vertexArray);
	}

	return quad;
}

// Find the triangle from the vertices surrounding the relevant vertix
vec3* findTriangle(GLfloat x, GLfloat z, TextureData *tex, GLfloat *vertexArray)
{
	vec3* quad = findQuad(x, z, tex, vertexArray);
	vec3* triangle = malloc(sizeof(GLfloat) * 3 * 3);
	vec3 lowerleft = quad[0];
	vec3 upperright = quad[3];
	GLfloat distLL = sqrt(pow(lowerleft.x - x, 2) + pow(lowerleft.z - z, 2));
	GLfloat distUR = sqrt(pow(upperright.x - x, 2) + pow(upperright.z - z, 2));
	if (distLL < distUR)
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

// Find the y-value of the current triangle
GLfloat findHeight(GLfloat x, GLfloat z, TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale)
{
	x = x / terrain_scale - terrain_pos.x / terrain_scale;
	z = z / terrain_scale - terrain_pos.z / terrain_scale;
	vec3* triangle = findTriangle(x, z, tex, vertexArray);
	vec3 normal_temp = CrossProduct(VectorSub(triangle[1], triangle[0]), VectorSub(triangle[2], triangle[0]));
	GLfloat a = normal_temp.x;
	GLfloat b = normal_temp.y;
	GLfloat c = normal_temp.z;
	GLfloat d = a * triangle[0].x + b * triangle[0].y + c * triangle[0].z;

	GLfloat height = (d - a * x - c * z) / b;

	return height * terrain_scale;
}


Model* GenerateTerrain(TextureData *tex)
{
	int vertexCount = tex->width * tex->height;
	int triangleCount = (tex->width - 1) * (tex->height - 1) * 2;
	int x, z;

	GLfloat *vertexArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *normalArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *texCoordArray = malloc(sizeof(GLfloat) * 2 * vertexCount);
	GLuint *indexArray = malloc(sizeof(GLuint) * triangleCount * 3);

	printf("bpp %d\n", tex->bpp);
	for (x = 0; x < tex->width; x++)
		for (z = 0; z < tex->height; z++)
		{
			// Vertex array. You need to scale this properly
			vertexArray[(x + z * tex->width) * 3 + 0] = x / 1.0;
			vertexArray[(x + z * tex->width) * 3 + 1] = tex->imageData[(x + z * tex->width) * (tex->bpp / 8)] / 20.0;
			vertexArray[(x + z * tex->width) * 3 + 2] = z / 1.0;
			// // Normal vectors. You need to calculate these.
			// 			normalArray[(x + z * tex->width)*3 + 0] = 0.0;
			// 			normalArray[(x + z * tex->width)*3 + 1] = 1.0;
			// 			normalArray[(x + z * tex->width)*3 + 2] = 0.0;
			// Texture coordinates. You may want to scale them.
			texCoordArray[(x + z * tex->width) * 2 + 0] = x; // (float)x / tex->width;
			texCoordArray[(x + z * tex->width) * 2 + 1] = z; // (float)z / tex->height;
		}
	for (x = 0; x < tex->width; x++)
		for (z = 0; z < tex->height; z++)
		{
			if (x == 0 || z == 0 || x == tex->width || z == tex->width * 3)
			{
				// Use a default normal (Tricky to calc)
				normalArray[(x + z * tex->width) * 3 + 0] = 0.0;
				normalArray[(x + z * tex->width) * 3 + 1] = 0.0;
				normalArray[(x + z * tex->width) * 3 + 2] = 0.5;
			}
			else
			{
				vec3 top = get_vertex(x, z - 1, tex->width, vertexArray);
				vec3 left = get_vertex(x - 1, z + 1, tex->width, vertexArray);
				vec3 right = get_vertex(x + 1, z + 1, tex->width, vertexArray);

				vec3 vec1 = VectorSub(left, top);
				vec3 vec2 = VectorSub(left, right);

				vec3 final = CrossProduct(vec1, vec2);

				normalArray[(x + z * tex->width) * 3 + 0] = final.x;
				normalArray[(x + z * tex->width) * 3 + 1] = final.y;
				normalArray[(x + z * tex->width) * 3 + 2] = final.z;
			}

		}

	for (x = 0; x < tex->width - 1; x++)
		for (z = 0; z < tex->height - 1; z++)
		{
			// Triangle 1
			indexArray[(x + z * (tex->width - 1)) * 6 + 0] = x + z * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 1] = x + (z + 1) * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 2] = x + 1 + z * tex->width;
			// Triangle 2
			indexArray[(x + z * (tex->width - 1)) * 6 + 3] = x + 1 + z * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 4] = x + (z + 1) * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 5] = x + 1 + (z + 1) * tex->width;
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
		triangleCount * 3);

	return model;
}