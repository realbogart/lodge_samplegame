#ifndef _TILEMAP_H
#define _TILEMAP_H

typedef struct tilemap* tilemap_t;

tilemap_t		tilemap_create(int width, int height);
void			tilemap_destroy(tilemap_t tilemap);

void			tilemap_set_id_at(tilemap_t tilemap, int x, int y, int id);
int				tilemap_get_id_at(tilemap_t tilemap, int x, int y);

void			tilemap_get_dimensions(tilemap_t tilemap, int* width, int* height);
void			tilemap_add_child(tilemap_t tilemap, int x, int y, tilemap_t child);

#endif
