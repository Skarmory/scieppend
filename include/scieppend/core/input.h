#ifndef SCIEPPEND_CORE_INPUT_H
#define SCIEPPEND_CORE_INPUT_H

#include "scieppend/core/input_keycodes.h"

#include <stdbool.h>

bool input_get_key(enum KeyCode key);
void input_poll(void);
void input_init(void);
void input_uninit(void);

#endif

