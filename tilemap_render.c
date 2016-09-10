#include "tilemap_render.h"

#include "animatedsprites.h"
#include "atlas.h"

struct tilemap_render
{
	tilemap_t				tilemap;
	struct animatedsprites* animatedsprites;
	struct sprite*			sprites;
	get_anim_for_tile_t		get_anim_fn;

	float					tile_size;
};

tilemap_render_t tilemap_render_create(tilemap_t tilemap, get_anim_for_tile_t get_anim_fn, float tile_size)
{
	struct tilemap_render* tilemap_render = malloc(sizeof(struct tilemap_render));

	tilemap_render->animatedsprites = animatedsprites_create();
	tilemap_render->tile_size = tile_size;
	tilemap_render->get_anim_fn = get_anim_fn;
	tilemap_render->tilemap = tilemap;

	int map_width, map_height;
	tilemap_get_dimensions(tilemap, &map_width, &map_height);

	tilemap_render->sprites = (struct sprite *) calloc(map_width * map_height, sizeof(struct sprite));

	for (int y = 0; y < map_height; y++)
	{
		for (int x = 0; x < map_width; x++)
		{
			struct sprite* sprite = &tilemap_render->sprites[map_width*y + x];

			set3f(sprite->position, x * tile_size, -y * tile_size, 0.0f);
			set2f(sprite->scale, 1.0f, 1.0f);

			int id = tilemap_get_id_at(tilemap_render->tilemap, x, y);
			struct anim* anim = tilemap_render->get_anim_fn(id);

			animatedsprites_add(tilemap_render->animatedsprites, sprite);
			animatedsprites_playanimation(sprite, anim);
		}
	}

	return tilemap_render;
}

void tilemap_render_destroy(tilemap_render_t tilemap)
{
	free(tilemap);
}

/* TODO: Pass view to update/render to only update and render tiles on screen */

void tilemap_render_update(tilemap_render_t tilemap_render, struct atlas* atlas, float dt)
{
	animatedsprites_update(tilemap_render->animatedsprites, atlas, dt);
}

void tilemap_render_render(tilemap_render_t tilemap_render, struct shader* s, struct graphics* g, GLuint tex, mat4 transform)
{
	animatedsprites_render(tilemap_render->animatedsprites, s, g, tex, transform);
}
