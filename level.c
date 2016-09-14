#include "level.h"

#include <math.h>

enum room_openings
{
	ROOM_OPENINGS_NONE		= 0,
	ROOM_OPENINGS_BOTTOM	= 1,
	ROOM_OPENINGS_LEFT		= 2,
	ROOM_OPENINGS_RIGHT		= 4,
	ROOM_OPENINGS_TOP		= 8,

	ROOM_OPENINGS_ANY		= 0x10,
	ROOM_OPENINGS_NOT_NONE	= 0x11
};

enum room_types
{
	ROOM_TYPES_ON_PATH = 1,
	ROOM_TYPES_START = 2,
	ROOM_TYPES_END = 4,
};

struct room_desc
{
	char tiles[64];

	float probability;
};

struct room_desc room_database[] =
{
	{
		"        "\
		" .      "\
		"  .     "\
		"        "\
		"  .     "\
		" .      "\
		".       "\
		"        ", 0.5f
	},
	{
		"....    "\
		"        "\
		"        "\
		"   ...  "\
		"        "\
		"        "\
		"...     "\
		"        ", 0.1f
	}
};

struct room
{
	tilemap_t tiles;

	enum room_openings openings;
	enum room_types types;
};

struct level
{
	tilemap_t tiles_background;
	tilemap_t tiles;
	tilemap_t tiles_foreground;

	struct room	rooms[16];

	int width;
	int height;
};

void level_clean(level_t level)
{
	for (int y = 0; y >= 0 && y < level->height; y++)
	{
		for (int x = 0; x >= 0 && x < level->width; x++)
		{
			tilemap_set_id_at(level->tiles_background, x, y, LEVEL_TILE_NONE);
			tilemap_set_id_at(level->tiles, x, y, LEVEL_TILE_NONE);
			tilemap_set_id_at(level->tiles_foreground, x, y, LEVEL_TILE_NONE);
		}
	}

	for (int i = 0; i < 16; i++)
	{
		level->rooms[i].openings = 0;
		level->rooms[i].types = 0;
	}
}

level_t level_create(int width, int height)
{
	struct level* level = malloc(sizeof(struct level));

	level->width = width;
	level->height = height;

	level->tiles_background = tilemap_create(width, height);
	level->tiles = tilemap_create(width, height);
	level->tiles_foreground = tilemap_create(width, height);

	level_clean(level);

	return level;
}

void level_destroy(level_t room)
{
	free(room);
}

int get_random_room()
{
	int room_count = sizeof(room_database) / sizeof(room_database[0]);
	
	int index = rand() % room_count;

	while(1)
	{
		struct room_desc* desc = &room_database[index];

		int dice = rand() % 100000000 + 1;
		float probability = (float)dice / 100000000.0f;

		if(probability < desc->probability)
			return index;

		index = ++index & room_count;
	}
}

void level_generate_path(level_t level)
{
	int current_index = rand() % 16;
	level->rooms[current_index].types = ROOM_TYPES_START;

	int step_table[] = { 1, -1, 4, -4,
						 1, -1, -4, 4,
						 1, 4, -4, -1,
						 1, 4, -1, -4,
						 1, -4, 4, -1,
						 1, -4, -1, 4 };

	enum room_openings step_mask_to[] = { ROOM_OPENINGS_RIGHT, ROOM_OPENINGS_LEFT, ROOM_OPENINGS_BOTTOM, ROOM_OPENINGS_TOP,
										  ROOM_OPENINGS_RIGHT, ROOM_OPENINGS_LEFT, ROOM_OPENINGS_TOP, ROOM_OPENINGS_BOTTOM,
										  ROOM_OPENINGS_RIGHT, ROOM_OPENINGS_BOTTOM, ROOM_OPENINGS_TOP, ROOM_OPENINGS_LEFT,
										  ROOM_OPENINGS_RIGHT, ROOM_OPENINGS_BOTTOM, ROOM_OPENINGS_LEFT, ROOM_OPENINGS_TOP,
										  ROOM_OPENINGS_RIGHT, ROOM_OPENINGS_TOP, ROOM_OPENINGS_BOTTOM, ROOM_OPENINGS_LEFT,
										  ROOM_OPENINGS_RIGHT, ROOM_OPENINGS_TOP, ROOM_OPENINGS_LEFT, ROOM_OPENINGS_BOTTOM };

	enum room_openings step_mask_from[] = { ROOM_OPENINGS_LEFT, ROOM_OPENINGS_RIGHT, ROOM_OPENINGS_TOP, ROOM_OPENINGS_BOTTOM,
										    ROOM_OPENINGS_LEFT, ROOM_OPENINGS_RIGHT, ROOM_OPENINGS_BOTTOM, ROOM_OPENINGS_TOP,
										    ROOM_OPENINGS_LEFT, ROOM_OPENINGS_TOP, ROOM_OPENINGS_BOTTOM, ROOM_OPENINGS_RIGHT,
										    ROOM_OPENINGS_LEFT, ROOM_OPENINGS_TOP, ROOM_OPENINGS_RIGHT, ROOM_OPENINGS_BOTTOM,
										    ROOM_OPENINGS_LEFT, ROOM_OPENINGS_BOTTOM, ROOM_OPENINGS_TOP, ROOM_OPENINGS_RIGHT,
										    ROOM_OPENINGS_LEFT, ROOM_OPENINGS_BOTTOM, ROOM_OPENINGS_RIGHT, ROOM_OPENINGS_TOP };

	int found_end = 0;
	int room_steps = 8;

	for (int i = 0; i < room_steps && !found_end; i++)
	{
		level->rooms[current_index].types |= ROOM_TYPES_ON_PATH;

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
				!(level->rooms[try_room].types & ROOM_TYPES_ON_PATH) &&
				!(step_dir == -1 && current_y != next_y) &&
				!(step_dir == 1 && current_y != next_y))
			{
				level->rooms[current_index].openings |= step_mask_to[step];
				level->rooms[try_room].openings |= step_mask_from[step];

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

	level->rooms[current_index].types |= ROOM_TYPES_END;
	level->rooms[current_index].types |= ROOM_TYPES_ON_PATH;
}

void level_generate(level_t level, int seed)
{
	srand(seed);

	level_generate_path(level);
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
