#pragma once

bool check_proximity(GLfloat a, GLfloat b, GLfloat prox);

int RandomNumber(const int nMin, const int nMax);

bool outside_map(int x, int z, GLfloat terrain_scale, vec3 terrain_pos);

vec3 get_vertex(int x, int z, int width, GLfloat *vertexArray);

vec3* findQuad(GLfloat x, GLfloat z, TextureData *tex, GLfloat *vertexArray);

vec3* findTriangle(GLfloat x, GLfloat z, TextureData *tex, GLfloat *vertexArray);

GLfloat findHeight(GLfloat x, GLfloat z, TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale);

Model* GenerateTerrain(TextureData *tex);