#ifndef __LIBC_H
#define __LIBC_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>	

int write( int fd, void* x, size_t n );
int read( void* buffer );
int fork();
void exit();
void writeNumb( int numb );
void numbToChar( int numb );

#endif
