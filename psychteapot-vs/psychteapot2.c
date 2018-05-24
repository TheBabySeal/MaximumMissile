/*
Written by Jakob Palm, jakpa844
Base project taken from pshycteapot-demo on TSBK07 course website.
I did not manage or find the time to rename the project. 

The code has not been completely cleaned, and there may be declared parameters that are not later used.

I did a ton of experiments with for example the skybox, for which I thought I would need multiple texture units. 
Therefore, I have not specifically structured the use of texture units in order. => you might see the use of texture unit 10,
whilst texture unit 8 and 9 never is used. 

Also, many shaders, especially fragment shaders, include much unnecessary code. Much of it was meant to be used for creating a fog
across the entire map. Functions for lighting has also been copied throughout many shaders, yet not used in all of them.
*/

#include <windows.h>
#include <mmsystem.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "MicroGlut.h"
// uses framework Cocoa
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "extrafunctions.h"
#include "objects.h"
#include "simplefont/simplefont.h"

#define gravity 9.82

// Terrain_scale can be used to scale the entire terrain. x-, y- and z-axises are all scaled with the same parameter.
GLfloat terrain_scale = 10.0;
GLfloat bush_scale = 100.0;
GLfloat missile_speed = 2.0; // velocity of the rockets
GLfloat expl_radius = 40, expl_active_time = 3.0, expl_expand_speed;
GLfloat t; //Global time parameter
GLfloat mouse_speed = 0.0010; //Mouse speed for moving camera
GLfloat old_t = 0.0; // Old time parameter used to calculate jumps
GLfloat jump_speed = 0.0;
GLfloat bonus_height = 0.0; //Bonus height achieved from jumps
GLfloat old_height = 0.0;
GLfloat t_prev_shot = 0.0; //time for the previous missile fired
GLfloat t_reload_start = 0.0; // time for started reload
GLfloat red = 0.0, green = 0.0, blue = 0.0; //Clear color
GLfloat delta_t = 0.0;
GLfloat fps = 60.0;

HWND game_window; // Used for centering mouse
HMONITOR monitor; // Supposed to be used for getting screen resolution, but did not work
MONITORINFO info; // See comment above

int jumping = 0;
int active_missiles = 0;
char text[32];

mat4 projectionMatrix, crosshair_projection, camMatrix, cam_rot;
mat4 worldToViewMatrix, modelToWorldMatrix;
mat4 rot, trans, total, terrain_trans, terrain_total, pot_trans,
	char_rot, char_trans, char_total,
	rpg_rot, rpg_trans, rpg_trans2, rpg_total,
	missile_start_trans,
	crosshair_trans, crosshair_total,
	bush_trans, bush_total,
	skybox_trans, skybox_total,
	heart_trans, heart_total;
vec3 cam = { 0, 20, 0 };
vec3 rpg_vec = { 0, 20, 0 }, rpg_vec2 = { 0, 20, 0 };
vec3 landmine_vec = { 0, 0, 0 };
vec3 lookAtPoint = { 0, 20, 1 };
vec3 pot_position = { 100, 45, 100 };
vec3 char_position = { 50, 0, 50 };
vec3 up_vec = { 0.0, 1.0, 0.0 };
vec3 go_dir = { 0, 0, 1 };
vec3 cross_rpg = { 0, 0, 0 };
vec3 terrain_vec = { -64, 0, -64 };
vec3 lookAtVec = { 0, 0, 1 };
vec3 missile_start = { 0,0,0 };
vec3 screen_object_color = { 1.0, 1.0, 1.0 };
//Models
Model *m, *tm, *m_char, *m_rpg, *m_missile, *m_landmine, *m_tank, *m_box, *m_bush, *m_grass, *m_sphere, *m_skybox,
	*m_front, *m_back, *m_right, *m_left, *m_up, *m_down, *m_heart, *m_billboard;

//Textures
GLuint tex, char_tex, grass_tex, grass_model_tex, rpg_tex, military_tex, pavement_tex, gravel_tex, 
		stones_tex, blendmap_tex, crosshair_tex, bush_tex, skybox_tex, cube_tex,
		explosion_tex, mine_explosion_tex,
		skybox_back, skybox_front, skybox_right, skybox_left, skybox_up, skybox_down,
		heart_tex, levels_tex, enemies_left_tex, numbers[10];

GLuint font;

// Terrain
TextureData ttex, cubemap_tex, font_tex_data;

// Reference to shader program
GLuint program; // Used for models
GLuint terrainprogram; // Used for terrain
GLuint crosshairprogram; // Used for the crosshair
GLuint fernprogram; // Used for ferns
GLuint skyboxprogram; // Used for skybox
GLuint enemyprogram; // Used for enemies
GLuint guiprogram;
GLuint explosionprogram;

// key press for start screen. Space to start game basically. 
void off_screen_key_press()
{
	if (glutKeyIsDown(GLUT_KEY_SPACE) && !game.running)
	{
		init_enemies();
		init_player();
		init_levels();
		game.running = 1;
	}
}

