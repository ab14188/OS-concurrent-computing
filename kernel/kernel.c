#include "kernel.h"

void setTimer();
void scheduler( ctx_t* ctx 			);
pid_t getSlot( pid_t ppid );
void createPcb(pid_t pid, pid_t ppid, ctx_t* ctx );

pcb_t pcb[ 7 ], *current = NULL;

void kernel_handler_rst( ctx_t* ctx 		){
	setTimer();
	irq_enable();

	// P0
  	memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  	pcb[ 0 ].pid      = 0;
  	pcb[ 0 ].priority = 0;
  	pcb[ 0 ].ctx.cpsr = 0x50;
  	pcb[ 0 ].ctx.pc   = ( uint32_t )( entry_P0 );
  	pcb[ 0 ].ctx.sp   = ( uint32_t )(  &tos_P0 );

  	// P1
  	memset( &pcb[ 1 ], 0, sizeof( pcb_t ) );
  	pcb[ 1 ].pid      = 1;
  	pcb[ 1 ].priority = 1;
  	pcb[ 1 ].ctx.cpsr = 0x50;
  	pcb[ 1 ].ctx.pc   = ( uint32_t )( entry_P1 );
  	pcb[ 1 ].ctx.sp   = ( uint32_t )(  &tos_P1 );

  	// P2
  	memset( &pcb[ 2 ], 0, sizeof( pcb_t ) );
  	pcb[ 2 ].pid      = 2;
  	pcb[ 2 ].priority = 2;
  	pcb[ 2 ].ctx.cpsr = 0x50;
  	pcb[ 2 ].ctx.pc   = ( uint32_t )( entry_P2 );
  	pcb[ 2 ].ctx.sp   = ( uint32_t )(  &tos_P2 );

  	//This is the terminal  
  	memset( &pcb[ 6 ], 0, sizeof( pcb_t ) );
  	pcb[ 6 ].pid      = 6;
  	pcb[ 6 ].priority = 6;
  	pcb[ 6 ].ctx.cpsr = 0x50;
  	pcb[ 6 ].ctx.pc   = ( uint32_t )( entry_terminal );
  	pcb[ 6 ].ctx.sp   = ( uint32_t )(  &tos_terminal );

  	// Set the priorities to 0:
  	for ( int i = 3; i < 6; i++ ) {
  		memset( &pcb[ i ], 0, sizeof( pcb_t ) );
  		pcb[ i ].priority = -1;
  	}

  	// Set the start Point:
  	current = &pcb[ 6 ]; memcpy( ctx, &current->ctx, sizeof( ctx_t ) );

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
			pid_t cpid 	= getSlot( ppid );

			if ( cpid != -1) {
				createPcb( cpid, ppid, ctx );

				//Pass ctrl to the child
				// memcpy( &pcb[ ppid ].ctx, ctx, sizeof( ctx_t ) );
     			// memcpy( ctx, &pcb[ cpid ].ctx, sizeof( ctx_t ) );
     			// current = &pcb[ cpid ];

			}else {
				char *x = "Error: 2 many pcbs. cannot create child pcb. \n";
        			for( int i = 0; i < 47; i++ ) {
          			PL011_putc( UART0, *x++ );
        		}
			}

			if ( pcb[ ppid ].priority != ppid){
				ctx -> gpr[ 0 ] = 0;
			}
			else ctx -> gpr[ 0 ] = cpid;
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

void setTimer(){
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

// Scheduler 
void scheduler( ctx_t* ctx 			){
	if      ( current == &pcb[ 0 ] ) {
    	memcpy( &pcb[ 0 ].ctx, ctx, sizeof( ctx_t ) );
    	memcpy( ctx, &pcb[ 1 ].ctx, sizeof( ctx_t ) );
    	current = &pcb[ 1 ];
  	}
  	else if ( current == &pcb[ 1 ] ) {
    	memcpy( &pcb[ 1 ].ctx, ctx, sizeof( ctx_t ) );
    	memcpy( ctx, &pcb[ 2 ].ctx, sizeof( ctx_t ) );
    	current = &pcb[ 2 ];
  	}
  	else if ( current == &pcb[ 2 ] ) {
   		memcpy( &pcb[ 2 ].ctx, ctx, sizeof( ctx_t ) );
    	memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );
    	current = &pcb[ 0 ];
  	}
}

pid_t getSlot( pid_t ppid) {
	pid_t cpid 	= -1;
	int i 		= 3;

	while ( i < 6 ){
		if ( pcb[ i ].priority == (-1) ) {
			pcb[ i ].priority 	= ppid;
			cpid 				= i; 
			i 					= 6;
		}
		i++;
	}

	return cpid;
}

// Create new Process -- copy parent data into child data
void createPcb(pid_t pid, pid_t ppid, ctx_t* ctx ) {
	// Blank pcb cpid before writing into it 
   memset (&pcb[ pid ], 0, sizeof(pcb_t));

   // Copying parent pcb
   pcb[ pid ].priority = ppid;
   pcb[ pid ].pid      = pid;
   pcb[ pid ].ctx.pc   = pcb[ ppid ].ctx.pc;
   pcb[ pid ].ctx.cpsr = pcb[ ppid ].ctx.cpsr;
   pcb[ pid ].ctx.sp   = pcb[ ppid ].ctx.sp + (pid - ppid)*0x00001000;  
   memcpy( &pcb[ pid ].ctx, ctx, sizeof(ctx_t));
}
