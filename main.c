/**
 * TODO: Write header.
 *
 * Author: AUTHOR_NAME <AUTHOR_EMAIL>
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

#include "tilemap.h"
#include "tilemap_render.h"
#include "room.h"

#define VIEW_WIDTH		320
#define VIEW_HEIGHT		180

const struct lodge_settings settings = {
	.view_width			= VIEW_WIDTH,
	.view_height		= VIEW_HEIGHT,
	.window_width		= VIEW_WIDTH,
	.window_height		= VIEW_HEIGHT,
	.window_title		= "fridaynight",
	.sound_listener		= { VIEW_WIDTH / 2.0f, VIEW_HEIGHT / 2.0f, 0.0f },
	.sound_distance_max = 500.0f,
};

struct player
{
	struct sprite sprite;
	struct anim* anim_idle;

	float speed;
};

struct tile_animations
{
	struct anim* ground;
	struct anim* wall;
	struct anim* wall_top;
};

struct game {
	struct animatedsprites*	batcher;
	struct player			player;

	room_t					testroom;

	tilemap_render_t		tilemap_render_background;
	tilemap_render_t		tilemap_render;
	tilemap_render_t		tilemap_render_foreground;

	struct tile_animations	tile_animations;

	vec2					camera_pos;
	float					camera_zoom;
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

int room_walkable_at(room_t room, float x, float y)
{
	tilemap_t tilemap = room_get_tiles(game->testroom);

	int world_width, world_height;
	tilemap_get_dimensions(tilemap, &world_width, &world_height);

	float inv_y = world_height - y;

	int grid_x = (int)floor((x + 8.0f) / 16.0f);
	int grid_y = (int)floor((inv_y - 24.0f) / 16.0f);

	int id = tilemap_get_id_at(tilemap, grid_x, grid_y);

	if (id == ROOM_TILE_WALL || id == -1)
		return 0;

	return 1;
}

void game_think(struct graphics *g, float dt)
{
	/* Move player */
	vec2 next_pos;
	vec2 dir;

	set2f(dir, 0.0f, 0.0f);

	core_console_printf("%f : %f\n", game->player.sprite.position[0], game->player.sprite.position[1]);

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
	if (room_walkable_at(game->testroom, next_pos[0] + 4.0f, game->player.sprite.position[1] - 4.0f) &&
		room_walkable_at(game->testroom, next_pos[0] - 5.0f, game->player.sprite.position[1] - 4.0f) &&
		room_walkable_at(game->testroom, next_pos[0] + 4.0f, game->player.sprite.position[1] - 8.0f) &&
		room_walkable_at(game->testroom, next_pos[0] - 5.0f, game->player.sprite.position[1] - 8.0f))
	{
		set2f(game->player.sprite.position, next_pos[0], game->player.sprite.position[1]);
	}

	// Y check
	if (room_walkable_at(game->testroom, game->player.sprite.position[0] + 4.0f, next_pos[1] - 4.0f) &&
		room_walkable_at(game->testroom, game->player.sprite.position[0] - 5.0f, next_pos[1] - 4.0f) &&
		room_walkable_at(game->testroom, game->player.sprite.position[0] + 4.0f, next_pos[1] - 8.0f) &&
		room_walkable_at(game->testroom, game->player.sprite.position[0] - 5.0f, next_pos[1] - 8.0f))
	{
		set2f(game->player.sprite.position, game->player.sprite.position[0], next_pos[1]);
	}

	/* Move camera to player */
	lerp2f(game->camera_pos, game->player.sprite.position, 0.01f*dt);

	animatedsprites_update(game->batcher, &assets->pyxels.textures.atlas, dt);

	tilemap_render_update(game->tilemap_render_background, &assets->pyxels.textures.atlas, dt);
	tilemap_render_update(game->tilemap_render, &assets->pyxels.textures.atlas, dt);
	tilemap_render_update(game->tilemap_render_foreground, &assets->pyxels.textures.atlas, dt);

	shader_uniforms_think(&assets->shaders.basic_shader, dt);
}