// Computes all keyboard interactions once game is running
void key_press(GLfloat t)
{
	GLfloat movement_time = 0.2;
	GLfloat rotation_time = 0.2;
	GLfloat strafe_time = 0.1;
	GLfloat old_cam_y = cam.y;
	
	// An attempt to make the game time based instead of frame based.
	GLfloat rotation_speed = rotation_time / delta_t;
	GLfloat movement_speed = movement_time / delta_t;
	GLfloat strafe_speed = strafe_time / delta_t;

	vec3 right_vec = CrossProduct(go_dir, up_vec);

	// Switch weapons with 1 and 2
	if (glutKeyIsDown('1'))
	{
		player.active_weapon = 1;
	}
	if (glutKeyIsDown('2'))
	{
		player.active_weapon = 2;
	}

	// Use r to reload landmines, removes currently active landmines!
	if (glutKeyIsDown('r'))
	{
		GLint active_landmines = 0;
		for (int i = 0; i < mines; ++i)
		{
			if (landmines[i].active)
			{
				++active_landmines;
			}
			landmines[i].reload_time = t;
		}
		if (active_landmines > 0)
		{
			player.reloading_mines = 1;
		}
	}

	// Use w to go forward
	if (glutKeyIsDown('w'))
	{
		//First check if player is outside map limits. 
		//Then move camera and the RPG-model in x- and z directions. 
		if (!outside_map(cam.x + go_dir.x*movement_speed, cam.z + go_dir.z*movement_speed, terrain_scale, terrain_vec))
		{
			cam.x = cam.x + go_dir.x*movement_speed;
			cam.z = cam.z + go_dir.z*movement_speed;

			lookAtPoint.x = lookAtPoint.x + go_dir.x*movement_speed;
			lookAtPoint.z = lookAtPoint.z + go_dir.z*movement_speed;

			rpg_vec.x = rpg_vec.x + go_dir.x*movement_speed;
			rpg_vec.z = rpg_vec.z + go_dir.z*movement_speed;
		}
	}
	// Use s to move backwards
	else if (glutKeyIsDown('s'))
	{
		if (!outside_map(cam.x - go_dir.x*movement_speed, cam.z - go_dir.z*movement_speed, terrain_scale, terrain_vec))
		{
			cam.x = cam.x - go_dir.x*movement_speed;
			cam.z = cam.z - go_dir.z*movement_speed;

			lookAtPoint.x = lookAtPoint.x - go_dir.x*movement_speed;
			lookAtPoint.z = lookAtPoint.z - go_dir.z*movement_speed;

			rpg_vec.x = rpg_vec.x - go_dir.x*movement_speed;
			rpg_vec.z = rpg_vec.z - go_dir.z*movement_speed;
		}
	}
	// Use a to rotate left
	if (glutKeyIsDown('a'))
	{
		rpg_rot = Mult(ArbRotate(up_vec, 0.05*rotation_speed), rpg_rot);

		cam_rot = ArbRotate(up_vec, 0.05*rotation_speed);

		lookAtVec = MultVec3(cam_rot, lookAtVec);

		lookAtPoint = VectorAdd(lookAtVec, cam);
	}
	// Use d to rotate right
	else if (glutKeyIsDown('d'))
	{
		rpg_rot = Mult(ArbRotate(up_vec, -0.05*rotation_speed), rpg_rot);

		cam_rot = ArbRotate(up_vec, -0.05*rotation_speed);

		lookAtVec = MultVec3(cam_rot, lookAtVec);

		lookAtPoint = VectorAdd(lookAtVec, cam);
	}

	//Strafe to the right
	if (glutKeyIsDown('e'))
	{
		//Move camera and RPG in x- and z directions
		if (!outside_map(cam.x + right_vec.x*strafe_speed, cam.z + right_vec.z*strafe_speed, terrain_scale, terrain_vec))
		{
			cam.x = cam.x + right_vec.x*strafe_speed;
			cam.z = cam.z + right_vec.z*strafe_speed;

			lookAtPoint.x = lookAtPoint.x + right_vec.x*strafe_speed;
			lookAtPoint.z = lookAtPoint.z + right_vec.z*strafe_speed;

			rpg_vec.x = rpg_vec.x + right_vec.x*strafe_speed;
			rpg_vec.z = rpg_vec.z + right_vec.z*strafe_speed;
		}
	}
	//Strafe to the left
	else if (glutKeyIsDown('q'))
	{
		//Move camera and RPG in x- and z directions
		if (!outside_map(cam.x - right_vec.x*strafe_speed, cam.z - right_vec.z*strafe_speed, terrain_scale, terrain_vec))
		{
			cam.x = cam.x - right_vec.x*strafe_speed;
			cam.z = cam.z - right_vec.z*strafe_speed;

			lookAtPoint.x = lookAtPoint.x - right_vec.x*strafe_speed;
			lookAtPoint.z = lookAtPoint.z - right_vec.z*strafe_speed;

			rpg_vec.x = rpg_vec.x - right_vec.x*strafe_speed;
			rpg_vec.z = rpg_vec.z - right_vec.z*strafe_speed;
		}
	}

	// Space to jump
	if (glutKeyIsDown(GLUT_KEY_SPACE) && !jumping)
	{
		jumping = 1;
		jump_speed = 15.0;
	}

	// recalculate the jump_speed and y-coordinate of the player.
	if (jumping)
	{
		GLfloat delta_height = 0;
		jump_speed -= gravity / 2 * delta_t; // Gravity / 2 because 9.82 felt too much in practical
		GLfloat temp_y = findHeight(cam.x, cam.z, &ttex, tm->vertexArray, terrain_vec, terrain_scale) + player.height;
		delta_height = temp_y - old_cam_y; // Adjust jump height according to difference between previous and current position of player.

		//Jumps does NOT take in mind delta_height as mentioned above, since I did not have to time to get this working properly.
		bonus_height += (jump_speed * delta_t);// Sets the jump height based on current jump speed
		if (bonus_height <= 0)
		{
			bonus_height = 0;
			jump_speed = 0;
			jumping = 0;
		}
	}

	//Move camera and RPG to height based on current x- and z coordinates
	cam.y = findHeight(cam.x, cam.z, &ttex, tm->vertexArray, terrain_vec, terrain_scale) + player.height + bonus_height;
	rpg_vec.y = findHeight(rpg_vec.x, rpg_vec.z, &ttex, tm->vertexArray, terrain_vec, terrain_scale) + player.height + bonus_height;
	lookAtPoint.y += cam.y - old_cam_y;
}


