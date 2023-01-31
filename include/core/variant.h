#ifndef SCIEPPEND_CORE_VARIANT_H
#define SCIEPPEND_CORE_VARIANT_H

/* A variant contains dynamically typed data.
 */

enum DataType
{
    DATA_TYPE_INT,
    DATA_TYPE_UINT,
    DATA_TYPE_CHAR,
    DATA_TYPE_PTR,
    DATA_TYPE_STRING
};

struct Variant
{
    enum DataType type;
    union
    {
        int          as_int;
        unsigned int as_uint;
        char         as_char;
        void*        as_ptr;
        char*        as_str;
    } data;
};

#endif
