#include "missile.h"

/*
void explosion_init(GLfloat radius, GLfloat active_time, GLfloat expand_speed)
{
	for (int i = 0; i < ammo; ++i)
	{
		explosions[i].active_time = active_time;
		explosions[i].radius = radius;
		explosions[i].expand_speed = expand_speed;
	}
}

void explode(int i, GLfloat time)
{
	for (int j = 0; j < ammo; ++j)
	{
		if (!explosions[j].active)
		{
			explosions[j].t_expl = time;
			explosions[j].trans = T(missiles[i].missile_vec.x, missiles[i].missile_vec.y, missiles[i].missile_vec.z);
			explosions[j].active = true;
			missiles[i].exploded = 1;
			return;
		}
	}
}

void fire_missile(int x, int y, vec3 rpg_vec, vec3 go_dir, mat4 rpg_rot)
{
	for (int i = 0; i < ammo; ++i)
	{
		if (!missiles[i].active)
		{
			missiles[i].missile_vec = rpg_vec;
			missiles[i].missile_direction = go_dir;
			missiles[i].rot = Mult(rpg_rot, Ry(M_PI));
			missiles[i].active = 1;
			return;
		}
	}
}

void terrain_collision(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, GLfloat time)
{
	for (int i = 0; i < ammo; ++i)
	{
		if (outside_map(missiles[i].missile_vec.x, missiles[i].missile_vec.z, terrain_scale, terrain_pos) || missiles[i].missile_vec.y <=
			findHeight(missiles[i].missile_vec.x, missiles[i].missile_vec.z, tex, vertexArray, terrain_pos, terrain_scale))
		{
			if (missiles[i].active && !missiles[i].exploded)
			{
				explode(i, time);
			}
		}
	}
}

void recharge_explosions(int i, GLfloat time)
{
	GLfloat delta_t = time - explosions[i].t_expl;
	if (delta_t >= explosions[i].active_time)
	{
		explosions[i].active = false;
	}
}

bool reload_missile(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, GLfloat t_reload_start, GLfloat t)
{
	if (t - t_reload_start >= 10)
	{
		for (int i = 0; i < ammo; ++i)
		{
			if ((missiles[i].active == 1) && (outside_map(missiles[i].missile_vec.x, missiles[i].missile_vec.z, terrain_scale, terrain_pos) || missiles[i].missile_vec.y <
				findHeight(missiles[i].missile_vec.x, missiles[i].missile_vec.z, tex, vertexArray, terrain_pos, terrain_scale)))
			{
				missiles[i].active = 0;
				missiles[i].exploded = 0;
				return true;
			}
		}

	}
	return false;
}

*/