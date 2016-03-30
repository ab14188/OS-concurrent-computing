#include "kernel.h"

void set_timer();
void scheduler( ctx_t* ctx 			);
void create_child_pcb( pid_t pid, pid_t ppid, ctx_t* ctx );
void create_pcb( pid_t pid, uint32_t pc, uint32_t sp); // might need to add the priority in here 
int get_numb_live_pcb();

pid_t get_slot( pid_t ppid );

pcb_t pcb[ 8 ], *current = NULL;
int total_pcb = 8;

void kernel_handler_rst( ctx_t* ctx 		){
	set_timer();
	irq_enable();

	create_pcb(0, ( uint32_t ) entry_P0, ( uint32_t ) &tos_P0);
	create_pcb(1, ( uint32_t ) entry_P1, ( uint32_t ) &tos_P1);
	create_pcb(2, ( uint32_t ) entry_P2, ( uint32_t ) &tos_P2);
	create_pcb(3, ( uint32_t ) entry_terminal, ( uint32_t ) &tos_terminal);
	
  	// Set the priorities of unused to -1:
  	for ( int i = 4; i < 8; i++ ) {
  		memset( &pcb[ i ], 0, sizeof( pcb_t ) );
  		pcb[ i ].priority = -1;
  	}

  	// Set the start Point:
  	current = &pcb[ 3 ]; memcpy( ctx, &current->ctx, sizeof( ctx_t ) );

  	return; 
}

// Handle the System calls 
void kernel_handler_svc( ctx_t* ctx, uint32_t id ){
	switch( id ){
		case 0x00 :{ // int write( int fd, void* x, size_t n )
			int   fd = ( int   )( ctx->gpr[ 0 ] );  
      		char*  x = ( char* )( ctx->gpr[ 1 ] );  
      		int    n = ( int   )( ctx->gpr[ 2 ] ); 

      		for( int i = 0; i < n; i++ ) {
        		PL011_putc( UART0, *x++ );
      		} 
      
      		ctx->gpr[ 0 ] = n;
      		break;
		}
		case 0x01 :{ // int read ( *buffer ) 
			//irq_unable();
			char*  buffer = ( char* )( ctx->gpr[ 0 ] );
      		int index     = 0;
      		int broken    = 1;

      		while( broken ){
        		buffer[ index ] = PL011_getc( UART0 );
        		if (buffer[ index ] == '\r')   broken = 0;
        		PL011_putc( UART0, buffer[ index ] );
        		index++;
      		}

      		ctx -> gpr[ 0 ] = index - 1; 

      		//irq_enable();
      		//TIMER0 -> Timer1IntClr = 0x01;
      		break;
		}
		case 0x02 :{ // int fork () 
			// disable the timer
			//irq_unable();

			pid_t ppid 	= current -> pid;
			pid_t cpid 	= get_slot( ppid );
			int error 	= 0;

			if ( cpid != -1) {
				create_child_pcb( cpid, ppid, ctx );
				scheduler( ctx );
				// Pass ctrl to the child
        		// memcpy( &pcb[ cpid ].ctx, ctx, sizeof(ctx_t));
        		// memcpy(&pcb[ ppid ].ctx, ctx, sizeof( ctx_t ) );      		
        		// memcpy( ctx, &pcb[ cpid ].ctx, sizeof( ctx_t ) );
        		// current = &pcb[ cpid ];
			
			}else {
				char *x = "ERROR -- error executing fork --\n      -- 2 many child process ? -- \n";
        			for( int i = 0; i < strlen(x); i++ ) {
          			PL011_putc( UART0, *x++ );
        		}
        		error = -1;
			}
			
			//As I always give control to the child when I fork -- different to POSIX -- this should always return 0 and -1 if error
			if ( error == -1 ) ctx -> gpr[ 0 ] = -1;
			else ctx -> gpr[ 0 ] = 0; 
			break;
		}
		case 0x03 :{ // int exit ()
			pid_t pid 	= current -> pid;
			pid_t ppid 	= pcb[ pid ].priority; 

			memcpy( &pcb[ pid ].ctx, ctx, sizeof( ctx_t ) );
     		memcpy( ctx, &pcb[ ppid ].ctx, sizeof( ctx_t ) );
			
			memset (&pcb[ pid ], 0, sizeof(pcb_t));
			pcb[ pid ].priority = -1;
     		
     		current = &pcb[ ppid ];
     		ctx -> gpr[ 0 ] = pid; // returns the pid of the child process terminated this is kind of like a mix between wait and exit  
			
			//setting back the timer 
			//irq_enable();
			//TIMER0 -> Timer1IntClr = 0x01;
			break;
		}
		default	:{
			char* error = "ERROR\n";
			for ( int i = 0; i < 6; i++) {
				PL011_putc( UART0, *error++ );
			}

			break;
		}
	}
}

