#include "walk.h"

void walk() {

  for (int i = 0; i < 3000; i++){
    write_numb(i);
    write(0, " km à pied ça use, ça use, \n", 32);
    write_numb(i);
    write(0, " km à pied ça use les souliers. \n \n", 36  );
  }
  return;
}

void (*entry_walk)() = &walk;

/* This file has been created to demonstrate an "exitable" process -- ie one that does not go on forever*/