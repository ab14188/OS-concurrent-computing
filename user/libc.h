#ifndef __LIBC_H
#define __LIBC_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>	

int write( int fd, void* x, size_t n );
int read( void* buffer );
int fork();
void exit();
void write_numb( int numb );
void numb_to_char( int numb );
int get_info();
int exec( int pid );
void P0();
void P1();
void P2();
void walking();	

#endif
