#ifndef SCIEPPEND_CORE_COMPONENT_H
#define SCIEPPEND_CORE_COMPONENT_H

/* Macros to help with defining component types.
 * TODO: Need to think of a better idea than these extern'd values. Compile time hashing?
 */

#include "scieppend/core/ecs_defs.h"
#include "scieppend/core/string.h"

#define NULL_COMPONENT_TYPE_PREHASH_MACRO 2025596145

#define COMPONENT_TYPE_ID(type_name) SCIEPPEND_COMPONENT_TYPE_ID__##type_name

#define COMPONENT_TYPE_NAME(type_name) SCIEPPEND_COMPONENT_TYPE_NAME__##type_name

#define COMPONENT_TYPE_DECL(type_name)\
    extern ComponentTypeHandle COMPONENT_TYPE_ID(type_name);\
    extern struct string* COMPONENT_TYPE_NAME(type_name);\
    struct type_name

#define COMPONENT_TYPE_DEF(type_name)\
    ComponentTypeHandle COMPONENT_TYPE_ID(type_name) = NULL_COMPONENT_TYPE_PREHASH_MACRO;\
    struct string* COMPONENT_TYPE_NAME(type_name) = NULL

#define COMPONENT_TYPE_INIT(type_name)\
    do {\
        COMPONENT_TYPE_NAME(type_name) = string_new(#type_name);\
        COMPONENT_TYPE_ID(type_name) = string_hash(COMPONENT_TYPE_NAME(type_name));\
    } while(0)


#define COMPONENT_TYPE_UNINIT(type_name)\
    do{\
        string_free(COMPONENT_TYPE_NAME(type_name));\
    } while(0)

#endif
