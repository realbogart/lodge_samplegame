#include "level.h"
#include "rooms.h"

#include <math.h>

enum path_properties
{
	PATH_PROPERTY_ON_PATH = 1,
	PATH_PROPERTY_START = 2,
	PATH_PROPERTY_END = 4
};

enum path_openings
{
	PATH_OPENINGS_NONE = 0,
	PATH_OPENINGS_BOTTOM = 1,
	PATH_OPENINGS_LEFT = 2,
	PATH_OPENINGS_RIGHT = 4,
	PATH_OPENINGS_TOP = 8,

	PATH_OPENINGS_ANY = 0x10
};

struct room_slot
{
	enum path_properties properties;
	enum path_openings openings;

	room_connectionmask connectionmask;
};

struct level
{
	tilemap_t tiles_background;
	tilemap_t tiles;
	tilemap_t tiles_foreground;

	rooms_t rooms;
	struct room_slot room_slots[16];

	int width;
	int height;
};

void level_clean(level_t level)
{
	for (int y = 0; y < level->height; y++)
	{
		for (int x = 0; x < level->width; x++)
		{
			tilemap_set_id_at(level->tiles_background, x, y, ROOM_TILE_NONE);
			tilemap_set_id_at(level->tiles, x, y, ROOM_TILE_NONE);
			tilemap_set_id_at(level->tiles_foreground, x, y, ROOM_TILE_NONE);
		}
	}

	for (int i = 0; i < 16; i++)
	{
		level->room_slots[i].openings = 0;
		level->room_slots[i].properties = 0;
	}
}

level_t level_create(rooms_t rooms, int width, int height)
{
	struct level* level = malloc(sizeof(struct level));

	level->width = width;
	level->height = height;

	level->rooms = rooms;
	level->tiles_background = tilemap_create(width, height);
	level->tiles = tilemap_create(width, height);
	level->tiles_foreground = tilemap_create(width, height);

	return level;
}

void level_destroy(level_t room)
{
	free(room);
}

//int get_random_room()
//{
//	int room_count = sizeof(room_database) / sizeof(room_database[0]);
//	
//	int index = rand() % room_count;
//
//	while(1)
//	{
//		struct room_desc* desc = &room_database[index];
//
//		int dice = rand() % 100000000 + 1;
//		float probability = (float)dice / 100000000.0f;
//
//		if(probability < desc->probability)
//			return index;
//
//		index = ++index & room_count;
//	}
//}

