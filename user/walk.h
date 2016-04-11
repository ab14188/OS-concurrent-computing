#ifndef __WALK_H
#define __WALK_H

#include <stddef.h>
#include <stdint.h>

#include "libc.h"

extern void (*entry_walk)(); 
extern uint32_t tos_walk;

void walk();

#endif
