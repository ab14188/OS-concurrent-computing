#include "kernel.h"

void set_timer();
void scheduler( ctx_t* ctx 			);
void age_process();
int pcbs_info();
void create_child_pcb( pid_t pid, pid_t ppid, ctx_t* ctx );

int create_pcb( uint32_t pc, uint32_t sp, int priority); // might need to add the priority in here 
int get_numb_live_pcb();

pid_t get_slot( pid_t ppid );

int total_pcb 	= 8;
int age_Time 	= 0;
int max_Age     = 3;

pcb_t pcb[ 8 ], *current = NULL;


void kernel_handler_rst( ctx_t* ctx 		){
	set_timer();
	irq_enable();

  	// Blank every process and set the priorities of to -1:
  	for ( int i = 0; i < 8; i++ ) {
  		memset( &pcb[ i ], 0, sizeof( pcb_t ) );
  		pcb[ i ].priority = -1;
  	}

	// Creating terminal process
	pid_t pid = create_pcb(( uint32_t ) entry_terminal, ( uint32_t ) &tos_terminal, 0);
	
  	// Set the start Point:
  	current = &pcb[ pid ]; memcpy( ctx, &current->ctx, sizeof( ctx_t ) );
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
      		break;
		}
		case 0x02 :{ // int fork () 
			pid_t ppid 	= current -> pid;
			pid_t cpid 	= get_slot( ppid );
			int error 	= 0;

			if ( cpid != -1) {
				create_child_pcb( cpid, ppid, ctx );
				//scheduler( ctx );		
			}else {
				char *x = "ERROR -- error executing fork -- 2 many child process ? -- \n";
        			for( int i = 0; i < 60; i++ ) {
          			PL011_putc( UART0, *x++ );
        		}
        		error = -1;
			}
		
			if ( error == -1 ) ctx -> gpr[ 0 ] = -1;
			else ctx -> gpr[ 0 ] = cpid; 
			break;
		}
		case 0x03 :{ // int exit ()
			pid_t pid 	= current -> pid;

			memset (&pcb[ pid ], 0, sizeof(pcb_t));
			pcb[ pid ].priority = -1;

     		// reset the stack ? -- how do you do this 
     		//memcpy (&pcb[ pid ].ctx.sp, ctx->sp, );


     		//memset (&pcb[ pid ].ctx, 0, sizeof(ctx_t));
     		ctx -> gpr[ 0 ] = -1; // returns -1 to say that it is dead  
     		scheduler( ctx );
			break;
		}
		case 0x04 :{ // int exec ( int pid )
			int pid = ( int ) (ctx -> gpr[0]) ;
			//memcpy( &pcb[ pid ].ctx, ctx, sizeof(ctx_t));
			scheduler( ctx );
			ctx -> gpr[0] = 0;
			break;
		}
		case 0x05:{ // int get_info()
			int pcbs = pcbs_info();
			ctx -> gpr[0] = pcbs;
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
		
		age_Time +=1 ;
		
		if ( age_Time >= max_Age ) {
			age_process();
			age_Time = 0;
		} 

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

	if ( best > (-1) ) { // there is no point in passing ctrl to the same pcb
		memcpy( &pcb[ current -> pid ].ctx, ctx, sizeof( ctx_t ) );
     	memcpy( ctx, &pcb[ best ].ctx, sizeof( ctx_t ) );
     	current = &pcb[ best ];
	}
}


// Get empty slot for the new Child process 
pid_t get_slot( pid_t ppid ) {
	pid_t cpid 	= -1;
	int i 		= 0;

	while ( i < 8 ){
		if ( pcb[ i ].priority == (-1) && pcb[ i ].pid == 0) {
			pcb[ i ].priority 	= ppid;
			cpid 				= i; 
			i 					= 8;
		}
		i++;
	}

	return cpid;
}

// Create new Process  
pid_t create_pcb( uint32_t pc, uint32_t sp, int priority){
	pid_t pid = get_slot( 0 );

	memset( &pcb[ pid ], 0, sizeof( pcb_t ) );
  	pcb[ pid ].pid      = pid;
  	pcb[ pid ].priority = priority;
  	pcb[ pid ].ctx.cpsr = 0x50;
  	pcb[ pid ].ctx.pc   = ( uint32_t )( pc );
  	pcb[ pid ].ctx.sp   = ( uint32_t )( sp );

  	return pid;
}

// Create new Process -- copy parent data into child data
void create_child_pcb( pid_t pid, pid_t ppid, ctx_t* ctx ){
	// Blank pcb cpid before writing into it 
   	memset (&pcb[ pid ], 0, sizeof(pcb_t));

   	// Copying parent pcb
   	pcb[ pid ].priority = get_numb_live_pcb(); 
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

void age_process(){
	int pid = current -> pid;

	if ( pcb[ pid ].priority != (-1) && pcb[ pid ].priority > 0 ){
		pcb[ pid ].priority -= 1;
	}
}

int pcbs_info(){
	int pcbs = 0;
	for (int i = 0; i < total_pcb ; i++){
		if ( pcb[ i ].ctx.sp != 0){
			pcbs ++;
		}
	}
	return pcbs;
}