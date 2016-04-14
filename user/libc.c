#include "libc.h"

// Write to the terminal
int write( int fd, void* x, size_t n ) {
  int r;
  asm volatile( "mov r0, %1 \n"
                "mov r1, %2 \n"
                "mov r2, %3 \n"
                "svc #0     \n"
                "mov %0, r0 \n" 
              : "=r" (r) 
              : "r" (fd), "r" (x), "r" (n) 
              : "r0", "r1", "r2" );
  return r;
}

// Read from user 
int read(void *buffer){
  int r;
  asm volatile( "mov r0, %1 \n"
                "svc #1     \n"
                "mov %0, r0 \n"
                : "=r" (r)
                : "r" (buffer)
                : "r0");
  return r;
}

// Fork parent Process: 
int fork(){
  int r;
  asm volatile( "svc #2     \n"
                "mov %0, r0 \n"
                : "=r" (r));
  return r;
}

// Exit current process
void exit(){
  asm volatile( "svc #3     \n");
}

// Execute process
int exec( int pid ){
  int r;
  asm volatile( "mov r0, %1 \n"
                "svc #4     \n"
                "mov %0, r0 \n"
                : "=r" (r)
                : "r" (pid)
                : "r0");
  return r;
}

// Get general information
int get_info(){
  int r;
  asm volatile( "svc #5     \n"
                "mov %0, r0 \n"
                : "=r" (r));
  return r;
}

// Create a channel
int create_channel( int chan_start, int chan_end) {
  int r;
  asm volatile( "mov r0, %1 \n"
                "mov r1, %2 \n"
                "svc #6     \n"
                "mov %0, r0 \n" 
              : "=r" (r) 
              : "r" (chan_start), "r" (chan_end) 
              : "r0", "r1");
  return r;
}

void* read_channel( int channel_id, int chan ){
  void* r;
  asm volatile( "mov r0, %1 \n"
                "mov r1, %2 \n"
                "svc #8     \n"
                "mov %0, r0 \n" 
              : "=r" (r) 
              : "r" (channel_id), "r" (chan) 
              : "r0", "r1");
  return r;
}

void* write_channel( int channel_id, int chan ){
  void* r;
  asm volatile( "mov r0, %1 \n"
                "mov r1, %2 \n"
                "svc #9     \n"
                "mov %0, r0 \n" 
              : "=r" (r) 
              : "r" (channel_id), "r" (chan) 
              : "r0", "r1");
  return r;
}


int yield( int pid ){
  int r;
  asm volatile( "mov r0, %1 \n"
                "svc #7     \n"
                "mov %0, r0 \n"
                : "=r" (r)
                : "r" (pid)
                : "r0");
  return r;
}


void write_numb( int numb ) {
	int size_numb = 0, reverse = 0, digit   = 0;

  	if (numb == 0) size_numb = 1;
  	while (numb != 0){
      	reverse = reverse * 10;
      	reverse = reverse + numb%10;
      	numb    /=10;
      	size_numb += 1;
  	}
  	for (int i = 0; i < size_numb; i++){
    	digit      = reverse % 10;
    	reverse   /= 10;
    	numb_to_char(digit);
  	}
}

void numb_to_char( int numb ) { 
  char* x = "0";
  switch (numb) {
    case 0 : x = "0"; break;
    case 1 : x = "1"; break;
    case 2 : x = "2"; break;
    case 3 : x = "3"; break;
    case 4 : x = "4"; break;
    case 5 : x = "5"; break;
    case 6 : x = "6"; break;
    case 7 : x = "7"; break;
    case 8 : x = "8"; break;
    case 9 : x = "9"; break;
    default: x = " "; break;
  }

  write (0, x, 1);
}