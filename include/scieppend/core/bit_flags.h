#ifndef SCIEPPEND_CORE_BIT_FLAGS_H
#define SCIEPPEND_CORE_BIT_FLAGS_H

#define BIT_FLAG(x) (1u << x)

#define bit_flags_set_flags(field, flags) (field |= flags)
#define bit_flags_unset_flags(field, flags) (field &= ~flags)
#define bit_flags_has_flags(field, flags) ((field & flags) == flags)

#endif