void mouse_movement()
{
	//Do not move camera if game window is not active
	if (GetActiveWindow() != game_window)
	{
		return;
	}
	
	vec3 horizontal_axis = CrossProduct(go_dir, up_vec); // Also named right_vec in other functions
	POINT mouse_pos;
	GetCursorPos(&mouse_pos);
	GLfloat width_center = game.width / 2;
	GLfloat height_center = game.height / 2;

	GLfloat delta_x = mouse_pos.x - width_center;
	GLfloat delta_y = mouse_pos.y - height_center;

	//No point in running rest of the function if mouse hasn't moved from center
	if (delta_x == 0 && delta_y == 0)
	{
		return;
	}

	//First rotate to the side
	cam_rot = ArbRotate(up_vec, - delta_x * mouse_speed);
	rpg_rot = Mult(ArbRotate(up_vec, - delta_x * mouse_speed), rpg_rot);

	lookAtVec = MultVec3(cam_rot, lookAtVec);
	
	//Then rotate up or down.
	cam_rot = ArbRotate(horizontal_axis, - delta_y * mouse_speed);
	rpg_rot = Mult(ArbRotate(horizontal_axis, - delta_y * mouse_speed), rpg_rot);

	lookAtVec = MultVec3(cam_rot, lookAtVec);
	lookAtPoint = VectorAdd(lookAtVec, cam);
	//Finally reset mouse position to center
	SetCursorPos(width_center, height_center);
}

