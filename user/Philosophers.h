#ifndef __PHILOSOPHERS_H
#define __PHILOSOPHERS_H

#include <stddef.h>
#include <stdint.h>

#include "libc.h"

extern void (*entry_philosophers)(); 
extern uint32_t tos_philosophers;

void philo_run();

#endif
