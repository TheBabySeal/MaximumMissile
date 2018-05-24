#include "objects.h"

GLuint player_life = 5;
GLfloat player_height = 30.0;

void init_game()
{
	game.width = window_width;
	game.height = window_height;
	game.running = 0;
}

void init_player()
{
	player.height = player_height;
	player.life = player_life;
	player.level = 0;
	player.level_delay = 50;
	player.delay_time = 0.0;
	player.active_weapon = 1;
	player.reloading_mines = 0;
}

void init_levels()
{
	for (int i = 0; i <= number_of_levels; ++i)
	{
		levels[i].enemies = number_of_enemies * i;
		levels[i].enemies_left = number_of_enemies * i;
		levels[i].enemy_ms = enemy_movementspeed + i * 0.01;
		levels[i].tank_ms = tank_movementspeed + i * 0.005;
		levels[i].life[0] = enemy_life;
		levels[i].life[1] = tank_life;
		levels[i].completed = 0;
	}
}

void init_enemies()
{
	for (int i = 0; i < levels[player.level].enemies; ++i)
	{
		if (i < player.level)
		{
			enemies[i].type = 1;
			enemies[i].width = tank_width;
			enemies[i].width_vec = SetVector(1.0, 0.0, 1.0);
			enemies[i].width_vec = ScalarMult(enemies[i].width_vec, enemies[i].width);
			enemies[i].height = tank_height;
			enemies[i].movement_speed = levels[player.level].tank_ms;
		}
		else
		{
			enemies[i].width = enemy_width;
			enemies[i].width_vec = SetVector(1.0, 0.0, 1.0);
			enemies[i].width_vec = ScalarMult(enemies[i].width_vec, enemies[i].width);
			enemies[i].height = enemy_height;
			enemies[i].movement_speed = levels[player.level].enemy_ms;
		}

		enemies[i].active = 0;
	}
}

void init_landmines()
{
	for (int i = 0; i < mines; ++i)
	{
		landmines[i].active = 0;
		landmines[i].width = 20;
		landmines[i].height = 10;
		landmines[i].reload_time = 0.0;
	}
}

vec3 generate_random_spawn(vec3 player_pos)
{
	vec3 spawn_pos;
	for (int i = 0; i < map_size * terrain_scale_; ++i)
	{
		spawn_pos.x = RandomNumber(50, map_size * terrain_scale_ - 50);
		spawn_pos.z = RandomNumber(50, map_size * terrain_scale_ - 50);
		if (((spawn_pos.x - player_pos.x > 200) || (spawn_pos.x - player_pos.x < -200))
			&& (spawn_pos.z - player_pos.z > 200) || (spawn_pos.z - player_pos.z < -200))
		{
			return spawn_pos;
		}
	}
}

void spawn_enemy(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, vec3 player_pos)
{
	for (int j = 0; j < levels[player.level].enemies; ++j)
	{
		if (!enemies[j].active)
		{
			enemies[j].position = generate_random_spawn(player_pos);
			enemies[j].position.y = findHeight(enemies[j].position.x, enemies[j].position.z, tex, vertexArray, terrain_pos, terrain_scale);
			enemies[j].trans = T(enemies[j].position.x, enemies[j].position.y, enemies[j].position.z);
			enemies[j].active = 1;
			enemies[j].life = levels[player.level].life[enemies[j].type];
			return;
		}
	}
}

bool enemy_collision(vec3 pos)
{
	for (int i = 0; i < levels[player.level].enemies; ++i)
	{
		if (check_proximity(pos.x, enemies[i].position.x, enemies[i].width_vec.x) && check_proximity(pos.z, enemies[i].position.z, enemies[i].width_vec.z)
			&& check_proximity(pos.y, enemies[i].position.y, enemies[i].height) && enemies[i].active)
		{
			return true;
		}
	}
	return false;
}

