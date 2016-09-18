#include "rooms.h"

struct room_desc room_database[] =
{
	{
		"...oo..."\
		"...    ."\
		"...    ."\
		"...    ."\
		"...    ."\
		"...    ."\
		"...    ."\
		"...oo...", 0.5f
	},
	{
		"....oo.."\
		".      ."\
		"o      o"\
		"o  ... o"\
		".      ."\
		"..     ."\
		"...    ."\
		"....oo..", 0.1f
	},
	{
		"oooooooo"\
		"o      o"\
		"o      o"\
		"o  ..  o"\
		"o  ..  o"\
		"o      o"\
		"o      o"\
		"oooooooo", 1.0f
	}
};

struct rooms
{
	struct room_desc* rooms;
	int rooms_count;
};

rooms_t rooms_init()
{
	struct rooms* rooms = malloc(sizeof(struct rooms));
	rooms->rooms_count = (sizeof(room_database) / sizeof(room_database[0]));
	rooms->rooms = malloc(sizeof(struct room_desc) * rooms->rooms_count);

	for(int i = 0; i < rooms->rooms_count; i++)
	{

	}

	return rooms;
}

void rooms_shutdown(rooms_t rooms)
{
	free(rooms->rooms);
	free(rooms);
}