void game_render(struct graphics *g, float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	tilemap_render_render(game->tilemap_render_background, &assets->shaders.basic_shader, g, assets->pyxels.textures.layers[0], final);
	tilemap_render_render(game->tilemap_render, &assets->shaders.basic_shader, g, assets->pyxels.textures.layers[0], final);

	animatedsprites_render(game->batcher, &assets->shaders.basic_shader, g, assets->pyxels.textures.layers[0], final);

	tilemap_render_render(game->tilemap_render_foreground, &assets->shaders.basic_shader, g, assets->pyxels.textures.layers[0], final);
}

void game_mousebutton_callback(lodge_window_t window, int button, int action, int mods)
{
	if(action == LODGE_PRESS) {
		float x = 0, y = 0;
		util_view_get_cursor(window, &x, &y, core_global->graphics.projection);
	}
}

void game_console_init(struct console *c, struct env *env)
{
	/* env_bind_1f(c, "print_fps", &(game->print_fps)); */
}

void testroom_init()
{
	int width, height;

	room_get_dimensions(game->testroom, &width, &height);

	for (int y = 0; y >= 0 && y < height; y++)
	{
		for (int x = 0; x >= 0 && x < width; x++)
		{
			if (rand() % 8 == 0)
			{
				room_place_wall(game->testroom, x, y);
			}
			else
			{
				room_place_floor(game->testroom, x, y);
			}
		}
	}
}

struct anim* get_tile_anim(int id)
{
	if (id == ROOM_TILE_WALL)
		return game->tile_animations.wall;
	else if (id == ROOM_TILE_WALL_TOP)
		return game->tile_animations.wall_top;
	else if (id == ROOM_TILE_FLOOR)
		return game->tile_animations.ground;
	else
		return 0;
}

void game_init()
{
	/* Create animated sprite batcher. */
	game->batcher = animatedsprites_create();

	int room_width = 32;
	int room_height = 32;

	/* Setup map */
	game->testroom = room_create(room_width, room_height);
	testroom_init();

	/* Setup player */
	game->player.anim_idle = pyxel_asset_get_anim(&assets->pyxels.textures, "player_idle");
	set3f(game->player.sprite.position, 40.0f, 0.0f, 0.0f);
	set3f(game->camera_pos, game->player.sprite.position[0], game->player.sprite.position[1], game->player.sprite.position[3]);
	set2f(game->player.sprite.scale, 1.0f, 1.0f);
	game->player.speed = 0.8f;

	/* Create tile renderers */
	game->tile_animations.ground = pyxel_asset_get_anim(&assets->pyxels.textures, "ground");
	game->tile_animations.wall = pyxel_asset_get_anim(&assets->pyxels.textures, "wall");
	game->tile_animations.wall_top = pyxel_asset_get_anim(&assets->pyxels.textures, "wall_top");

	game->tilemap_render_background = tilemap_render_create(room_get_tiles_background(game->testroom), &get_tile_anim, 16.0f);
	game->tilemap_render = tilemap_render_create(room_get_tiles(game->testroom), &get_tile_anim, 16.0f);
	game->tilemap_render_foreground = tilemap_render_create(room_get_tiles_foreground(game->testroom), &get_tile_anim, 16.0f);

	animatedsprites_playanimation(&game->player.sprite, game->player.anim_idle);
	animatedsprites_add(game->batcher, &game->player.sprite);

	/* Set camera position */
	set2f(game->camera_pos, 0.0f, 0.0f);
	game->camera_zoom = 1.0f;
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
		}
	}
}

void game_assets_load()
{
	assets_load();
}

void game_assets_release()
{
	room_destroy(game->testroom);

	assets_release();
}

void game_fps_callback(struct frames *f)
{
}
