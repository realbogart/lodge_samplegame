#include "entities.h"

#include <stdlib.h>

#define ENTITIES_MAX_TYPES	256

struct entity_type
{
	int lol;
};

struct entities
{
	struct entity_type types[ENTITIES_MAX_TYPES];

	int types_count;
};

entities_t entities_create()
{
	struct entities* entities = malloc(sizeof(struct entities));
	entities->types_count = 0;

	return entities;
}

void entities_destroy(entities_t entities)
{

}

entity_type_t entities_register_type()
{

}
