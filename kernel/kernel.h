#ifndef __KERNEL_H
#define __KERNEL_H

#include <stddef.h>
#include <stdint.h>

#include   "GIC.h"
#include "PL011.h"
#include "SP804.h"

#include "interrupt.h"

// Include functionality from newlib, the embedded standard C library.
#include <string.h>	
#include <stdlib.h>

//Include definitions relating to the 3 user Programs and the user terminal
#include "P0.h"
#include "P1.h"
#include "P2.h"
#include "walking.h"
#include "DiningPhilosophers.h"
#include "terminal.h"

typedef struct {
  uint32_t cpsr, pc, gpr[ 13 ], sp, lr;
} ctx_t;

typedef int pid_t;

typedef struct {
  pid_t pid;
  int priority;
  ctx_t ctx;
} pcb_t;


#endif