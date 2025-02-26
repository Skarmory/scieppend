#ifndef SCIEPPEND_CORE_COMPONENT_H
#define SCIEPPEND_CORE_COMPONENT_H

/* Macros to help with defining component types.
 * TODO: Need to think of a better idea than these extern'd values. Compile time hashing?
 */

#include "scieppend/core/ecs_defs.h"
#include "scieppend/core/string.h"

#define COMPONENT_TYPE_ID(type_name) SCIEPPEND_COMPONENT_TYPE_ID__##type_name

#define COMPONENT_TYPE_NAME(type_name) SCIEPPEND_COMPONENT_TYPE_NAME__##type_name

#define COMPONENT_TYPE_DECL(type_name)\
    extern COMPONENT_TYPE_HANDLE COMPONENT_TYPE_ID(type_name);\
    extern struct string* COMPONENT_TYPE_NAME(type_name);\
    struct type_name\

#define COMPONENT_TYPE_DEF(type_name)\
    COMPONENT_TYPE_HANDLE COMPONENT_TYPE_ID(type_name) = 0;\
    struct string COMPONENT_TYPE_NAME(type_name);\

#define COMPONENT_INIT(type_name)\
    do {\
        COMPONENT_TYPE_NAME(type_name) = string_init(#type_name);\
        COMPONENT_TYPE_ID(type_name) = string_hash(COMPONENT_TYPE_NAME(type_name);\
    } while(0)

#endif
