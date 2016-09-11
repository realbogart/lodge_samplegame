#ifndef _ROOM_H
#define _ROOM_H

#include "tilemap.h"

typedef struct room* room_t;

enum room_tile_mask
{
	ROOM_TILE_BOTTOM	= 1,
	ROOM_TILE_LEFT		= 2,
	ROOM_TILE_RIGHT		= 4,
	ROOM_TILE_TOP		= 8,

	ROOM_TILE_TYPEMASK	= 255,
};

#define TILE_TYPE_SHIFT		8

#define ROOM_TILE_NONE		-1

#define ROOM_TILE_FLOOR		0
#define ROOM_TILE_WALL		1
#define ROOM_TILE_WALL_TOP	2

room_t		room_create(int width, int height);
void		room_destroy(room_t room);

void		room_place_wall(room_t room, int x, int y);
void		room_place_floor(room_t room, int x, int y);

int			room_walkable_at(room_t room, float x, float y);

tilemap_t	room_get_tiles_background(room_t room);
tilemap_t	room_get_tiles(room_t room);
tilemap_t	room_get_tiles_foreground(room_t room);

void		room_get_dimensions(room_t room, int* width, int* height);

#endif
