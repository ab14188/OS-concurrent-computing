#ifndef __LIBC_H
#define __LIBC_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>	

// Passing ctrl between different processes 
int fork();
int exec( int pid );
int yield( int pid );
void exit();

// Writing and reading things
int read( void* buffer );
int write( int fd, void* x, size_t n );
void write_numb( int numb );
void numb_to_char( int numb );

//Channel related operations
int read_channel( int channel_id, int lr );
int write_channel( int channel_id, int chopstick, int taken );
int create_channel();

// Info on different pcbs and sleeping for a pcb 
int get_info();
int sleep( int timer_id, uint32_t sleep_time );

// This is used to make my life easier + no warnings 
void P0();
void P1();
void P2();
void walk();
void Table();	

#endif
