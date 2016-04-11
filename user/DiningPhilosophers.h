#ifndef __DININGPHILOSOPHERS_H
#define __DININGPHILOSOPHERS_H

#include <stddef.h>
#include <stdint.h>

#include "libc.h"

extern void (*entry_philosophers)(); 
extern uint32_t tos_philosophers;

void dining_Philosophers();

#endif
