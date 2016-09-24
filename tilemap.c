#include "tilemap.h"

struct tile
{
	tilemap_t parent;

	int x;
	int y;

	int id;
};

struct tilemap
{
	struct tile* tiles;

	int width;
	int height;
};

tilemap_t tilemap_create(int width, int height)
{
	struct tilemap* tilemap = malloc(sizeof(struct tilemap));
	tilemap->tiles = malloc(width * height * sizeof(struct tile));
	tilemap->width = width;
	tilemap->height = height;

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			tilemap->tiles[y*width + x].parent = tilemap;
			tilemap->tiles[y*width + x].x = x;
			tilemap->tiles[y*width + x].y = y;
			tilemap->tiles[y*width + x].id = -1;
		}

	return tilemap;
}

void tilemap_destroy(tilemap_t tilemap)
{
	free(tilemap->tiles);
	free(tilemap);
}

void tilemap_get_dimensions(tilemap_t tilemap, int* width, int* height)
{
	*width = tilemap->width;
	*height = tilemap->height;
}

void tilemap_add_child(tilemap_t tilemap, int x, int y, tilemap_t child)
{
	int child_width, child_height;
	tilemap_get_dimensions(child, &child_width, &child_height);

	int ex = x + child_width;
	int ey = y + child_height;

	for (int cy = y; cy >= 0 && cy < ey; cy++)
	{
		for (int cx = x; cx >= 0 && cx < ex; cx++)
		{
			int child_x = cx - x;
			int child_y = cy - y;

			if (tilemap_get_id_at(child, child_x, child_y) != -1)
			{
				struct tile* tile = &tilemap->tiles[tilemap->width*cy + cx];
				tile->parent = child;
				tile->x = child_x;
				tile->y = child_y;
				tile->id = -1;
			}
		}
	}
}

void tilemap_set_id_at(tilemap_t tilemap, int x, int y, int id)
{
	if (x < 0 || x >= tilemap->width || y < 0 || y >= tilemap->height)
		return;

	tilemap->tiles[tilemap->width*y + x].id = id;
}

int tilemap_get_id_at(tilemap_t tilemap, int x, int y)
{
	if (x < 0 || x >= tilemap->width || y < 0 || y >= tilemap->height)
		return -1;

	struct tile* tile = &tilemap->tiles[tilemap->width*y + x];

	if (tile->parent == tilemap)
		return tile->id;
	else
	{
		return tilemap_get_id_at(tile->parent, tile->x, tile->y);
	}
}
