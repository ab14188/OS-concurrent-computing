#include "terminal.h"

int extractCommand( char* command, char* buffer ){
  while (*buffer != '\r') {
    if (*buffer != *command) return -1;
    buffer++, command++;
  }
  return 1;
}

void execute( char* buffer ){
  if ( extractCommand("run p0\r", buffer) == 1 ){ // problem is that i has not been stored for after friiick
    int i     = fork();
    if ( i == 0) { 
      P0();
      exit();
    } 
    else {
      write(0, "run p1\n", 7);
    } 
  }
  else if ( extractCommand("run p1\r", buffer) == 1 ){
    write(0, "run p1\n", 7);
  }
  else if ( extractCommand("run p2\r", buffer) == 1 ){
    write(0, "run p2\n", 7);
  }
  else write(0, "ERROR -- command does not exist\n", 32);
  return;
}


void terminal() {
  char buffer[ 101 ]; 
  
  while (1){
    //writeNumb(i);
    write( 0, "terminal$ ", 10);
    read( buffer );
    write (0, "\n", 1);
    execute( buffer );
  }
}

void (*entry_terminal)() = &terminal;