void move_enemies(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, vec3 player_pos, GLfloat delta_t)
{
	vec3 temp_move;
	for (int i = 0; i < levels[player.level].enemies; ++i)
	{
		enemies[i].movement_speed = levels[player.level].enemy_ms / delta_t;
		enemies[i].direction = Normalize(VectorSub(player_pos, enemies[i].position));
		temp_move = ScalarMult(enemies[i].direction, enemies[i].movement_speed);
		enemies[i].position = VectorAdd(enemies[i].position, temp_move);
		enemies[i].position.y = findHeight(enemies[i].position.x, enemies[i].position.z, tex, vertexArray, terrain_pos, terrain_scale);
		enemies[i].trans = T(enemies[i].position.x, enemies[i].position.y, enemies[i].position.z);
	}
}

void explosion_init(GLfloat radius, GLfloat active_time, GLfloat expand_speed)
{
	for (int i = 0; i < ammo; ++i)
	{
		explosions[i].active_time = active_time;
		explosions[i].radius = radius;
		explosions[i].expand_speed = expand_speed;
		explosions[i].type = 0;
	}

	for (int j = 0; j < mines; ++j)
	{
		mine_explosions[j].active_time = active_time;
		mine_explosions[j].radius = radius * 2;
		mine_explosions[j].expand_speed = mine_explosions[j].radius / mine_explosions[j].active_time;
		mine_explosions[j].type = 1;
	}
}

void explode(int i, GLfloat time)
{
	for (int j = 0; j < ammo; ++j)
	{
		if (!explosions[j].active && !missiles[i].exploded)
		{
			explosions[j].t_expl = time;
			explosions[j].position = missiles[i].position;
			explosions[j].trans = T(explosions[j].position.x, explosions[j].position.y, explosions[j].position.z);
			explosions[j].active = true;
			missiles[i].exploded = 1;

			// ½ the value of a landmine explosion
			waveOutSetVolume(0, 0x4FFF);
			PlaySound(TEXT("Sound/explosion"), NULL, SND_ASYNC | SND_NODEFAULT | SND_FILENAME);
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
			missiles[i].position = rpg_vec;
			missiles[i].direction = go_dir;
			missiles[i].rot = Mult(rpg_rot, Ry(M_PI));
			missiles[i].active = 1;
			//PlaySound(TEXT("Sound/explosion.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
			return;
		}
	}
}

void terrain_collision(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, GLfloat time)
{
	for (int i = 0; i < ammo; ++i)
	{
		if (outside_map(missiles[i].position.x, missiles[i].position.z, terrain_scale, terrain_pos) || missiles[i].position.y <=
			findHeight(missiles[i].position.x, missiles[i].position.z, tex, vertexArray, terrain_pos, terrain_scale))
		{
			if (missiles[i].active && !missiles[i].exploded)
			{
				explode(i, time);
			}
		}
	}
}

void recharge_explosions(int i, GLfloat time, struct explosion explosions[])
{
	GLfloat delta_t = time - explosions[i].t_expl;
	if (delta_t >= explosions[i].active_time)
	{
		explosion_collision(i, explosions);
		explosions[i].active = false;
	}
}

bool reload_missile(TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale, GLfloat t_reload_start, GLfloat t)
{
	if (t - t_reload_start >= 15)
	{
		for (int i = 0; i < ammo; ++i)
		{
			if ((missiles[i].active == 1) && (outside_map(missiles[i].position.x, missiles[i].position.z, terrain_scale, terrain_pos) || missiles[i].position.y <
				findHeight(missiles[i].position.x, missiles[i].position.z, tex, vertexArray, terrain_pos, terrain_scale)))
			{
				missiles[i].active = 0;
				missiles[i].exploded = 0;
				return true;
			}
		}

	}
	return false;
}

// 2D spherical collision detection between explosions and enemies
void explosion_collision(int i, struct explosion explosions[])
{
	if (explosions[i].active)
	{
		for (int j = 0; j < levels[player.level].enemies; ++j)
		{
			vec3 vec = VectorSub(enemies[j].position, explosions[i].position);
			GLfloat distance = (GLfloat)sqrt(pow(vec.x, 2) + pow(vec.z, 2));

			if (explosions[i].radius + enemies[j].width >= distance)
			{
				if (explosions[i].type == 0)
				{
					--enemies[j].life;
				}
				else if (explosions[i].type == 1)
				{
					enemies[j].life -= 3;
				}
				
			}
		}
	}
}

