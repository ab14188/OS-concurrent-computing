#include "terminal.h"

int extract_command( char* command, char* buffer ){
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



// add another timer timer1 such that you exit back to the terminal when its done because it took too much time
// to my taste 

// might need to reset timer0 when i pass control to another process however that implicates another way of
// determining wether or not a process is terminated such that the scheduler takes it into account when I 
// exit the whole thing

// read up on schedulers priority based 
// determin what to do when process terminated -- cannot pass control back to that => set priority to -1 
// determin what you want to do if the process runs forever -- have a max time it can actually run for before
// handling back control to the user -- set another timer to determin that 
// implement intelligent priority based scheduler 
// implement round-robin and compare the both of them, like what is more efficient and so on 
// implement communication between different processes
// copy the stack when forking to a child process
// might need to change the fork style into 2 different things : exec and fork instead of having both of them together that way fork can return what it really must ie cpid and 0 
// start looking at disks and so on