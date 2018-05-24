#pragma once

/*
#include <windows.h>
#include "MicroGlut.h"
// uses framework Cocoa
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "extrafunctions.h"

#define ammo 5

struct missile
{
	mat4 rot, trans, total;
	int active, exploded;
	vec3 missile_vec, missile_direction;
};

struct explosion
{
	mat4 rot, trans, total;
	vec3 vec;
	GLfloat t_expl;
	GLfloat radius;
	GLfloat active_time;
	GLfloat expand_speed;
	bool active;
};

struct explosion explosions[ammo];
struct missile missiles[ammo];

void explosion_init(GLfloat radius, GLfloat active_time, GLfloat expand_speed);

void explode(int i, GLfloat time);

void fire_missile(int x, int y, vec3 rpg_vec, vec3 go_dir, mat4 rpg_rot);

void terrain_collision(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, GLfloat time);

void recharge_explosions(int i, GLfloat time);

bool reload_missile(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, GLfloat t_reload_start, GLfloat t);

*/