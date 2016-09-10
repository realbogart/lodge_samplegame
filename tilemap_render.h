#ifndef _TILEMAP_RENDER_H
#define _TILEMAP_RENDER_H

#include "tilemap.h"
#include "math4.h"
#include "shader.h"
#include "graphics.h"

typedef struct tilemap_render* tilemap_render_t;
typedef struct anim* (*get_anim_for_tile_t)(int id);

tilemap_render_t	tilemap_render_create(tilemap_t tilemap, get_anim_for_tile_t get_anim_fn, float tile_size);
void				tilemap_render_destroy(tilemap_render_t tilemap_render);

void				tilemap_render_update(tilemap_render_t tilemap_render, struct atlas* atlas, float dt);
void				tilemap_render_render(tilemap_render_t tilemap_render, struct shader* s, struct graphics* g, GLuint tex, mat4 transform);

#endif
