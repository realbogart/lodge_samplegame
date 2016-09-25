#ifndef _ENTITIES_H
#define _ENTITIES_H

typedef struct entities* entities_t;
typedef struct entity_type* entity_type_t;

struct entity_type_desc
{
	int lol;
};


entities_t		entities_create();
void			entities_destroy(entities_t entities);

entity_type_t	entities_register_type();

#endif
