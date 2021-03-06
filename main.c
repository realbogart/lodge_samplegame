/**
 * Author: Johan Yngman <johan.yngman@gmail.com>
 * Date: 2016
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "math4.h"
#include "assets.h"
#include "game.h"
#include "core.h"
#include "util_reload.h"
#include "util_graphics.h"
#include "input.h"
#include "atlas.h"
#include "animatedsprites.h"
#include "top-down/tiles.h"
#include "lodge_window.h"
#include "framebuffer.h"
#include "drawable.h"

#include "tilemap.h"
#include "tilemap_render.h"
#include "level.h"
#include "rooms.h"

#define VIEW_WIDTH		320
#define VIEW_HEIGHT		180

const struct lodge_settings settings = {
	.view_width			= VIEW_WIDTH,
	.view_height		= VIEW_HEIGHT,
	.window_width		= VIEW_WIDTH*4,
	.window_height		= VIEW_HEIGHT*4,
	.window_title		= "fridaynight",
	.sound_listener		= { VIEW_WIDTH / 2.0f, VIEW_HEIGHT / 2.0f, 0.0f },
	.sound_distance_max = 500.0f,
};

struct player
{
	struct sprite sprite;
	struct anim* anim_idle;

	float speed;
	int noclip;
};

struct tile_animations
{
	struct anim* ground[16];
	struct anim* wall[16];
	struct anim* wall_top[16];
};

struct game_textures
{
	GLint diffuse;
	GLint normal;
	GLint depth;
	GLint palette;
};

struct game {
	struct animatedsprites*			batcher;
	struct player					player;
	struct drawable					fullscreen_quad;

	struct console_cmd				cmd_postprocess;
	struct console_cmd				cmd_editmode;

	framebuffer_t					framebuffer;
	texture_t						frame_texture;

	level_t							testroom;
	rooms_t							rooms;

	int								enable_postprocess;
	int								enable_editmode;

	struct game_textures			textures;

	tilemap_render_t				tilemap_render_background;
	tilemap_render_t				tilemap_render;
	tilemap_render_t				tilemap_render_foreground;

	struct tile_animations			tile_animations;

	vec2							camera_pos;
	float							camera_zoom;
} *game = NULL;

struct lodge_settings* game_get_settings()
{
	return &settings;
}

void game_init_memory(struct shared_memory *shared_memory, int reload)
{
	if(!reload) {
		memset(shared_memory->game_memory, 0, sizeof(struct game));
	}

	game = (struct game *) shared_memory->game_memory;
	core_global = shared_memory->core;
	assets = shared_memory->assets;
	vfs_global = shared_memory->vfs;
	input_global = shared_memory->input;
}

void game_think(struct graphics *g, float dt)
{
	shader_uniforms_think(&assets->shaders.tilemap_shader, dt);
	shader_uniforms_think(&assets->shaders.objects, dt);
	shader_uniforms_think(&assets->shaders.obstructed, dt);
	shader_uniforms_think(&assets->shaders.postprocess, dt);

	/* Move player */
	vec2 next_pos;
	vec2 dir;

	set2f(dir, 0.0f, 0.0f);

	//core_console_printf("%f : %f\n", game->player.sprite.position[0], game->player.sprite.position[1]);

	set2f(next_pos, game->player.sprite.position[0], game->player.sprite.position[1]);

	if (key_down(LODGE_KEY_RIGHT))
		dir[0] = 1.0f;
	if (key_down(LODGE_KEY_LEFT))
		dir[0] = -1.0f;
	if (key_down(LODGE_KEY_UP))
		dir[1] = 1.0f;
	if (key_down(LODGE_KEY_DOWN))
		dir[1] = -1.0f;

	norm2f(dir);
	mult2f(dir, game->player.speed, game->player.speed);
	add2f(next_pos, xy_of(dir));

	// X check
	if((level_walkable_at(game->testroom, next_pos[0] + 4.0f, game->player.sprite.position[1] - 4.0f) &&
		level_walkable_at(game->testroom, next_pos[0] - 5.0f, game->player.sprite.position[1] - 4.0f) &&
		level_walkable_at(game->testroom, next_pos[0] + 4.0f, game->player.sprite.position[1] - 12.0f) &&
		level_walkable_at(game->testroom, next_pos[0] - 5.0f, game->player.sprite.position[1] - 12.0f)) ||
		game->player.noclip)
	{
		set2f(game->player.sprite.position, next_pos[0], game->player.sprite.position[1]);
	}

	// Y check
	if((level_walkable_at(game->testroom, game->player.sprite.position[0] + 4.0f, next_pos[1] - 4.0f) &&
		level_walkable_at(game->testroom, game->player.sprite.position[0] - 5.0f, next_pos[1] - 4.0f) &&
		level_walkable_at(game->testroom, game->player.sprite.position[0] + 4.0f, next_pos[1] - 12.0f) &&
		level_walkable_at(game->testroom, game->player.sprite.position[0] - 5.0f, next_pos[1] - 12.0f)) ||
		game->player.noclip)
	{
		set2f(game->player.sprite.position, game->player.sprite.position[0], next_pos[1]);
	}

	/* Move camera to player */
	float level_width, level_height;
	level_get_dimensions(game->testroom, &level_width, &level_height);

	vec2 camera_target;
	set2f(camera_target, xy_of(game->player.sprite.position));
	
	//camera_target[0] = max(camera_target[0], (VIEW_WIDTH / 2) / game->camera_zoom) - LEVEL_TILE_SIZE / 2.0f;
	//camera_target[0] = min(camera_target[0], level_width - (VIEW_WIDTH / 2) / game->camera_zoom - LEVEL_TILE_SIZE / 2.0f);
	//camera_target[1] = min(camera_target[1], - (VIEW_HEIGHT / 2) / game->camera_zoom) + LEVEL_TILE_SIZE / 2.0f;
	//camera_target[1] = max(camera_target[1], - level_height + (VIEW_HEIGHT / 2) / game->camera_zoom + LEVEL_TILE_SIZE / 2.0f);
	lerp2f(game->camera_pos, camera_target, 0.01f*dt);

	animatedsprites_update(game->batcher, &assets->pyxels.textures.atlas, dt);

	tilemap_render_update(game->tilemap_render_background, &assets->pyxels.textures.atlas, dt);
	tilemap_render_update(game->tilemap_render, &assets->pyxels.textures.atlas, dt);
	tilemap_render_update(game->tilemap_render_foreground, &assets->pyxels.textures.atlas, dt);
}

