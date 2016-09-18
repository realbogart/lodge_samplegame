#include "rooms.h"

#define ROOMS_COUNT_MAX 1024

/* TODO: Load this from file? */
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
		"...oo...", 0.5f, ROOM_FLIP_NONE
	},
	{
		"...oo..."\
		".      ."\
		".      ."\
		"o  ... o"\
		"o      o"\
		"..     ."\
		"...    ."\
		"...oo...", 0.1f, ROOM_FLIP_NONE
	},
	{
		"oooooooo"\
		"o      o"\
		"o      o"\
		"o  ..  o"\
		"o  ..  o"\
		"o      o"\
		"o      o"\
		"oooooooo", 1.0f, ROOM_FLIP_NONE
	}
};

struct rooms
{
	struct room rooms[ROOMS_COUNT_MAX];
	int rooms_count;
};

void rooms_load_database(rooms_t rooms)
{
	int rooms_database_count = sizeof(room_database) / sizeof(room_database[0]);

	for (int i = 0; i < rooms_database_count; i++)
		rooms_add(rooms, &room_database[i]);
}

void room_generate_connectionmask(struct room* room)
{
	room_connectionmask top;
	room_connectionmask left;
	room_connectionmask right;
	room_connectionmask bottom;
	room_connectionmask final;

	top = left = right = bottom = final = 0;

	// Top and bottom
	for (int x = 0; x < ROOM_SIZE; x++)
	{
		int id_top = tilemap_get_id_at(room->tiles, x, 0);
		int id_bottom = tilemap_get_id_at(room->tiles, x, ROOM_SIZE - 1);

		top = top << 1;
		bottom = bottom << 1;

		if (id_top == ROOM_TILE_OPENING)
			top |= 1;

		if (id_bottom == ROOM_TILE_OPENING)
			bottom |= 1;
	}

	// Left and right
	for (int y = 0; y < ROOM_SIZE; y++)
	{
		int id_left = tilemap_get_id_at(room->tiles, 0, y);
		int id_right = tilemap_get_id_at(room->tiles, ROOM_SIZE - 1, y);

		left = left << 1;
		right = right << 1;

		if (id_left == ROOM_TILE_OPENING)
			left |= 1;

		if (id_right == ROOM_TILE_OPENING)
			right |= 1;
	}

	final |= top << 24;
	final |= left << 16;
	final |= right << 8;
	final |= bottom;

	room->connectionmask = final;
}

void rooms_add(rooms_t rooms, struct room_desc* desc)
{
	struct room* room = &rooms->rooms[rooms->rooms_count];
	room->tiles = tilemap_create(ROOM_SIZE, ROOM_SIZE);

	for (int y = 0; y < ROOM_SIZE; y++)
	{
		for (int x = 0; x < ROOM_SIZE; x++)
		{
			switch (desc->tiles[y*ROOM_SIZE + x])
			{
			case ' ':
				tilemap_set_id_at(room->tiles, x, y, ROOM_TILE_FLOOR);
				break;
			case '.':
				tilemap_set_id_at(room->tiles, x, y, ROOM_TILE_WALL);
				break;
			case 'o':
				tilemap_set_id_at(room->tiles, x, y, ROOM_TILE_OPENING);
				break;
			default:
				break;
			}
		}
	}

	room_generate_connectionmask(room);
	room->type = 0;

	rooms->rooms_count++;
}

rooms_t rooms_init()
{
	struct rooms* rooms = malloc(sizeof(struct rooms));
	rooms->rooms_count = 0;

	rooms_load_database(rooms);

	return rooms;
}

void rooms_shutdown(rooms_t rooms)
{
	free(rooms);
}

struct room* rooms_get_random(rooms_t rooms, room_connectionmask connectionmask, enum room_type type)
{
	for (int i = rand() % rooms->rooms_count, tries = 0;
		tries < rooms->rooms_count; 
		tries++, i = ++i % rooms->rooms_count)
	{
		room_connectionmask test_top = connectionmask & MASK_TOP;
		room_connectionmask test_left = connectionmask & MASK_LEFT;
		room_connectionmask test_right = connectionmask & MASK_RIGHT;
		room_connectionmask test_bottom = connectionmask & MASK_BOTTOM;

		if ((!test_top || rooms->rooms[i].connectionmask & test_top) &&
			(!test_left || rooms->rooms[i].connectionmask & test_left) &&
			(!test_right || rooms->rooms[i].connectionmask & test_right) &&
			(!test_bottom || rooms->rooms[i].connectionmask & test_bottom))
		{
			return &rooms->rooms[i];
		}
	}

	return 0;
}