void level_generate_path(level_t level)
{
	int current_index = rand() % 16;
	level->room_slots[current_index].properties = PATH_PROPERTY_START;

	int step_table[] = { 1, -1, 4, -4,
						 1, -1, -4, 4,
						 1, 4, -4, -1,
						 1, 4, -1, -4,
						 1, -4, 4, -1,
						 1, -4, -1, 4 };

	enum room_openings step_mask_to[] = { PATH_OPENINGS_RIGHT, PATH_OPENINGS_LEFT, PATH_OPENINGS_BOTTOM, PATH_OPENINGS_TOP,
										  PATH_OPENINGS_RIGHT, PATH_OPENINGS_LEFT, PATH_OPENINGS_TOP, PATH_OPENINGS_BOTTOM,
										  PATH_OPENINGS_RIGHT, PATH_OPENINGS_BOTTOM, PATH_OPENINGS_TOP, PATH_OPENINGS_LEFT,
										  PATH_OPENINGS_RIGHT, PATH_OPENINGS_BOTTOM, PATH_OPENINGS_LEFT, PATH_OPENINGS_TOP,
										  PATH_OPENINGS_RIGHT, PATH_OPENINGS_TOP, PATH_OPENINGS_BOTTOM, PATH_OPENINGS_LEFT,
										  PATH_OPENINGS_RIGHT, PATH_OPENINGS_TOP, PATH_OPENINGS_LEFT, PATH_OPENINGS_BOTTOM };

	enum room_openings step_mask_from[] = { PATH_OPENINGS_LEFT, PATH_OPENINGS_RIGHT, PATH_OPENINGS_TOP, PATH_OPENINGS_BOTTOM,
										    PATH_OPENINGS_LEFT, PATH_OPENINGS_RIGHT, PATH_OPENINGS_BOTTOM, PATH_OPENINGS_TOP,
										    PATH_OPENINGS_LEFT, PATH_OPENINGS_TOP, PATH_OPENINGS_BOTTOM, PATH_OPENINGS_RIGHT,
										    PATH_OPENINGS_LEFT, PATH_OPENINGS_TOP, PATH_OPENINGS_RIGHT, PATH_OPENINGS_BOTTOM,
										    PATH_OPENINGS_LEFT, PATH_OPENINGS_BOTTOM, PATH_OPENINGS_TOP, PATH_OPENINGS_RIGHT,
										    PATH_OPENINGS_LEFT, PATH_OPENINGS_BOTTOM, PATH_OPENINGS_RIGHT, PATH_OPENINGS_TOP };

	int found_end = 0;
	int room_steps = 7;

	while (rand() % 2 == 0 && room_steps < 14)
		room_steps++;

	for (int i = 0; i < room_steps && !found_end; i++)
	{
		level->room_slots[current_index].properties |= PATH_PROPERTY_ON_PATH;

		int step = rand() % 24;
		int tries = 0;

		int valid_step = 0;

		while (tries < 4)
		{
			int step_dir = step_table[step];
			int try_room = current_index + step_dir;

			int current_y = current_index / 4;
			int next_y = try_room / 4;

			if (try_room >= 0 && try_room < 16 && 
				!(level->room_slots[try_room].properties & PATH_PROPERTY_ON_PATH) &&
				!(step_dir == -1 && current_y != next_y) &&
				!(step_dir == 1 && current_y != next_y))
			{
				level->room_slots[current_index].openings |= step_mask_to[step];
				level->room_slots[try_room].openings |= step_mask_from[step];

				current_index = try_room;
				valid_step = 1;
				break;
			}

			step = ++step % 24;
			tries++;
		}

		if (!valid_step)
			found_end = 1;
	}

	level->room_slots[current_index].properties |= PATH_PROPERTY_END;
	level->room_slots[current_index].properties |= PATH_PROPERTY_ON_PATH;
}

void spawn_box(level_t level, int box_width, int box_height, int x, int y)
{
	for (int sy = y; sy < y + box_height; sy++)
	{
		for (int sx = x; sx < x + box_width; sx++)
		{
			level_place_wall(level, sx, sy);
		}
	}
}

void carve_box(level_t level, int box_width, int box_height, int x, int y)
{
	for (int sy = y; sy < y + box_height; sy++)
	{
		for (int sx = x; sx < x + box_width; sx++)
		{
			level_place_floor(level, sx, sy);
		}
	}
}

room_connectionmask level_get_connectionmask(level_t level, int x, int y)
{
	room_connectionmask mask = 0;

	if (x - 1 >= 0)
	{
		int index_left = y * 4 + x - 1;
		struct room_slot* slot_left = &level->room_slots[index_left];

		room_connectionmask mask_left = slot_left->connectionmask & MASK_RIGHT;
		mask_left = (mask_left << 8) & MASK_LEFT;
		mask |= mask_left;
	}

	if (x + 1 < 4)
	{
		int index_right = y * 4 + x + 1;
		struct room_slot* slot_left = &level->room_slots[index_right];

		room_connectionmask mask_right = slot_left->connectionmask & MASK_LEFT;
		mask_right = (mask_right >> 8) & MASK_RIGHT;
		mask |= mask_right;
	}

	if (y - 1 >= 0)
	{
		int index_top = (y - 1) * 4 + x;
		struct room_slot* slot_left = &level->room_slots[index_top];

		room_connectionmask mask_top = slot_left->connectionmask & MASK_BOTTOM;
		mask_top = (mask_top << 24) & MASK_TOP;
		mask |= mask_top;
	}

	if (y + 1 < 4)
	{
		int index_bottom = (y + 1) * 4 + x;
		struct room_slot* slot_left = &level->room_slots[index_bottom];

		room_connectionmask mask_bottom = slot_left->connectionmask & MASK_TOP;
		mask_bottom = (mask_bottom >> 24) & MASK_BOTTOM;
		mask |= mask_bottom;
	}

	return mask;
}

