#include "scieppend/core/entity.h"

#define DEFAULT_ENTITY_COMPONENTS_MAX 8

const int C_NULL_ENTITY_HANDLE = 0xffffffff;

// ---------- INTERNAL FUNCS ----------

static int _compare_component_lookup_by_type(const void* lhs, const void* rhs)
{
    const struct ComponentLookup* _lhs = lhs;
    const ComponentTypeHandle* _rhs = rhs;
    return _lhs->component_type_handle == *_rhs;
}

// ---------- EXTERNAL FUNCS ----------

void entity_init(struct Entity* entity)
{
    array_ts_init(&entity->components, sizeof(struct ComponentLookup), DEFAULT_ENTITY_COMPONENTS_MAX, NULL, NULL);
}

void entity_uninit(struct Entity* entity)
{
    array_ts_uninit(&entity->components);
}

void entity_init_wrapper(void* entity, [[maybe_unused]] const void* args)
{
    entity_init((struct Entity*)entity);
}

void entity_uninit_wrapper(void* entity)
{
    entity_uninit((struct Entity*)entity);
}

int entity_components_count(const struct Entity* entity)
{
    return array_ts_count(&entity->components);
}

bool entity_has_component(const struct Entity* entity, const ComponentTypeHandle component_type_handle)
{
    return array_ts_find(&entity->components, &component_type_handle, &_compare_component_lookup_by_type) != C_NULL_COMPONENT_HANDLE;
}

bool entity_has_components(const struct Entity* entity, const struct Array* component_type_handles)
{
    int has_count = 0;

    array_ts_lock(&entity->components, READ);

    for(int i = 0; i < array_count(&entity->components.array); ++i)
    {
        struct ComponentLookup* lookup = array_get(&entity->components.array, i);

        for(int j = 0; j < array_count(component_type_handles); ++j)
        {
            if(_compare_component_lookup_by_type(lookup, array_get(component_type_handles, j)))
            {
                ++has_count;
            }
        }
    }

    array_ts_unlock(&entity->components, READ);

    return has_count == array_count(component_type_handles);
}

ComponentHandle entity_get_component(const struct Entity* entity, const ComponentTypeHandle component_type_handle)
{
    ComponentHandle handle = C_NULL_COMPONENT_HANDLE;

    array_ts_lock(&entity->components, READ);

    int lu_handle = array_find(&entity->components.array, &component_type_handle, &_compare_component_lookup_by_type);
    if (lu_handle != - 1)
    {
        struct ComponentLookup* lookup = array_get(&entity->components.array, lu_handle);
        handle = lookup->component_handle;
    }

    array_ts_unlock(&entity->components, READ);

    return handle;
}

const struct Array* entity_get_components(const struct Entity* entity)
{
    return &entity->components.array;
}

void entity_add_component(struct Entity* entity, const ComponentHandle component_handle, const ComponentTypeHandle component_type_handle)
{
    struct ComponentLookup lookup;
    lookup.component_handle = component_handle;
    lookup.component_type_handle = component_type_handle;
    array_ts_add(&entity->components, &lookup);
}

void entity_remove_component(struct Entity* entity, const ComponentTypeHandle component_type_handle)
{
    array_ts_find_and_remove(&entity->components, &component_type_handle, &_compare_component_lookup_by_type);
}

bool entity_lock(struct Entity* entity, bool write)
{
    return array_ts_lock(&entity->components, write);
}

void entity_unlock(struct Entity* entity, bool write)
{
    array_ts_unlock(&entity->components, write);
}
