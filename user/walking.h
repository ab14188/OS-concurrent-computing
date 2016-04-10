#ifndef __WALKING_H
#define __WALKING_H

#include <stddef.h>
#include <stdint.h>

#include "libc.h"

extern void (*entry_walking)(); 
extern uint32_t tos_walking;

void walking();

#endif