void level_set_initial_connectionmasks(level_t level)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			struct room_slot* room_slot = &level->room_slots[y * 4 + x];
			room_slot->connectionmask = 0;

			if (room_slot->openings & PATH_OPENINGS_TOP)
				room_slot->connectionmask |= MASK_TOP;

			if (room_slot->openings & PATH_OPENINGS_LEFT)
				room_slot->connectionmask |= MASK_LEFT;

			if (room_slot->openings & PATH_OPENINGS_RIGHT)
				room_slot->connectionmask |= MASK_RIGHT;

			if (room_slot->openings & PATH_OPENINGS_BOTTOM)
				room_slot->connectionmask |= MASK_BOTTOM;
		}
	}
}

void level_generate_rooms(level_t level)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			struct room_slot* room_slot = &level->room_slots[y*4 + x];
			room_slot->connectionmask = level_get_connectionmask(level, x, y);

			struct room* room = rooms_get_random(level->rooms, room_slot->connectionmask, 0);

			if (room/* && room_slot->properties & PATH_PROPERTY_ON_PATH*/)
				level_place_room(level, room, x * ROOM_SIZE, y * ROOM_SIZE);
		}
	}
}

void level_generate(level_t level, int seed)
{
	srand(seed);

	level_clean(level);
	//spawn_box(level, 32, 32, 0, 0);

	level_generate_path(level);
	level_set_initial_connectionmasks(level);
	level_generate_rooms(level);

	//for (int y = 0; y < 4; y++)
	//{
	//	for (int x = 0; x < 4; x++)
	//	{
	//		if (level->room_slots[4 * y + x].properties & PATH_PROPERTY_ON_PATH)
	//		{
	//			carve_box(level, 7, 7, x*8 + 1, y*8 + 1);
	//		}
	//		else
	//		{
	//			carve_box(level, 3, 3, x * 8 + 3, y * 8 + 3);
	//		}
	//	}
	//}
}

enum level_tile_mask tile_get_mask(tilemap_t tilemap, int x, int y, int type)
{
	if (type == ROOM_TILE_NONE)
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

	if (id != ROOM_TILE_NONE)
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
	if (type != ROOM_TILE_NONE)
	{
		enum level_tile_mask mask = tile_get_mask(tilemap, x, y, type);

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
	return tilemap_get_id_at(tilemap, x, y) & LEVEL_TILE_TYPEMASK;
}

void level_place_wall(level_t level, int x, int y)
{
	tile_place(level->tiles_background, x, y, ROOM_TILE_FLOOR);
	tile_place(level->tiles, x, y, ROOM_TILE_WALL);
	tile_place(level->tiles_foreground, x, y - 1, ROOM_TILE_WALL_TOP);
}

void level_place_floor(level_t level, int x, int y)
{
	tile_place(level->tiles_background, x, y, ROOM_TILE_FLOOR);
	tile_place(level->tiles, x, y, ROOM_TILE_FLOOR);
	tile_place(level->tiles_foreground, x, y - 1, ROOM_TILE_NONE);
}

void level_place_room(level_t level, struct room* room, int x, int y)
{
	for (int cy = 0; cy < ROOM_SIZE; cy++)
	{
		for (int cx = 0; cx < ROOM_SIZE; cx++)
		{
			int tile = tilemap_get_id_at(room->tiles, cx, cy);

			switch (tile)
			{
			case ROOM_TILE_FLOOR:
				level_place_floor(level, cx + x, cy + y);
				break;
			case ROOM_TILE_WALL:
				level_place_wall(level, cx + x, cy + y);
				break;
			case ROOM_TILE_OPENING:
				level_place_floor(level, cx + x, cy + y);
				break;
			default:
				break;
			}
		}
	}
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
	
	if (id == ROOM_TILE_WALL)
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
