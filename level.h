#ifndef _LEVEL_H
#define _LEVEL_H

#include "rooms.h"

typedef struct level* level_t;

enum level_tile_mask
{
	LEVEL_TILE_BOTTOM	= 1,
	LEVEL_TILE_LEFT		= 2,
	LEVEL_TILE_RIGHT	= 4,
	LEVEL_TILE_TOP		= 8,

	LEVEL_TILE_TYPEMASK	= 255,
};

#define TILE_TYPE_SHIFT	8
#define LEVEL_TILE_SIZE	16.0f

level_t		level_create(int width, int height);
void		level_destroy(level_t level);
void		level_generate(level_t level, int seed);

void		level_place_wall(level_t level, int x, int y);
void		level_place_floor(level_t level, int x, int y);

int			level_walkable_at(level_t level, float x, float y);

tilemap_t	level_get_tiles_background(level_t level);
tilemap_t	level_get_tiles(level_t level);
tilemap_t	level_get_tiles_foreground(level_t level);

void		level_get_tile_dimensions(level_t level, int* width, int* height);
void		level_get_dimensions(level_t level, float* width, float* height);

#endif
