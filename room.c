#include "room.h"

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

void room_place_wall(room_t room, int x, int y)
{
	tilemap_set_id_at(room->tiles_background, x, y, ROOM_TILE_FLOOR);
	tilemap_set_id_at(room->tiles, x, y, ROOM_TILE_WALL);
	tilemap_set_id_at(room->tiles_foreground, x, y + 1, ROOM_TILE_WALL_TOP);
}

void room_place_floor(room_t room, int x, int y)
{
	tilemap_set_id_at(room->tiles_background, x, y, -1);
	tilemap_set_id_at(room->tiles, x, y, ROOM_TILE_FLOOR);
	tilemap_set_id_at(room->tiles_foreground, x, y + 1, -1);
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
