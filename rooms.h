#ifndef _ROOMS_H
#define _ROOMS_H

typedef struct rooms* rooms_t;

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

struct room_desc
{
	char tiles[64];

	float probability;
};

rooms_t		rooms_init();
void		rooms_shutdown(rooms_t rooms);

void		rooms_get_random(rooms_t rooms, enum room_openings openings, enum room_type type);

#endif
