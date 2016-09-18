#ifndef _ROOMS_H
#define _ROOMS_H

#include "tilemap.h"

#define ROOM_SIZE			8

#define ROOM_TILE_NONE		-1
#define ROOM_TILE_FLOOR		0
#define ROOM_TILE_WALL		1
#define ROOM_TILE_WALL_TOP	2
#define ROOM_TILE_OPENING	3

typedef struct rooms*	rooms_t;
typedef int				room_connectionmask;

enum room_flip
{
	ROOM_FLIP_NONE = 0,
	ROOM_FLIP_HORIZONTAL = 1,
	ROOM_FLIP_VERTICAL = 2,

	ROOM_FLIP_BOTH = ROOM_FLIP_HORIZONTAL | ROOM_FLIP_VERTICAL
};

enum room_openings
{
	ROOM_OPENINGS_NONE = 0,
	ROOM_OPENINGS_BOTTOM = 1,
	ROOM_OPENINGS_LEFT = 2,
	ROOM_OPENINGS_RIGHT = 4,
	ROOM_OPENINGS_TOP = 8,

	ROOM_OPENINGS_ANY = 0x10
};

enum room_type
{
	ROOM_TYPE_NONE = 0,
	ROOM_TYPE_SECRET,
	ROOM_TYPE_BOSS,
	ROOM_TYPE_SHOP,
	ROOM_TYPE_START,
	ROOM_TYPE_END
};

enum room_properties
{
	ROOM_PROPERTY_ON_PATH = 1,
	ROOM_PROPERTY_START = 2,
	ROOM_PROPERTY_END = 4
};

struct room
{
	tilemap_t tiles;

	enum room_openings		openings;
	enum room_properties	properties;
	enum room_type			type;

	room_connectionmask		connectionmask;
};

struct room_desc
{
	char tiles[64];

	float probability;

	enum room_flip flip;
	enum room_type type;
};

rooms_t		rooms_init();
void		rooms_shutdown(rooms_t rooms);

void		rooms_get_random(rooms_t rooms, room_connectionmask connectionmask, enum room_type type);
void		rooms_add(rooms_t rooms, struct room_desc* desc);

#endif
