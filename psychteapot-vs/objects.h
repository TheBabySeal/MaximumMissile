#pragma once

#include <windows.h>
#include <mmsystem.h>
#include "MicroGlut.h"
// uses framework Cocoa
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "extrafunctions.h"
#include "simplefont\simplefont.h"

#define map_size 255
#define terrain_scale_ 10
#define bush_scale_ 100
#define number_of_enemies 5
#define enemy_width 15
#define enemy_height 85
#define tank_width 60
#define tank_height 75
#define enemy_scale 1.0
#define enemy0_scale 0.5
#define enemy_movementspeed 0.05
#define tank_movementspeed 0.02
#define ammo 5
#define mines 4
#define enemy_life 1
#define tank_life 3
#define number_of_levels 10
#define mine_reload_time 20
#define window_width 1400
#define window_height 700

struct game
{
	GLint running;
	GLfloat width, height;
};

struct player
{
	GLint life, level, 
		reloading_mines, active_weapon; //1 - rocketlauncher, 2 - landmine
	GLfloat height, level_delay, delay_time;
};

struct level
{
	GLint enemies, life[2], completed, enemies_left;
	GLfloat enemy_ms, tank_ms;
};

struct fern
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

struct enemy
{
	vec3 position, width_vec, direction;
	GLfloat width, height, angle, movement_speed, life;
	mat4 trans, rot, total;
	int active, type; //type: 0 - enemy, 1 - tank
};

struct missile
{
	mat4 rot, trans, total;
	int active, exploded;
	vec3 position, direction;
	GLfloat width, height, reload_time;
};

struct explosion
{
	mat4 rot, trans, total;
	vec3 position;
	GLfloat t_expl;
	GLfloat radius, angle;
	GLfloat active_time;
	GLfloat expand_speed;
	bool active;
	GLint type; // type: 0 - missile, 1 - landmine
};

// Allocate memory for all arrays.
struct enemy enemies[number_of_levels * number_of_enemies];
struct explosion explosions[ammo];
struct explosion mine_explosions[mines];
struct missile missiles[ammo];
struct missile landmines[mines];
struct fern ferns[map_size * terrain_scale_ / bush_scale_][map_size * terrain_scale_ / bush_scale_];
struct player player;
struct level levels[number_of_levels];
struct game game;

void init_game();

void init_player();

void init_levels();

void init_enemies();

vec3 generate_random_spawn(vec3 player_pos);

void spawn_enemy(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, vec3 player_pos);

bool enemy_collision(vec3 pos);

void move_enemies(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, vec3 player_pos, GLfloat delta_t);

void explosion_init(GLfloat radius, GLfloat active_time, GLfloat expand_speed);

void explode(int i, GLfloat time);

void fire_missile(int x, int y, vec3 rpg_vec, vec3 go_dir, mat4 rpg_rot);

void terrain_collision(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, GLfloat time);

void recharge_explosions(int i, GLfloat time, struct explosion explosions[]);

bool reload_missile(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, GLfloat t_reload_start, GLfloat t);

void explosion_collision(int i, struct explosion explosions[]);

void check_dead_enemies();

void check_player_life(vec3 player_pos);

void check_level();

bool delay_level(GLfloat time);

void place_landmine(vec3 position, vec3 go_dir, TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale);

void step_on_mine(GLfloat t);

void trigger_landmine(GLint i, GLfloat t);

void reload_landmines(GLfloat t);