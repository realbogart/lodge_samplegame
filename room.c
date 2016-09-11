#include "room.h"

#include <math.h>

struct room
{
	tilemap_t tiles_background;
	tilemap_t tiles;
	tilemap_t tiles_foreground;

	int width;
	int height;
};

room_t room_create(int width, int height)
{
	struct room* room = malloc(sizeof(struct room));

	room->width = width;
	room->height = height;

	room->tiles_background = tilemap_create(width, height);
	room->tiles = tilemap_create(width, height);
	room->tiles_foreground = tilemap_create(width, height);

	return room;
}

void room_destroy(room_t room)
{
	free(room);
}

enum room_tile_mask tile_get_mask(tilemap_t tilemap, int x, int y, int type)
{
	if (type == ROOM_TILE_NONE)
		return 0;

	enum room_tile_mask mask = 0;

	int type_bottom = tilemap_get_id_at(tilemap, x, y + 1) & ROOM_TILE_TYPEMASK;
	int type_left = tilemap_get_id_at(tilemap, x - 1, y) & ROOM_TILE_TYPEMASK;
	int type_right = tilemap_get_id_at(tilemap, x + 1, y) & ROOM_TILE_TYPEMASK;
	int type_top = tilemap_get_id_at(tilemap, x, y - 1) & ROOM_TILE_TYPEMASK;

	if (type_bottom == type)
		mask |= ROOM_TILE_BOTTOM;
	if (type_left == type)
		mask |= ROOM_TILE_LEFT;
	if (type_right == type)
		mask |= ROOM_TILE_RIGHT;
	if (type_top == type)
		mask |= ROOM_TILE_TOP;

	return mask;
}

void tile_update(tilemap_t tilemap, int x, int y)
{
	int id = tilemap_get_id_at(tilemap, x, y);

	if (id != ROOM_TILE_NONE)
	{
		int type = id & ROOM_TILE_TYPEMASK;
		enum room_tile_mask mask = tile_get_mask(tilemap, x, y, type);

		id = mask;
		id = id << TILE_TYPE_SHIFT;
		id |= (int)type;

		tilemap_set_id_at(tilemap, x, y, id);
	}
}

void tile_place(tilemap_t tilemap, int x, int y, int type)
{
	if (type != ROOM_TILE_NONE)
	{
		enum room_tile_mask mask = tile_get_mask(tilemap, x, y, type);

		int id = mask;
		id = id << TILE_TYPE_SHIFT;
		id |= (int)type;

		tilemap_set_id_at(tilemap, x, y, id);
	}
	else
	{
		tilemap_set_id_at(tilemap, x, y, ROOM_TILE_NONE);
	}

	// Update neighbours
	tile_update(tilemap, x - 1, y);
	tile_update(tilemap, x + 1, y);
	tile_update(tilemap, x, y - 1);
	tile_update(tilemap, x, y + 1);
}

int tile_get_type_at(tilemap_t tilemap, int x, int y)
{
	return tilemap_get_id_at(tilemap, x, y) & ROOM_TILE_TYPEMASK;
}

void room_place_wall(room_t room, int x, int y)
{
	tile_place(room->tiles_background, x, y, ROOM_TILE_FLOOR);
	tile_place(room->tiles, x, y, ROOM_TILE_WALL);

	//if (tile_get_type_at(room->tiles, x, y - 1) != ROOM_TILE_WALL)
		tile_place(room->tiles_foreground, x, y - 1, ROOM_TILE_WALL_TOP);
}

void room_place_floor(room_t room, int x, int y)
{
	tile_place(room->tiles_background, x, y, ROOM_TILE_FLOOR);
	tile_place(room->tiles, x, y, ROOM_TILE_FLOOR);
	tile_place(room->tiles_foreground, x, y - 1, ROOM_TILE_NONE);
}

int room_walkable_at(room_t room, float x, float y)
{
	tilemap_t tilemap = room_get_tiles(room);

	int world_width, world_height;
	tilemap_get_dimensions(tilemap, &world_width, &world_height);

	float inv_y = world_height - y;

	int grid_x = (int)floor((x + 8.0f) / 16.0f);
	int grid_y = (int)floor((inv_y - 24.0f) / 16.0f);

	int id = tilemap_get_id_at(tilemap, grid_x, grid_y);

	if (id == -1)
		return 0;

	id &= ROOM_TILE_TYPEMASK;
	
	if (id == ROOM_TILE_WALL)
		return 0;

	return 1;
}

tilemap_t room_get_tiles_background(room_t room)
{
	return room->tiles_background;
}

tilemap_t room_get_tiles(room_t room)
{
	return room->tiles;
}

tilemap_t room_get_tiles_foreground(room_t room)
{
	return room->tiles_foreground;
}

void room_get_dimensions(room_t room, int* width, int* height)
{
	*width = room->width;
	*height = room->height;
}