void game_render(struct graphics *g, float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glEnable(GL_ALPHA_TEST);
	//glAlphaFunc(GL_GREATER, 0);

	mat4 transform;
	mat4 offset;
	mat4 scale_m;

	mat4 final;

	set2f(offset, 0.0f, 0.0f);
	translate(offset, -game->camera_pos[0] + (VIEW_WIDTH / 2) / game->camera_zoom, -game->camera_pos[1] + (VIEW_HEIGHT / 2) / game->camera_zoom, 0.0f);
	scale(scale_m, game->camera_zoom, game->camera_zoom, 1.0f);
	mult(offset, scale_m, offset);

	transpose(final, offset);

	identity(transform);

	glUseProgram(assets->shaders.tilemap_shader.program);

	GLint uniform_transform = glGetUniformLocation(assets->shaders.tilemap_shader.program, "transform");
	glUniformMatrix4fv(uniform_transform, 1, GL_FALSE, final);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, game->textures.diffuse);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, game->textures.normal);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, game->textures.depth);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, game->textures.palette);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (game->enable_postprocess)
	{
		framebuffer_bind(game->framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	tilemap_render_render(game->tilemap_render_background, &assets->shaders.tilemap_shader);
	tilemap_render_render(game->tilemap_render, &assets->shaders.tilemap_shader);

	animatedsprites_render_simple(game->batcher, &assets->shaders.objects, game->textures.diffuse, g->projection, final);
	
	//glEnable(GL_STENCIL_TEST);
	//glEnable(GL_DEPTH_TEST);
	//glClear(GL_STENCIL_BUFFER_BIT);
	//glStencilFunc(GL_ALWAYS, 10, 0xFF);
	//glStencilOpSeparate(GL_FRONT, GL_REPLACE, GL_REPLACE, GL_REPLACE);
	//glStencilMask(0xFF);

	tilemap_render_render(game->tilemap_render_foreground, &assets->shaders.tilemap_shader);

	//glStencilFunc(GL_EQUAL, 10, 0xFF);
	//glStencilMask(0);
	animatedsprites_render_simple(game->batcher, &assets->shaders.obstructed, game->textures.diffuse, g->projection, final);

	if (game->enable_postprocess)
	{
		framebuffer_unbind();
		//glViewport(VIEW_WIDTH, VIEW_WIDTH, VIEW_WIDTH, VIEW_HEIGHT);
		glUseProgram(assets->shaders.postprocess.program);
		texture_bind(game->frame_texture, 0);
		drawable_render(&game->fullscreen_quad);
		glUseProgram(0);
	}

	glDisable(GL_STENCIL_TEST);
}

void game_mousebutton_callback(lodge_window_t window, int button, int action, int mods)
{
	if(action == LODGE_PRESS) {
		float x = 0, y = 0;
		util_view_get_cursor(window, &x, &y, core_global->graphics.projection);
	}
}

void toggle_postprocess(struct console* c, struct console_cmd* cmd, struct list* argv)
{
	game->enable_postprocess = !game->enable_postprocess;
}

void toggle_editmode(struct console* c, struct console_cmd* cmd, struct list* argv)
{
	game->enable_editmode = !game->enable_editmode;

	if (game->enable_editmode)
	{
		game->camera_zoom = 0.3f;
		game->player.noclip = 1;
		game->player.speed = 5.0f;
	}
	else
	{
		game->camera_zoom = 1.0f;
		game->player.noclip = 0;
		game->player.speed = 0.5f;
	}
}

void game_console_init(struct console *c, struct env *env)
{
	game->cmd_postprocess.argc = 0;
	game->cmd_postprocess.callback = &toggle_postprocess;
	strcpy(game->cmd_postprocess.name, "postprocess");
	console_cmd_add(&game->cmd_postprocess, &c->root_cmd);

	game->cmd_editmode.argc = 0;
	game->cmd_editmode.callback = &toggle_editmode;
	strcpy(game->cmd_editmode.name, "editmode");
	console_cmd_add(&game->cmd_editmode, &c->root_cmd);

	env_bind_2f(&core_global->env, "player_position", &game->player.sprite.position);
	env_bind_1f(&core_global->env, "player_speed", &game->player.speed);
	env_bind_bool(&core_global->env, "player_noclip", &game->player.noclip);
}

void spawn_room(level_t level, int width, int height, int x, int y)
{
	spawn_box(game->testroom, width, height, x, y);
	carve_box(game->testroom, width-2, height-2, x+1, y+1);
}

void testlevel_init()
{
	int width, height;

	level_get_tile_dimensions(game->testroom, &width, &height);

	//spawn_box(game->testroom, width, height, 0, 0);
	//spawn_room(game->testroom, 8, 8, 0, 0);
	//spawn_room(game->testroom, 8, 8, 8, 8);

	for (int y = 0; y >= 0 && y < height; y++)
	{
		for (int x = 0; x >= 0 && x < width; x++)
		{
			if (rand() % 8 == 0)
			{
				level_place_wall(game->testroom, x, y);
			}
			else
			{
				level_place_floor(game->testroom, x, y);
			}
		}
	}
}

struct anim* get_tile_anim(int id)
{
	int type = id & LEVEL_TILE_TYPEMASK;
	int variation = (id >> TILE_TYPE_SHIFT) & LEVEL_TILE_TYPEMASK;

	if (type == ROOM_TILE_WALL)
		return game->tile_animations.wall[variation];
	else if (type == ROOM_TILE_WALL_TOP)
		return game->tile_animations.wall_top[variation];
	else if (type == ROOM_TILE_FLOOR)
		return game->tile_animations.ground[variation];
	else
		return 0;
}

void game_init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	int screen_width, screen_height;

	lodge_window_get_screen_size(&screen_width, &screen_height);

	game->framebuffer = framebuffer_create();
	game->frame_texture = texture_create_rgba(screen_width, screen_height);

	framebuffer_bind(game->framebuffer);
	framebuffer_attach_texture(game->framebuffer, game->frame_texture, FRAMEBUFFER_TARGET_COLOR);
	framebuffer_unbind();

	game->enable_postprocess = 0;

	//drawable_new_rect_solidf(&game->fullscreen_quad, VIEW_WIDTH / 2.0f, VIEW_HEIGHT / 2.0f, VIEW_WIDTH, VIEW_HEIGHT, &assets->shaders.postprocess);
	drawable_new_rect_fullscreen(&game->fullscreen_quad, &assets->shaders.postprocess);

	/* Create animated sprite batcher. */
	game->batcher = animatedsprites_create();

	int level_width = 32;
	int level_height = 32;

	srand(time(0));

	/* Setup map */
	game->rooms = rooms_init();
	game->testroom = level_create(game->rooms, level_width, level_height);
	level_generate(game->testroom, rand());

	//testlevel_init();

	/* Setup player */
	game->player.anim_idle = pyxel_asset_get_anim(&assets->pyxels.textures, "bomb");
	set3f(game->player.sprite.position, 80.0f, -80.0f, 0.0f);
	set3f(game->camera_pos, game->player.sprite.position[0], game->player.sprite.position[1], game->player.sprite.position[3]);
	set2f(game->player.sprite.scale, 0.5f, 0.5f);
	game->player.speed = 0.5f;
	game->player.noclip = 0;

	/* Setup shaders */
	shader_constant_uniform1i(&assets->shaders.postprocess, "texture_diffuse", 0);
	//shader_uniform_matrix4f(&assets->shaders.postprocess, "projection", &core_global->graphics.projection);

	shader_constant_uniform1i(&assets->shaders.tilemap_shader, "diffuse", 0);
	shader_constant_uniform1i(&assets->shaders.tilemap_shader, "normal", 1);
	shader_constant_uniform1i(&assets->shaders.tilemap_shader, "depth", 2);
	shader_constant_uniform1i(&assets->shaders.tilemap_shader, "palette", 3);
	shader_uniform3f(&assets->shaders.tilemap_shader, "lightpos", &game->player.sprite.position);
	shader_uniform_matrix4f(&assets->shaders.tilemap_shader, "projection", &core_global->graphics.projection);

	/* Load tile variations*/
	const char* tile_name[] = {"ground", "wall", "wall_top"};
	struct anim** tile_variation[] = { game->tile_animations.ground, game->tile_animations.wall, game->tile_animations.wall_top };

	char buffer[256];

	for (int i = 0, i_size = sizeof(tile_name)/sizeof(tile_name[0]); i < i_size; i++)
	{
		struct anim* base = pyxel_asset_get_anim(&assets->pyxels.textures, tile_name[i]);

		for (int j = 0; j < 16; j++)
		{
			sprintf(buffer, "%s_%d", tile_name[i], j);
			tile_variation[i][j] = pyxel_asset_get_anim(&assets->pyxels.textures, buffer);

			if (tile_variation[i][j] == 0)
				tile_variation[i][j] = base;
		}
	}

	/* Create tile renderers */
	game->textures.diffuse = assets->pyxels.textures.layers[2];
	game->textures.normal = assets->pyxels.textures.layers[1];
	game->textures.depth = assets->pyxels.textures.layers[0];
	game->textures.palette = assets->textures.palette;

	game->tilemap_render_background = tilemap_render_create(level_get_tiles_background(game->testroom), &get_tile_anim, 16.0f);
	game->tilemap_render = tilemap_render_create(level_get_tiles(game->testroom), &get_tile_anim, 16.0f);
	game->tilemap_render_foreground = tilemap_render_create(level_get_tiles_foreground(game->testroom), &get_tile_anim, 16.0f);

	animatedsprites_playanimation(&game->player.sprite, game->player.anim_idle);
	animatedsprites_add(game->batcher, &game->player.sprite);

	/* Set camera position */
	set2f(game->camera_pos, 0.0f, 0.0f);
	game->camera_zoom = 1.0f;

	env_bind_1f(&core_global->env, "camera_zoom", &game->camera_zoom);
}

void game_key_callback(lodge_window_t window, int key, int scancode, int action, int mods)
{
	if(action == LODGE_PRESS) {
		switch(key) {
			case LODGE_KEY_ESCAPE:
				lodge_window_destroy(window);
				break;
			case LODGE_KEY_ENTER:
				lodge_window_toggle_fullscreen(window);
				break;
			case LODGE_KEY_Z:
				lodge_window_toggle_fullscreen(window);
				break;
			case LODGE_KEY_X:
				level_generate(game->testroom, rand());
				tilemap_render_set(game->tilemap_render_background, level_get_tiles_background(game->testroom));
				tilemap_render_set(game->tilemap_render, level_get_tiles(game->testroom));
				tilemap_render_set(game->tilemap_render_foreground, level_get_tiles_foreground(game->testroom));
				break;
		}
	}
}

void game_assets_load()
{
	assets_load();
}

void game_assets_release()
{
	level_destroy(game->testroom);
	rooms_shutdown(game->rooms);

	assets_release();
}

void game_fps_callback(struct frames *f)
{
}
