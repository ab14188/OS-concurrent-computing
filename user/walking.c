#include "walking.h"

void walking() {

  for (int i = 0; i < 64; i++){
    write_numb(i);
    write(0, " kilomètre à pied ça use, ça use, \n", 40);
    write_numb(i);
    write(0, " km à pied ça use les souliers. \n \n", 36  );
  }
  return;
}

void (*entry_walking)() = &walking;