void check_dead_enemies()
{
	for (int i = 0; i < levels[player.level].enemies; ++i)
	{
		if (enemies[i].active && enemies[i].life <= 0)
		{
			enemies[i].active = 0;
			--levels[player.level].enemies_left;
		}
	}
}

void check_player_life(vec3 player_pos)
{
	for (int i = 0; i < levels[player.level].enemies; ++i)
	{
		if (check_proximity(player_pos.x, enemies[i].position.x, enemies[i].width_vec.x) &&
			check_proximity(player_pos.z, enemies[i].position.z, enemies[i].width_vec.z) &&
			enemies[i].active)
		{
			--player.life;
			--levels[player.level].enemies_left;
			enemies[i].active = 0;
			PlaySound(TEXT("Sound/lose_life"), NULL, SND_ASYNC | SND_NODEFAULT | SND_FILENAME);
		}
	}

	if (player.life <= 0)
	{
		game.running = 0;
	}
}

void check_level()
{
	if (levels[player.level].enemies_left <= 0)
	{
		levels[player.level].completed = 1;
		if (player.level == number_of_levels)
		{
			game.running = 0;
		}
		//++player.level;
	}
}

bool delay_level(GLfloat time)
{
	GLfloat delta_time = time - player.delay_time;
	char text[32];
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	sfDrawString(0.42*window_width, 0.45*window_height, "Next level starts in: ");
	snprintf(text, sizeof(text), "%d", (GLint)((player.level_delay - delta_time)/10) + 1);
	sfDrawString(0.58*window_width, 0.45*window_height, text);
	glDisable(GL_BLEND);
	return delta_time >= player.level_delay;
}

void place_landmine(vec3 position, vec3 go_dir, TextureData *tex, GLfloat *vertexArray, vec3 terrain_pos, GLfloat terrain_scale)
{
	for (int i = 0; i < mines; ++i)
	{
		if (!landmines[i].active)
		{
			landmines[i].position = VectorAdd(position, ScalarMult(go_dir, 0.4));
			landmines[i].position.y = findHeight(landmines[i].position.x, landmines[i].position.z, 
				tex, vertexArray, terrain_pos, terrain_scale);
			landmines[i].trans = T(landmines[i].position.x, landmines[i].position.y, landmines[i].position.z);
			landmines[i].active = 1;
			return;
		}
	}
}

void step_on_mine(GLfloat t)
{
	for (int i = 0; i < levels[player.level].enemies; ++i)
	{
		for (int j = 0; j < mines; ++j)
		{
			if (check_proximity(enemies[i].position.x, landmines[j].position.x, enemies[i].width + landmines[j].width) &&
				check_proximity(enemies[i].position.z, landmines[j].position.z, enemies[i].width + landmines[j].width) &&
				enemies[i].active && landmines[j].active)
			{
				trigger_landmine(j, t);
			}
		}
	}
}

void trigger_landmine(GLint i, GLfloat t)
{
	for (int j = 0; j < mines; ++j)
	{
		if (!mine_explosions[j].active && !landmines[i].exploded)
		{
			mine_explosions[j].t_expl = t;
			mine_explosions[j].position = landmines[i].position;
			mine_explosions[j].trans = T(mine_explosions[j].position.x, mine_explosions[j].position.y, mine_explosions[j].position.z);
			mine_explosions[j].active = true;
			landmines[i].exploded = 1;
			waveOutSetVolume(0, 0xFFFF);
			PlaySound(TEXT("Sound/explosion"), NULL, SND_ASYNC | SND_NODEFAULT | SND_FILENAME);
			return;
		}
	}
}

void reload_landmines(GLfloat t)
{
	GLfloat delta_t = t - landmines[0].reload_time;
	if (delta_t >= mine_reload_time)
	{
		for (int i = 0; i < mines; ++i)
		{
			landmines[i].active = 0;
			landmines[i].exploded = 0;
		}
		player.reloading_mines = 0;
	}
}