// Handle the Interrupt Requests
void kernel_handler_irq( ctx_t* ctx 		){
	// Read interrupt Id
	uint32_t id = GICC0 -> IAR;

	// Handle interrupt then reset Timer
	if ( id == GIC_SOURCE_TIMER0 ) {
		TIMER0 -> Timer1IntClr = 0x01;
		scheduler( ctx ); 
	}

	// Signal that we are done
	GICC0 -> EOIR = id;
}

void set_timer(){
	TIMER0->Timer1Load     = 0x00010000; // select period = 2^20 ticks ~= 1 sec // change the 1 to 2 => 2sec -- you can make this much faster and much slower
  	TIMER0->Timer1Ctrl     = 0x00000002; // select 32-bit   timer
  	TIMER0->Timer1Ctrl    |= 0x00000040; // select periodic timer
  	TIMER0->Timer1Ctrl    |= 0x00000020; // enable          timer interrupt
  	TIMER0->Timer1Ctrl    |= 0x00000080; // enable          Timer1Ctrl

  	GICC0->PMR             = 0x000000F0; // unmask all            interrupts
  	GICD0->ISENABLER[ 1 ] |= 0x00000010; // enable timer          interrupt
  	GICC0->CTLR            = 0x00000001; // enable GIC interface
  	GICD0->CTLR            = 0x00000001; // enable GIC distributor
}

// Scheduler : given // round robin ? or the simple predefined one I dont know
// void scheduler( ctx_t* ctx 			){
// 	if      ( current == &pcb[ 0 ] ) {
//     	memcpy( &pcb[ 0 ].ctx, ctx, sizeof( ctx_t ) );
//     	memcpy( ctx, &pcb[ 1 ].ctx, sizeof( ctx_t ) );
//     	current = &pcb[ 1 ];
//   	}
//   	else if ( current == &pcb[ 1 ] ) {
//     	memcpy( &pcb[ 1 ].ctx, ctx, sizeof( ctx_t ) );
//     	memcpy( ctx, &pcb[ 2 ].ctx, sizeof( ctx_t ) );
//     	current = &pcb[ 2 ];
//   	}
//   	else if ( current == &pcb[ 2 ] ) {
//    		memcpy( &pcb[ 2 ].ctx, ctx, sizeof( ctx_t ) );
//     	memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );
//     	current = &pcb[ 0 ];
//   	}
// }

// Scheduler : priority -- this should give priority to the one with the highest priority 
// there is a problem with this for now there is definitely some sort of pb 
// should not give control back to the parent should it ?
// have a case for when you finish a program 
void scheduler( ctx_t* ctx 			){
	int best = -1;

	for ( int i = 0; i < 7 ; i++ ) { // up to the possible number of processes -- need some sort of thing to determin wether or not should pass control 
		if ( pcb[ i ].priority > best /*&& i != current -> pid*/ ) best = i;
	}

	if ( best >= 0 && best != current -> pid ) { // there is no point in passing ctrl to the same pcb
		memcpy( &pcb[ current -> pid ].ctx, ctx, sizeof( ctx_t ) );
     	memcpy( ctx, &pcb[ best ].ctx, sizeof( ctx_t ) );
     	current = &pcb[ best ];
	}
}


// Get empty slot for the new Child process 
pid_t get_slot( pid_t ppid) {
	pid_t cpid 	= -1;
	int i 		= 4;

	while ( i < 8 ){
		if ( pcb[ i ].priority == (-1) ) {
			pcb[ i ].priority 	= ppid;
			cpid 				= i; 
			i 					= 8;
		}
		i++;
	}

	return cpid;
}

// Create new Process  
void create_pcb( pid_t pid, uint32_t pc, uint32_t sp ){

	memset( &pcb[ pid ], 0, sizeof( pcb_t ) );
  	pcb[ pid ].pid      = pid;
  	pcb[ pid ].priority = pid;
  	pcb[ pid ].ctx.cpsr = 0x50;
  	pcb[ pid ].ctx.pc   = ( uint32_t )( pc );
  	pcb[ pid ].ctx.sp   = ( uint32_t )(  sp );
}

// Create new Process -- copy parent data into child data
void create_child_pcb( pid_t pid, pid_t ppid, ctx_t* ctx ){
	// Blank pcb cpid before writing into it 
   	memset (&pcb[ pid ], 0, sizeof(pcb_t));

   	// Copying parent pcb
   	pcb[ pid ].priority = get_numb_live_pcb() + 1;//ppid; // might need to change how the priority works here 
   	pcb[ pid ].pid      = pid;
   	pcb[ pid ].ctx.pc   = pcb[ ppid ].ctx.pc;
   	pcb[ pid ].ctx.cpsr = pcb[ ppid ].ctx.cpsr;
   	pcb[ pid ].ctx.sp   = pcb[ ppid ].ctx.sp + (pid - ppid)*0x00001000; 

   	memcpy( &pcb[ pid ].ctx, ctx, sizeof(ctx_t));
}

// Gets number of live processes
int get_numb_live_pcb(){
	int livePcb = 0;
	
	for( int i = 0; i < total_pcb; i++ ) {
		if ( pcb[ i ].priority != (-1) ) livePcb += 1;
	}

	return livePcb;
}