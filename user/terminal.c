#include "terminal.h"

int extract_command( char* command, char* buffer ){
  if (buffer[0] == '\r') return -1;
  
  while (*buffer != '\r' && *command != '\r') {
    if (*buffer != *command) return -1;
    buffer++, command++;
  }
  
  return 1;
}

// Execute the command entered by the user 
void execute( char* buffer ){

  if ( extract_command("run p0\r", buffer) == 1 ){ 
    int pid     = fork();    
    if ( pid == 0 ) { // Case in Child process
      P0();
      exit();
    }
    else if ( pid > 0 ){ // Case in Parent process
      exec( pid );  
    } 
    else 
      return;
  }
  else if ( extract_command("run p1\r", buffer) == 1 ){
    int pid     = fork();

    if ( pid == 0 ) { // Case in Child process
      P1();
      exit();
    }
    else if ( pid > 0 ){ // Case in Parent process
      exec( pid );  
    } 
    else 
      return;
  }
  else if ( extract_command("run p2\r", buffer) == 1 ){
    int pid     = fork();  

    if ( pid == 0 ) { // Case in Child process
      P2();
      exit();
    }
    else if ( pid > 0 ){ // Case in Parent process
      exec( pid );  
    } 
    else 
      return;
  }
  else if ( extract_command("run p0 &\r", buffer) == 1 ){
    int pid = fork();
    return ;
  }
  else if ( extract_command("run p1 &\r", buffer) == 1 ){
    int pid = fork();
    return ;
  }
  else if ( extract_command("run p2 &\r", buffer) == 1 ){
    int pid = fork();
    return ;
  }
  else if ( extract_command("/proc\r", buffer) == 1 ){
    int pcbs = get_info();
    
    write(0, "Number of existing pcbs: ", 26);
    write_numb( pcbs );
    write(0, "\n", 1);
  }
  else if ( extract_command(":q\r", buffer) == 1 || extract_command("quit\r", buffer) == 1 ){
    //exit();  
  }
  else write(0, "ERROR -- command does not exist --\n", 36);
  
  return;
}


void terminal() {
  char buffer[ 101 ]; 
  
  while (1){
    write( 0, "terminal$ ", 10);
    read( buffer );
    write (0, "\n", 1);
    execute( buffer );
  }

  return;
}

void ( *entry_terminal )() = &terminal;