void init(void)
{
	// GL inits
	glClearColor(red, green, blue, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);

	srand(time(NULL));

	//If you want fullscreen, type in the values of your screens resolution in game.width & game.height and remove comments.
	//glutFullScreen();
	//game.width = 1366.0;
	//game.height = 768.0;
	glutHideCursor();
	SetCursorPos(game.width / 2, game.height / 2);

	// Load and compile shaders
	program = loadShaders("model.vert", "model.frag");
	terrainprogram = loadShaders("terrain.vert", "terrain.frag");
	crosshairprogram = loadShaders("crosshair.vert", "crosshair.frag");
	fernprogram = loadShaders("fern.vert", "fern.frag");
	skyboxprogram = loadShaders("skybox.vert", "skybox.frag");
	enemyprogram = loadShaders("enemies.vert", "enemies.frag");
	guiprogram = loadShaders("gui.vert", "gui.frag");
	explosionprogram = loadShaders("explosion.vert", "explosion.frag");

	glUseProgram(skyboxprogram);
	LoadTGATextureSimple("skybox/mnight.tga", &skybox_tex);

	//did a few attempts getting this to work, finally decided to not use it
	m_skybox = LoadModelPlus("skybox/testbox2.obj");
	
	//6 different planes instead of one cube model
	m_front = LoadModelPlus("skybox/skybox_front.obj");
	m_back = LoadModelPlus("skybox/skybox_back.obj");
	m_up = LoadModelPlus("skybox/skybox_up.obj");
	m_down = LoadModelPlus("skybox/skybox_down.obj");
	m_left = LoadModelPlus("skybox/skybox_left.obj");
	m_right = LoadModelPlus("skybox/skybox_right.obj");

	glActiveTexture(GL_TEXTURE7);
	LoadTGATextureSimple("skybox/japan/back.tga", &skybox_back);
	LoadTGATextureSimple("skybox/japan/front.tga", &skybox_front);
	LoadTGATextureSimple("skybox/japan/right.tga", &skybox_right);
	LoadTGATextureSimple("skybox/japan/left.tga", &skybox_left);
	LoadTGATextureSimple("skybox/japan/up.tga", &skybox_up);
	LoadTGATextureSimple("skybox/japan/down.tga", &skybox_down);
	
	glUseProgram(fernprogram);
	LoadTGATextureSimple("Terrain/fern.tga", &bush_tex);
	m_bush = LoadModelPlus("Terrain/fern.obj");
	m_grass = LoadModelPlus("Terrain/grassModel.obj");
	LoadTGATextureSimple("Terrain/grassTexture.tga", &grass_model_tex);

	glUseProgram(crosshairprogram);
	LoadTGATextureSimple("crosshair.tga", &crosshair_tex);
	LoadTGATextureSimple("heart.tga", &heart_tex);
	LoadTGATextureSimple("TextTextures/Level.tga", &levels_tex);
	LoadTGATextureSimple("TextTextures/Enemies.tga", &enemies_left_tex);

	//An experiment with self-made numbers that could be written on screen. Ended up using simplefont from the demo-page.
	LoadTGATextureSimple("TextTextures/0.tga", &numbers[0]);
	LoadTGATextureSimple("TextTextures/1.tga", &numbers[1]);
	LoadTGATextureSimple("TextTextures/2.tga", &numbers[2]);
	LoadTGATextureSimple("TextTextures/3.tga", &numbers[3]);
	LoadTGATextureSimple("TextTextures/4.tga", &numbers[4]);
	LoadTGATextureSimple("TextTextures/5.tga", &numbers[5]);
	LoadTGATextureSimple("TextTextures/6.tga", &numbers[6]);
	LoadTGATextureSimple("TextTextures/7.tga", &numbers[7]);
	LoadTGATextureSimple("TextTextures/8.tga", &numbers[8]);
	LoadTGATextureSimple("TextTextures/9.tga", &numbers[9]);

	glUseProgram(terrainprogram);
	// Load terrain data
	LoadTGATextureData("fft-terrain.tga", &ttex);
	LoadTGATextureSimple("Terrain/blendMap.tga", &blendmap_tex);
	LoadTGATextureSimple("Terrain/Gravel.tga", &gravel_tex);
	LoadTGATextureSimple("Terrain/Pavement.tga", &pavement_tex);
	LoadTGATextureSimple("Terrain/Stones.tga", &stones_tex);
	LoadTGATextureSimple("Terrain/GrassGreenTexture0001.tga", &grass_tex);

	// Upload terrain to GPU
	tm = GenerateTerrain(&ttex);

	glUseProgram(program);

	LoadTGATextureSimple("maskros512.tga", &tex);
	LoadTGATextureSimple("Muro_body_dm.tga", &char_tex);
	LoadTGATextureSimple("weapons/rocketlauncher.tga", &rpg_tex);
	LoadTGATextureSimple("weapons/explosion.tga", &explosion_tex);
	LoadTGATextureSimple("weapons/mine_explosion.tga", &mine_explosion_tex);
	LoadTGATextureSimple("weapons/camouflage.tga", &military_tex);

	// Upload geometry to the GPU:
	m = LoadModelPlus("teapot.obj");
	m_char = LoadModelPlus("muro.obj");
	m_rpg = LoadModelPlus("weapons/rocketlauncher.obj");
	m_missile = LoadModelPlus("weapons/rocketlauncher_shell.obj");
	m_box = LoadModelPlus("skybox/skybox_front.obj");
	m_sphere = LoadModelPlus("sphere.obj");
	m_landmine = LoadModelPlus("weapons/landmine.obj");
	m_tank = LoadModelPlus("tank/tank.obj");
	m_billboard = LoadModelPlus("weapons/billboard.obj");
	// End of upload of geometry

	// Camera init
	lookAtVec = VectorSub(lookAtPoint, cam);

	cam.y = findHeight(cam.x, cam.z, &ttex, tm->vertexArray, terrain_vec, terrain_scale) + player.height;
	lookAtPoint.y = cam.y;
	camMatrix = lookAt(cam.x, cam.y, cam.z,
		lookAtPoint.x, lookAtPoint.y, lookAtPoint.z,
		0.0, 1.0, 0.0);

	pot_position.y = findHeight(pot_position.x, pot_position.z, &ttex, tm->vertexArray, terrain_vec, terrain_scale);
	trans = T(pot_position.x, pot_position.y, pot_position.z);

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 300*terrain_scale);
	worldToViewMatrix = camMatrix;
	modelToWorldMatrix = IdentityMatrix();
	
	//Initialize models and terrain matrices
	total = Mult(modelToWorldMatrix, Mult(trans, Rx(-M_PI / 2))); // trans centers teapot, Rx rotates the teapot to a comfortable default
	terrain_trans = T(terrain_vec.x, terrain_vec.y, terrain_vec.z);

	// Crosshair initialization
	crosshair_trans = T(0, 0, -1.0);

	// RPG
	rpg_vec.y = findHeight(rpg_vec.x, rpg_vec.z, &ttex, tm->vertexArray, terrain_vec, terrain_scale) + player.height;
	rpg_trans = T(rpg_vec.x, rpg_vec.y, rpg_vec.z);
	rpg_rot = Ry(M_PI);

	// Move RPG slightly to the right-side of screen
	cross_rpg = CrossProduct(up_vec, go_dir);
	rpg_vec2 = VectorSub(ScalarMult(cross_rpg, 0.15), ScalarMult(go_dir, 0.50));
	// Using different scaling for the landmine weapon than the RPG
	landmine_vec = VectorSub(ScalarMult(cross_rpg, 0.15), ScalarMult(go_dir, 0.80));

	// Starting position of the missiles, based on the current position of the player
	missile_start = VectorAdd(ScalarMult(cross_rpg, -0.50), ScalarMult(go_dir, 0.50));
	missile_start = VectorAdd(missile_start, ScalarMult(up_vec, -0.55));
	missile_start_trans = T(missile_start.x, missile_start.y, missile_start.z);

	// Upload projectionMatrix to all relevant shaders, as well as the clear color, 
	// in case I had succeded with implementing fog
	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform3f(glGetUniformLocation(program, "skyColor"), red, green, blue);

	glUseProgram(enemyprogram);
	glUniformMatrix4fv(glGetUniformLocation(enemyprogram, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform3f(glGetUniformLocation(enemyprogram, "skyColor"), red, green, blue);

	glUseProgram(skyboxprogram);
	glUniformMatrix4fv(glGetUniformLocation(skyboxprogram, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform3f(glGetUniformLocation(skyboxprogram, "skyColor"), red, green, blue);
	skybox_trans = T(cam.x, cam.y, cam.z);

	glUseProgram(fernprogram);
	glUniformMatrix4fv(glGetUniformLocation(fernprogram, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform3f(glGetUniformLocation(fernprogram, "skyColor"), red, green, blue);
	
	glUseProgram(explosionprogram);
	glUniformMatrix4fv(glGetUniformLocation(explosionprogram, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform3f(glGetUniformLocation(explosionprogram, "skyColor"), red, green, blue);

	glUseProgram(terrainprogram);
	glUniform1fv(glGetUniformLocation(terrainprogram, "terrain_scale"), 1, &terrain_scale);
	glUniformMatrix4fv(glGetUniformLocation(terrainprogram, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform3f(glGetUniformLocation(terrainprogram, "skyColor"), red, green, blue);

	// Bind terrain-textures to suited texture unit. 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, blendmap_tex);
	glUniform1i(glGetUniformLocation(terrainprogram, "blendMap"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, grass_tex);
	glUniform1i(glGetUniformLocation(terrainprogram, "background"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pavement_tex);
	glUniform1i(glGetUniformLocation(terrainprogram, "rtex"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gravel_tex);
	glUniform1i(glGetUniformLocation(terrainprogram, "gtex"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, stones_tex);
	glUniform1i(glGetUniformLocation(terrainprogram, "btex"), 2);

	glUseProgram(program);

	// Randomize spawn positions of the plants
	for (int k = 1; k + 2 < map_size * terrain_scale / bush_scale; ++k)
	{
		for (int j = 1; j + 2 < map_size * terrain_scale / bush_scale; ++j)
		{
			ferns[k][j].x = (GLfloat)RandomNumber(bush_scale * k, bush_scale * (k + 1));
			ferns[k][j].z = (GLfloat)RandomNumber(bush_scale * j, bush_scale * (j + 1));
			ferns[k][j].y = findHeight(ferns[k][j].x, ferns[k][j].z, &ttex, tm->vertexArray, terrain_vec, terrain_scale);
		}
	}

	// Set the explosion expansion speed according to set radius and active time.
	expl_expand_speed = expl_radius / expl_active_time;
	explosion_init(expl_radius, expl_active_time, expl_expand_speed);

	//Spawn level 0. Basically zero enemies.
	for (int i = 0; i < levels[player.level].enemies; ++i)
	{
		spawn_enemy(&ttex, tm->vertexArray, terrain_vec, terrain_scale, cam);
	}
}

void display(void)
{
	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	t = glutGet(GLUT_ELAPSED_TIME) / 100.0;

	delta_t = t - old_t;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Start screen
	if (!game.running)
	{
		sfDrawString(0.05*game.width, 0.28*game.height, "w - move forward");
		sfDrawString(0.05*game.width, 0.31*game.height, "s - move backwards");
		sfDrawString(0.05*game.width, 0.34*game.height, "a - rotate left");
		sfDrawString(0.05*game.width, 0.37*game.height, "d - rotate right");
		sfDrawString(0.05*game.width, 0.40*game.height, "q - strafe left");
		sfDrawString(0.05*game.width, 0.43*game.height, "e - strafe right");
		sfDrawString(0.05*game.width, 0.50*game.height, "LMB - fire missile/place landmine");
		sfDrawString(0.05*game.width, 0.57*game.height, "r - reload landmines");
		sfDrawString(0.05*game.width, 0.60*game.height, "1 - switch to RPG");
		sfDrawString(0.05*game.width, 0.63*game.height, "2 - switch to landmines");
		sfDrawString(0.60*game.width, 0.05*game.height, "Tip: Use landmines versus tanks");
		sfDrawString(0.40*game.width, 0.50*game.height, "Space to start game");

		off_screen_key_press();

		// Reload mines, so that you always start with maximum amount of landmines
		reload_landmines(t);

		// Reload missiles
		if (reload_missile(&ttex, tm->vertexArray, terrain_vec, terrain_scale, t_reload_start, t))
		{
			t_reload_start = t;
		}
		// Swap buffers to clear the screen and get the black background color.
		glutSwapBuffers();
	}
	
	// An attempt to get steady FPS, did not work properly on my laptop, maybe on a better PC.
	if (delta_t < (1.0 / (10.0*fps)) || !game.running)
	{
		return;
	}

	glDisable(GL_BLEND);

	glUseProgram(skyboxprogram);
	
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE7);
	
	skybox_trans = T(cam.x, cam.y, cam.z);
	skybox_total = skybox_trans;
	glUniformMatrix4fv(glGetUniformLocation(skyboxprogram, "world_to_view"), 1, GL_TRUE, camMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(skyboxprogram, "model_to_world"), 1, GL_TRUE, skybox_total.m);
	
	// Draw skybox, one side at a time.

	glBindTexture(GL_TEXTURE_2D, skybox_front);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i(glGetUniformLocation(skyboxprogram, "tex"), 7);
	DrawModel(m_front, skyboxprogram, "inPosition", "inNormal", "inTexCoord");

	glBindTexture(GL_TEXTURE_2D, skybox_back);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i(glGetUniformLocation(skyboxprogram, "tex"), 7);
	DrawModel(m_back, skyboxprogram, "inPosition", "inNormal", "inTexCoord");

	glBindTexture(GL_TEXTURE_2D, skybox_up);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i(glGetUniformLocation(skyboxprogram, "tex"), 7);
	DrawModel(m_up, skyboxprogram, "inPosition", "inNormal", "inTexCoord");
	
	glBindTexture(GL_TEXTURE_2D, skybox_down);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i(glGetUniformLocation(skyboxprogram, "tex"), 7);
	DrawModel(m_down, skyboxprogram, "inPosition", "inNormal", "inTexCoord");

	glBindTexture(GL_TEXTURE_2D, skybox_left);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i(glGetUniformLocation(skyboxprogram, "tex"), 7);
	DrawModel(m_left, skyboxprogram, "inPosition", "inNormal", "inTexCoord");

	glBindTexture(GL_TEXTURE_2D, skybox_right);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i(glGetUniformLocation(skyboxprogram, "tex"), 7);
	DrawModel(m_right, skyboxprogram, "inPosition", "inNormal", "inTexCoord");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);
	
	glUseProgram(enemyprogram);

	glUniformMatrix4fv(glGetUniformLocation(enemyprogram, "world_to_view"), 1, GL_TRUE, camMatrix.m);
	glUniform3f(glGetUniformLocation(enemyprogram, "cam_pos"), cam.x, cam.y, cam.z);

	// Draw enemies
	for (int i = 0; i < levels[player.level].enemies; ++i)
	{
		if (enemies[i].active)
		{
			GLfloat z_dist = cam.z - enemies[i].position.z;
			GLfloat x_dist = cam.x - enemies[i].position.x;
			enemies[i].angle = (GLfloat)atan2(x_dist, z_dist); // Rotate enemies towards player
			enemies[i].total = Mult(enemies[i].trans, Mult(Ry(enemies[i].angle), S(enemy0_scale, enemy0_scale, enemy0_scale)));
			glUniformMatrix4fv(glGetUniformLocation(enemyprogram, "model_to_world"), 1, GL_TRUE, enemies[i].total.m);

			// Draw walking enemies
			if (enemies[i].type == 0)
			{
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, char_tex);
				glUniform1i(glGetUniformLocation(enemyprogram, "tex"), 6);
				DrawModel(m_char, enemyprogram, "inPosition", "inNormal", "inTexCoord");
			}
			// Draw tanks
			else if (enemies[i].type == 1)
			{
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, military_tex);
				glUniform1i(glGetUniformLocation(enemyprogram, "tex"), 6);
				DrawModel(m_tank, enemyprogram, "inPosition", "inNormal", "inTexCoord");
			}
		}
	}

	// Reupload player position to every relevant program.
	glUseProgram(explosionprogram);
	glUniformMatrix4fv(glGetUniformLocation(explosionprogram, "world_to_view"), 1, GL_TRUE, camMatrix.m);
	glUniform3f(glGetUniformLocation(explosionprogram, "cam_pos"), cam.x, cam.y, cam.z);

	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "world_to_view"), 1, GL_TRUE, camMatrix.m);
	glUniform3f(glGetUniformLocation(program, "cam_pos"), cam.x, cam.y, cam.z);

	glActiveTexture(GL_TEXTURE5);

	//First translate to right-side of screen, then rotate then translate to current camera position
	rpg_trans = T(rpg_vec.x, rpg_vec.y - 0.3, rpg_vec.z);
	rpg_trans2 = T(rpg_vec2.x, rpg_vec2.y, rpg_vec2.z);
	rpg_total = Mult(Mult(rpg_trans, Mult(rpg_rot, rpg_trans2)), S(1.0f, 1.0f, 1.0f));
	
	// Draw RPG
	if (player.active_weapon == 1)
	{
		glBindTexture(GL_TEXTURE_2D, char_tex);
		glUniform1i(glGetUniformLocation(program, "tex"), 5);
		glUniformMatrix4fv(glGetUniformLocation(program, "model_to_world"), 1, GL_TRUE, rpg_total.m);
		DrawModel(m_rpg, program, "inPosition", "inNormal", "inTexCoord");
	}
	// Draw Landmine-weapon
	else if (player.active_weapon == 2)
	{
		glBindTexture(GL_TEXTURE_2D, military_tex);
		glUniform1i(glGetUniformLocation(program, "tex"), 5);
		rpg_trans2 = T(landmine_vec.x, landmine_vec.y, landmine_vec.z);
		rpg_total = Mult(Mult(rpg_trans, Mult(rpg_rot, rpg_trans2)), S(0.08f, 0.08f, 0.08f));
		glUniformMatrix4fv(glGetUniformLocation(program, "model_to_world"), 1, GL_TRUE, rpg_total.m);
		DrawModel(m_landmine, program, "inPosition", "inNormal", "inTexCoord");
	}
	active_missiles = 0;
	for (int i = 0; i < ammo; ++i)
	{
		if (missiles[i].active)
		{
			missiles[i].position = VectorAdd(missiles[i].position, ScalarMult(missiles[i].direction, missile_speed/delta_t));
			missiles[i].trans = T(missiles[i].position.x, missiles[i].position.y, missiles[i].position.z);
			missiles[i].total = Mult(Mult(missiles[i].trans, Mult(missiles[i].rot, missile_start_trans)), S(20.0f, 20.0f, 20.0f));
			glUseProgram(program);
			glUniformMatrix4fv(glGetUniformLocation(program, "model_to_world"), 1, GL_TRUE, missiles[i].total.m);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, char_tex);
			glUniform1i(glGetUniformLocation(program, "tex"), 5);

			// Check enemy collision for every missile
			if (enemy_collision(missiles[i].position))
			{
				explode(i, t);
			}
			// If the missile didn't explode yet, draw it.
			if (!missiles[i].exploded)
			{
				DrawModel(m_missile, program, "inPosition", "inNormal", "inTexCoord");
			}
			// Calculate the amount of active missiles.
			++active_missiles;
		}

		if (explosions[i].active)
		{
			glUseProgram(explosionprogram);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, explosion_tex);
			glUniform1i(glGetUniformLocation(explosionprogram, "tex"), 5);
			glUniform1i(glGetUniformLocation(explosionprogram, "type"), explosions[i].type);
			GLfloat delta_expl_t = t - explosions[i].t_expl;
			GLfloat z_dist = explosions[i].position.z - cam.z;
			GLfloat x_dist = explosions[i].position.x - cam.x;
			explosions[i].angle = (GLfloat)atan2(x_dist, z_dist); // Rotate the explosion-billboard toward the player
			explosions[i].total = Mult(explosions[i].trans, Mult(Ry(explosions[i].angle), 
				S(explosions[i].expand_speed * delta_expl_t,
				explosions[i].expand_speed * delta_expl_t, enemy_width))); //Scale z-coordiante with enemy_width to make sure the explosion is visible
			glUniformMatrix4fv(glGetUniformLocation(explosionprogram, "model_to_world"), 1, GL_TRUE, explosions[i].total.m);
			DrawModel(m_back, explosionprogram, "inPosition", "inNormal", "inTexCoord");
		}

		recharge_explosions(i, t, &explosions);
	}

	if (active_missiles == 0)
	{
		t_reload_start = t;
	}

	// Do the same as above, but for landmines
	GLint active_landmines = 0;
	for (int i = 0; i < mines; ++i)
	{
		glUseProgram(program);
		if (landmines[i].active)
		{
			++active_landmines;
			landmines[i].total = Mult(landmines[i].trans, S(1.0, 1.0, 1.0));
			glUniformMatrix4fv(glGetUniformLocation(program, "model_to_world"), 1, GL_TRUE, landmines[i].total.m);
			
			if (!landmines[i].exploded)
			{
				glBindTexture(GL_TEXTURE_2D, military_tex);
				glUniform1i(glGetUniformLocation(program, "tex"), 5);
				DrawModel(m_landmine, program, "inPosition", "inNormal", "inTexCoord");
			}
		}

		glUseProgram(explosionprogram);
		if (mine_explosions[i].active)
		{
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, mine_explosion_tex);
			glUniform1i(glGetUniformLocation(explosionprogram, "tex"), 5);
			glUniform1i(glGetUniformLocation(explosionprogram, "type"), mine_explosions[i].type);
			GLfloat delta_expl_t = t - mine_explosions[i].t_expl;
			GLfloat z_dist = mine_explosions[i].position.z - cam.z;
			GLfloat x_dist = mine_explosions[i].position.x - cam.x;
			mine_explosions[i].angle = (GLfloat)atan2(x_dist, z_dist); //
			mine_explosions[i].total = Mult(mine_explosions[i].trans, Mult(Ry(mine_explosions[i].angle), S(mine_explosions[i].expand_speed * delta_expl_t,
				mine_explosions[i].expand_speed * delta_expl_t, mine_explosions[i].expand_speed * delta_expl_t)));
			glUniformMatrix4fv(glGetUniformLocation(explosionprogram, "model_to_world"), 1, GL_TRUE, mine_explosions[i].total.m);
			DrawModel(m_billboard, explosionprogram, "inPosition", "inNormal", "inTexCoord");
		}
		recharge_explosions(i, t, &mine_explosions);

	}

	glUseProgram(fernprogram);
	glUniformMatrix4fv(glGetUniformLocation(fernprogram, "world_to_view"), 1, GL_TRUE, camMatrix.m);
	glUniform3f(glGetUniformLocation(fernprogram, "cam_pos"), cam.x, cam.y, cam.z);
	glDisable(GL_CULL_FACE);
	glActiveTexture(GL_TEXTURE6);

	// Draw plants across the entire terrain with a frequency factor bush_scale
	for (int k = 1; k + 2 < map_size * terrain_scale / bush_scale; ++k)
	{
		for (int j = 1; j + 2 < map_size * terrain_scale / bush_scale; ++j)
		{
			bush_trans = T(ferns[k][j].x, ferns[k][j].y, ferns[k][j].z);
			bush_total = Mult(bush_trans, S(2.0, 2.0, 2.0));
			glUniformMatrix4fv(glGetUniformLocation(fernprogram, "model_to_world"), 1, GL_TRUE, bush_total.m);
			glBindTexture(GL_TEXTURE_2D, bush_tex);
			glUniform1i(glGetUniformLocation(fernprogram, "tex"), 6);
			DrawModel(m_bush, fernprogram, "inPosition", "inNormal", "inTexCoord");

			bush_total = Mult(bush_trans, S(8.0, 8.0, 8.0));
			glUniformMatrix4fv(glGetUniformLocation(fernprogram, "model_to_world"), 1, GL_TRUE, bush_total.m);
			glBindTexture(GL_TEXTURE_2D, grass_model_tex);
			glUniform1i(glGetUniformLocation(fernprogram, "tex"), 6);
			DrawModel(m_grass, fernprogram, "inPosition", "inNormal", "inTexCoord");
		}
	}

	glUseProgram(crosshairprogram);
	// The current crosshair is 4 cube-models rotated to look like a crosshair. Later I realized
	// that a simple billboard with texture would've been much easier, but I did not bother changing this.

	screen_object_color = SetVector(1.0, 1.0, 1.0);
	glUniform3f(glGetUniformLocation(crosshairprogram, "inColor"), screen_object_color.x, screen_object_color.y, screen_object_color.z);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, crosshair_tex);
	glUniform1i(glGetUniformLocation(crosshairprogram, "tex"), 5);

	//Down right
	crosshair_total = Mult(T(0.03, -0.03, 0), Mult(Rz(-M_PI/4), S(0.01, 0.001, 0.01))); 
	glUniformMatrix4fv(glGetUniformLocation(crosshairprogram, "model_to_world"), 1, GL_TRUE, crosshair_total.m);
	DrawModel(m_box, crosshairprogram, "inPosition", "inNormal", "inTexCoord");

	//Down left
	crosshair_total = Mult(T(-0.03, -0.03, 0), Mult(Rz(M_PI/4), S(0.01, 0.001, 0.01)));
	glUniformMatrix4fv(glGetUniformLocation(crosshairprogram, "model_to_world"), 1, GL_TRUE, crosshair_total.m);
	DrawModel(m_box, crosshairprogram, "inPosition", "inNormal", "inTexCoord");

	//Up left
	crosshair_total = Mult(T(-0.03, 0.03, 0), Mult(Rz(-M_PI/4), S(0.01, 0.001, 0.01)));
	glUniformMatrix4fv(glGetUniformLocation(crosshairprogram, "model_to_world"), 1, GL_TRUE, crosshair_total.m);
	DrawModel(m_box, crosshairprogram, "inPosition", "inNormal", "inTexCoord");

	//Up right
	crosshair_total = Mult(T(0.03, 0.03, 0), Mult(Rz(M_PI/4), S(0.01, 0.001, 0.01)));
	glUniformMatrix4fv(glGetUniformLocation(crosshairprogram, "model_to_world"), 1, GL_TRUE, crosshair_total.m);
	DrawModel(m_box, crosshairprogram, "inPosition", "inNormal", "inTexCoord");

	//Draw ammo, bottom left of screen. This is for the GUI
	screen_object_color = SetVector( 0.0, 0.0, 0.0 );
	glUniform3f(glGetUniformLocation(crosshairprogram, "inColor"), screen_object_color.x, screen_object_color.y, screen_object_color.z);

	for (int i = 0; i < ammo - active_missiles; ++i)
	{
		crosshair_total = Mult(T(-0.9 + 0.05*i, -0.9, 0), Mult(Ry(M_PI / 2), S(0.2, 0.5, 0.2)));
		glUniformMatrix4fv(glGetUniformLocation(crosshairprogram, "model_to_world"), 1, GL_TRUE, crosshair_total.m);
		DrawModel(m_missile, crosshairprogram, "inPosition", "inNormal", "inTexCoord");
	}

	for (int i = 0; i < mines - active_landmines; ++i)
	{
		crosshair_total = Mult(T(-0.9 + 0.05*i, -0.84, 0), Mult(Rx(0), S(0.01, 0.03, 0.01)));
		glUniformMatrix4fv(glGetUniformLocation(crosshairprogram, "model_to_world"), 1, GL_TRUE, crosshair_total.m);
		DrawModel(m_landmine, crosshairprogram, "inPosition", "inNormal", "inTexCoord");
	}
	
	glUseProgram(guiprogram);
	// Draw any GUI that did not require specific models, but instead billboards.
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, heart_tex);
	glUniform1i(glGetUniformLocation(guiprogram, "tex"), 5);

	for (int i = 0; i < player.life; ++i)
	{
		heart_total = Mult(T(-0.9 + 0.04*i, 0.9, 0), S(0.02, 0.06, 0.0));
		glUniformMatrix4fv(glGetUniformLocation(guiprogram, "model_to_world"), 1, GL_TRUE, heart_total.m);
		DrawModel(m_box, guiprogram, "inPosition", "inNormal", "inTexCoord");
	}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);
	
	// Upload terrain to GPU, first switch shaders
	glUseProgram(terrainprogram);
	terrain_total = Mult(terrain_trans, S(terrain_scale, terrain_scale, terrain_scale));
	glUniformMatrix4fv(glGetUniformLocation(terrainprogram, "model_to_world"), 1, GL_TRUE, terrain_total.m);
	glUniformMatrix4fv(glGetUniformLocation(terrainprogram, "world_to_view"), 1, GL_TRUE, camMatrix.m);
	glUniform3f(glGetUniformLocation(terrainprogram, "cam_pos"), cam.x, cam.y, cam.z);

	//glBindTexture(GL_TEXTURE_2D, grass_tex);
	DrawModel(tm, terrainprogram, "inPosition", "inNormal", "inTexCoord");
	
	//Draw text on screen
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	sfDrawString(0.9*game.width, 0.57*game.height, "fps:");
	snprintf(text, sizeof(text), "%f", 10/delta_t);
	sfDrawString(0.96*game.width, 0.57*game.height, text);

	sfDrawString(0.9*game.width, 0.60*game.height, "Level:");
	snprintf(text, sizeof(text), "%d", player.level);
	sfDrawString(0.96*game.width, 0.60*game.height, text);

	sfDrawString(0.9*game.width, 0.63*game.height, "Enemies:");
	snprintf(text, sizeof(text), "%d", levels[player.level].enemies_left);
	sfDrawString(0.96*game.width, 0.63*game.height, text);
	glDisable(GL_BLEND);

	// Check if current level has been completed.
	if (levels[player.level].completed == 1)
	{
		if (player.delay_time == 0.0)
		{
			player.delay_time = t;
		}
		
		// If the delay between levels has been reached, load next level.
		if (delay_level(t))
		{
			++player.level;
			//printf("enemies: %d", levels[player.level].enemies);
			init_enemies();
			for (int i = 0; i < levels[player.level].enemies; ++i)
			{
				spawn_enemy(&ttex, tm->vertexArray, terrain_vec, terrain_scale, cam);
			}
			player.delay_time = 0.0;
		}
	}

	// Check for user input
	key_press(t);

	// Check for mouse movement
	mouse_movement();

	// Check if missiles have left the map
	terrain_collision(&ttex, tm->vertexArray, terrain_vec, terrain_scale, t);

	// Move enemies toward the player
	move_enemies(&ttex, tm->vertexArray, terrain_vec, terrain_scale, cam, delta_t);

	// Check if enemies have stepped on a mine
	step_on_mine(t);

	// Remove any enemies that are dead
	check_dead_enemies();

	// Check if player is still alive
	check_player_life(cam);

	// Check if player has killed all enemies on current level
	check_level();

	if (reload_missile(&ttex, tm->vertexArray, terrain_vec, terrain_scale, t_reload_start, t))
	{
		t_reload_start = t;
	}
	
	// Reload landmines if player has pressed r
	if (player.reloading_mines)
	{
		reload_landmines(t);
	}

	glutSwapBuffers();

	// Update Cam matrix
	camMatrix = lookAt(cam.x, cam.y, cam.z,
		lookAtPoint.x, lookAtPoint.y, lookAtPoint.z,
		0.0, 1.0, 0.0);

	// Sets the current moving direction
	go_dir = Normalize(VectorSub(lookAtPoint, cam));

	// Make sure to update old_t to get proper fps.
	old_t = t;
}

void timer(int i)
{
	glutTimerFunc(20, &timer, i);
	glutPostRedisplay();
}

// Used to fire missiles and place landmines.
void mouse_click(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		// Delay between shots.
		if (t - t_prev_shot < 3)
		{}
		else
		{
			// RPG
			if (player.active_weapon == 1)
			{
				fire_missile(x, y, rpg_vec, go_dir, rpg_rot);
				t_prev_shot = t;
			}
			// Landmines
			else if (player.active_weapon == 2)
			{
				//printf("Placing mine");
				place_landmine(cam, go_dir, &ttex, tm->vertexArray, terrain_vec, terrain_scale);
			}
		}
	}
}

void reshape(GLsizei w, GLsizei h)
{
	// Viewport is a separate setting
	glViewport(0, 0, w, h);
	sfSetRasterSize(w, h);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2);
	init_game();
	glutInitWindowSize(game.width, game.height);
	glutCreateWindow ("MAXIMUM MISSILE");
	game_window = GetForegroundWindow();
#ifdef WIN32
	glewInit();
#endif
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse_click);
		//glutMotionFunc(mouseDragged);
	//glutPassiveMotionFunc(mouse_movement);
	glutRepeatingTimer(20);
	init ();
	//glutTimerFunc(20, &timer, 0);

	sfMakeRasterFont(); // init font
	sfSetRasterSize(game.width, game.height);

	glutMainLoop();
	exit(0);
}
