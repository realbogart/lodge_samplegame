#include "level.h"

#include <math.h>

struct level
{
	tilemap_t tiles_background;
	tilemap_t tiles;
	tilemap_t tiles_foreground;

	int width;
	int height;
};

level_t level_create(int width, int height)
{
	struct level* level = malloc(sizeof(struct level));

	level->width = width;
	level->height = height;

	level->tiles_background = tilemap_create(width, height);
	level->tiles = tilemap_create(width, height);
	level->tiles_foreground = tilemap_create(width, height);

	return level;
}

void level_destroy(level_t room)
{
	free(room);
}

enum level_tile_mask tile_get_mask(tilemap_t tilemap, int x, int y, int type)
{
	if (type == LEVEL_TILE_NONE)
		return 0;

	enum level_tile_mask mask = 0;

	int type_bottom = tilemap_get_id_at(tilemap, x, y + 1) & LEVEL_TILE_TYPEMASK;
	int type_left = tilemap_get_id_at(tilemap, x - 1, y) & LEVEL_TILE_TYPEMASK;
	int type_right = tilemap_get_id_at(tilemap, x + 1, y) & LEVEL_TILE_TYPEMASK;
	int type_top = tilemap_get_id_at(tilemap, x, y - 1) & LEVEL_TILE_TYPEMASK;

	if (type_bottom == type)
		mask |= LEVEL_TILE_BOTTOM;
	if (type_left == type)
		mask |= LEVEL_TILE_LEFT;
	if (type_right == type)
		mask |= LEVEL_TILE_RIGHT;
	if (type_top == type)
		mask |= LEVEL_TILE_TOP;

	return mask;
}

void tile_update(tilemap_t tilemap, int x, int y)
{
	int id = tilemap_get_id_at(tilemap, x, y);

	if (id != LEVEL_TILE_NONE)
	{
		int type = id & LEVEL_TILE_TYPEMASK;
		enum level_tile_mask mask = tile_get_mask(tilemap, x, y, type);

		id = mask;
		id = id << TILE_TYPE_SHIFT;
		id |= (int)type;

		tilemap_set_id_at(tilemap, x, y, id);
	}
}

void tile_place(tilemap_t tilemap, int x, int y, int type)
{
	if (type != LEVEL_TILE_NONE)
	{
		enum level_tile_mask mask = tile_get_mask(tilemap, x, y, type);

		int id = mask;
		id = id << TILE_TYPE_SHIFT;
		id |= (int)type;

		tilemap_set_id_at(tilemap, x, y, id);
	}
	else
	{
		tilemap_set_id_at(tilemap, x, y, LEVEL_TILE_NONE);
	}

	// Update neighbours
	tile_update(tilemap, x - 1, y);
	tile_update(tilemap, x + 1, y);
	tile_update(tilemap, x, y - 1);
	tile_update(tilemap, x, y + 1);
}

int tile_get_type_at(tilemap_t tilemap, int x, int y)
{
	return tilemap_get_id_at(tilemap, x, y) & LEVEL_TILE_TYPEMASK;
}

void level_place_wall(level_t level, int x, int y)
{
	tile_place(level->tiles_background, x, y, LEVEL_TILE_FLOOR);
	tile_place(level->tiles, x, y, LEVEL_TILE_WALL);
	tile_place(level->tiles_foreground, x, y - 1, LEVEL_TILE_WALL_TOP);
}

void level_place_floor(level_t level, int x, int y)
{
	tile_place(level->tiles_background, x, y, LEVEL_TILE_FLOOR);
	tile_place(level->tiles, x, y, LEVEL_TILE_FLOOR);
	tile_place(level->tiles_foreground, x, y - 1, LEVEL_TILE_NONE);
}

int level_walkable_at(level_t level, float x, float y)
{
	tilemap_t tilemap = level_get_tiles(level);

	int world_width, world_height;
	tilemap_get_dimensions(tilemap, &world_width, &world_height);

	int grid_x = (int)floor((x + LEVEL_TILE_SIZE/2.0f) / LEVEL_TILE_SIZE);
	int grid_y = (int)floor((-y + LEVEL_TILE_SIZE/2.0f) / LEVEL_TILE_SIZE);

	int id = tilemap_get_id_at(tilemap, grid_x, grid_y);

	if (id == -1)
		return 0;

	id &= LEVEL_TILE_TYPEMASK;
	
	if (id == LEVEL_TILE_WALL)
		return 0;

	return 1;
}

tilemap_t level_get_tiles_background(level_t level)
{
	return level->tiles_background;
}

tilemap_t level_get_tiles(level_t level)
{
	return level->tiles;
}

tilemap_t level_get_tiles_foreground(level_t level)
{
	return level->tiles_foreground;
}

void level_get_tile_dimensions(level_t level, int* width, int* height)
{
	*width = level->width;
	*height = level->height;
}

void level_get_dimensions(level_t level, float* width, float* height)
{
	*width = level->width * LEVEL_TILE_SIZE;
	*height = level->height * LEVEL_TILE_SIZE;
}
