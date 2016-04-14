#include "kernel.h"

void 	init_timer	(); 											// initialise the timer
void 	init_ipcs_pcbs(); 											// initialise the space reserved for pcbs and ipcs
void 	age_process(); 												// age current process
void 	scheduler ( ctx_t* ctx 			); 							// priority scheduler
void 	write_error( char* error_msg, int size); 					// write error msg

int 	pcbs_info(); 												// get pcbs information
int 	get_numb_live_pcb(); 										// get number of used pcbs
int 	get_ipc_slot(); 											// get an empty slot for ipc
pid_t 	get_pcb_slot( pid_t ppid ); 								// get slot for new pcb
int 	create_ipc(); 												// create ipc
int 	create_pcb( uint32_t pc, uint32_t sp, int priority); 		// create pcb 
void 	create_child_pcb( pid_t pid, pid_t ppid, ctx_t* ctx ); 		// create child pcb 


void init_timer0_2( uint32_t sleep_time );
void init_timer1_1( uint32_t sleep_time );
void init_timer1_2( uint32_t sleep_time );
void init_timer2_1( uint32_t sleep_time );
void init_timer2_2( uint32_t sleep_time );


int total_pcb 	= 8;                                                // number of pcbs
int age_Time 	= 0;												// age of current process
int max_Age     = 3;												// max age a process can get 2
int awake[5];
uint32_t stack 	= (uint32_t) &tos_terminal;

pcb_t pcb[ 8 ], *current = NULL;
ipc_t ipc[ 8 ];


void kernel_handler_rst( ctx_t* ctx 		){
	init_timer();
	irq_enable();
	
	init_ipcs_pcbs();

	// Creating terminal process
	pid_t pid = create_pcb(( uint32_t ) entry_terminal, ( uint32_t ) &tos_terminal, 0);
	
  	// Set the start Point:
  	current = &pcb[ pid ]; memcpy( ctx, &current->ctx, sizeof( ctx_t ) );

  	// For philosophers 
  	for ( int i = 0; i< 5; i++ ) awake[i] = 0;
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
			pid_t cpid 	= get_pcb_slot( ppid );
			int error 	= 0;

			if ( cpid != -1) {
				create_child_pcb( cpid, ppid, ctx );
				//pcb[cpid].ctx.gpr[0] = cpid;	 might need to change this 
			}else {
				char *error_msg  = "-- error executing fork -- 2 many child process ? -- ";
				write_error( error_msg, 54);
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

			// memcpy( &pcb[ current -> pid ].ctx, ctx, sizeof( ctx_t ) );
   //   		memcpy( ctx, &pcb[ pid ].ctx, sizeof( ctx_t ) );
   //   		current = &pcb[ pid ];
			//memcpy( &pcb[ pid ].ctx, ctx, sizeof(ctx_t));
			scheduler( ctx );
			ctx -> gpr[0] = 0;
			break;
		}
		case 0x05 :{ // int get_info()
			int pcbs = pcbs_info();
			ctx -> gpr[0] = pcbs;
			break;
		}
		case 0x06 :{ // int create_channel( int chan_start, int chan_end);
			int chan_start 	= ( int )(ctx -> gpr[0]);
			int chan_end 	= ( int )(ctx -> gpr[1]);
			int ipc_id 		= create_ipc(chan_start, chan_end);

			if ( ipc_id == -1){
				char* error_msg = " -- creating channel --";
				write_error( error_msg, 24);
			} 

			ctx -> gpr[0] = ipc_id;
			break;
		}
		case 0x07 :{ // yield (int pid);
			int pid = ( int )(ctx -> gpr[0]);

			memcpy( &pcb[ current -> pid ].ctx, ctx, sizeof( ctx_t ) );
     		memcpy( ctx, &pcb[ pid ].ctx, sizeof( ctx_t ) );
     		current = &pcb[ pid ];
     		ctx -> gpr[0] = pid;
     		break;
		}
		case 0x08 :{ // void* read_channel( int channel_id )
			break;
		}
		case 0x09 :{ // void* write_channel( int channel_id, void* msg )
			break;
		}
		case 10 :{ // int sleep ( int i )
			int timer_id = ( int )(ctx -> gpr[0]);
			uint32_t sleep_time = (uint32_t)(ctx -> gpr[1]);
			irq_unable(); //look if it is efficient ornot to decactivate and then reactivate the timers 
			switch( timer_id ){
				case 0x00 :{
					init_timer0_2( sleep_time ); 
					irq_enable();
					write(0, "Asleep 0_2 \n", 11);
					while( !awake[0] ){ /*wait till done sleeping*/ }
					awake[0] = 0;
					write(0, "Awake 0_2 \n", 11);
					break;
				}
				case 0x01:{
					init_timer1_1( sleep_time );
					irq_enable();
					while( !awake[1] ){ }
					write(0, "Timer 1_1 \n", 11);
					break;
				}
				case 0x02 :{
					init_timer1_2( sleep_time );
					irq_enable();
					while( !awake[2] ){ }
					write(0, "Timer 1_2 \n", 11);
					break;
				}
				case 0x03:{
					init_timer2_1( sleep_time );
					irq_enable();
					while( !awake[3] ){ }
					write(0, "Timer 2_1 \n", 11);
					break;
				}
				case 0x04:{
					init_timer2_2( sleep_time );
					irq_enable();
					while( !awake[4] ){ }
					write(0, "Timer 2_3 \n", 11);
					break;
				}
				default : {
					char* error_msg = " -- timer id -- \n";
					write_error( error_msg, 17);
					break;
				}
			}
			irq_enable();
			break;
		}
		default	:{
			char* error_msg = " -- kernel oops -- ";
			write_error( error_msg, 20);
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
		
		if ( TIMER0 -> Timer1Value == 0x00 ) {
			age_Time +=1 ;
			
			if ( age_Time >= max_Age ) {
				age_process();
				age_Time = 0;
			} 

			scheduler( ctx ); 
			//write(0, "Timer0 1ctrl irq\n", 19);
			TIMER0 -> Timer1IntClr = 0x01;
		}
		else if ( TIMER0 -> Timer2Value == 0x00 ) { // case philo[0] 
			// Wake up process
			awake[0] = 1;

			// Timer reset
			TIMER0 -> Timer2IntClr 	= 0x01;     // reset timer for when I want to use it again 
			TIMER0 -> Timer2Ctrl 	= 0x00000000; // this unables the timer 
		}
	}
	else if ( id == GIC_SOURCE_TIMER1 ){
		if ( TIMER1 -> Timer1Value == 0x00 ) { 		// case philo[1]
			// Wake up process
			awake[1] = 1;

			// Timer reset
			TIMER1 -> Timer1IntClr 	= 0x01;     // reset timer for when I want to use it again 
			TIMER1 -> Timer1Ctrl 	= 0x00000000; // this unables the timer 
		}
		else if ( TIMER1 -> Timer2Value == 0x00 ) { //case philo[2]
			// Wake up process
			awake[2] = 1;

			// Timer reset
			TIMER1 -> Timer2IntClr 	= 0x01;     // reset timer for when I want to use it again 
			TIMER1 -> Timer2Ctrl 	= 0x00000000; // this unables the timer 
		}
	}
	else if ( id == GIC_SOURCE_TIMER2 ){
		if ( TIMER2 -> Timer1Value == 0x00 ) { 		// case philo[3]
			// Wake up process
			awake[3] = 1;

			// Timer reset
			TIMER2 -> Timer1IntClr 	= 0x01;     // reset timer for when I want to use it again 
			TIMER2 -> Timer1Ctrl 	= 0x00000000; // this unables the timer 
		}
		else if ( TIMER2 -> Timer2Value == 0x00 ) { //case philo[4]
			// Wake up process
			awake[4] = 1;

			// Timer reset
			TIMER2 -> Timer2IntClr 	= 0x01;     // reset timer for when I want to use it again 
			TIMER2 -> Timer2Ctrl 	= 0x00000000; // this unables the timer 
		}
	}

	// Signal that we are done
	GICC0 -> EOIR = id;
}

// Initialise timer
void init_timer(){
	TIMER0->Timer1Load     = 0x01000000; // select period = 2^20 ticks ~= 1 sec  
  	TIMER0->Timer1Ctrl     = 0x00000002; // select 32-bit   timer
  	TIMER0->Timer1Ctrl    |= 0x00000040; // select periodic timer
  	TIMER0->Timer1Ctrl    |= 0x00000020; // enable          timer interrupt
  	TIMER0->Timer1Ctrl    |= 0x00000080; // enable          TIMER0Ctrl

  	GICC0->PMR             = 0x000000F0; // unmask all            interrupts
  	GICD0->ISENABLER[ 1 ] |= 0x00000010; // enable timer          interrupt
  	GICC0->CTLR            = 0x00000001; // enable GIC interface
  	GICD0->CTLR            = 0x00000001; // enable GIC distributor
}

// Initialise timer for philosopher 
void init_timer0_2( uint32_t sleep_time ){
	TIMER0->Timer2Load     = sleep_time; // select period = 2^20 ticks ~= 1 sec // change the 1 to 2 => 2sec -- you can make this much faster and much slower
  	TIMER0->Timer2Ctrl     = 0x00000002; // select 32-bit   timer
  	TIMER0->Timer2Ctrl    |= 0x00000040; // select periodic timer
  	TIMER0->Timer2Ctrl    |= 0x00000020; // enable          timer interrupt
  	TIMER0->Timer2Ctrl    |= 0x00000080; // enable          TIMER0Ctrl

  	GICC0->PMR             = 0x000000F0; // unmask all            interrupts
  	GICD0->ISENABLER[ 1 ] |= 0x00000010; // enable timer          interrupt
  	GICC0->CTLR            = 0x00000001; // enable GIC interface
  	GICD0->CTLR            = 0x00000001; // enable GIC distributor
}

void init_timer1_1( uint32_t sleep_time ){
	TIMER1->Timer1Load     = sleep_time; // select period = 2^20 ticks ~= 1 sec // change the 1 to 2 => 2sec -- you can make this much faster and much slower
  	TIMER1->Timer1Ctrl     = 0x00000002; // select 32-bit   timer
  	TIMER1->Timer1Ctrl    |= 0x00000040; // select periodic timer
  	TIMER1->Timer1Ctrl    |= 0x00000020; // enable          timer interrupt
  	TIMER1->Timer1Ctrl    |= 0x00000080; // enable          TIMER0Ctrl

  	GICC0->PMR             = 0x000000F0; // unmask all            interrupts
  	GICD0->ISENABLER[ 1 ] |= 0x00000010; // enable timer          interrupt
  	GICC0->CTLR            = 0x00000001; // enable GIC interface
  	GICD0->CTLR            = 0x00000001; // enable GIC distributor
}

void init_timer1_2( uint32_t sleep_time ){
	TIMER1->Timer2Load     = sleep_time; // select period = 2^20 ticks ~= 1 sec // change the 1 to 2 => 2sec -- you can make this much faster and much slower
  	TIMER1->Timer2Ctrl     = 0x00000002; // select 32-bit   timer
  	TIMER1->Timer2Ctrl    |= 0x00000040; // select periodic timer
  	TIMER1->Timer2Ctrl    |= 0x00000020; // enable          timer interrupt
  	TIMER1->Timer2Ctrl    |= 0x00000080; // enable          TIMER0Ctrl

  	GICC0->PMR             = 0x000000F0; // unmask all            interrupts
  	GICD0->ISENABLER[ 1 ] |= 0x00000010; // enable timer          interrupt
  	GICC0->CTLR            = 0x00000001; // enable GIC interface
  	GICD0->CTLR            = 0x00000001; // enable GIC distributor
}
void init_timer2_1( uint32_t sleep_time ){
	TIMER2->Timer1Load     = sleep_time; // select period = 2^20 ticks ~= 1 sec // change the 1 to 2 => 2sec -- you can make this much faster and much slower
  	TIMER2->Timer1Ctrl     = 0x00000002; // select 32-bit   timer
  	TIMER2->Timer1Ctrl    |= 0x00000040; // select periodic timer
  	TIMER2->Timer1Ctrl    |= 0x00000020; // enable          timer interrupt
  	TIMER2->Timer1Ctrl    |= 0x00000080; // enable          TIMER0Ctrl

  	GICC0->PMR             = 0x000000F0; // unmask all            interrupts
  	GICD0->ISENABLER[ 1 ] |= 0x00000010; // enable timer          interrupt
  	GICC0->CTLR            = 0x00000001; // enable GIC interface
  	GICD0->CTLR            = 0x00000001; // enable GIC distributor
}
void init_timer2_2( uint32_t sleep_time ){
	TIMER2->Timer2Load     = sleep_time; // select period = 2^20 ticks ~= 1 sec // change the 1 to 2 => 2sec -- you can make this much faster and much slower
  	TIMER2->Timer2Ctrl     = 0x00000002; // select 32-bit   timer
  	TIMER2->Timer2Ctrl    |= 0x00000040; // select periodic timer
  	TIMER2->Timer2Ctrl    |= 0x00000020; // enable          timer interrupt
  	TIMER2->Timer2Ctrl    |= 0x00000080; // enable          TIMER0Ctrl

  	GICC0->PMR             = 0x000000F0; // unmask all            interrupts
  	GICD0->ISENABLER[ 1 ] |= 0x00000010; // enable timer          interrupt
  	GICC0->CTLR            = 0x00000001; // enable GIC interface
  	GICD0->CTLR            = 0x00000001; // enable GIC distributor
}


// Scheduler : priority based
void scheduler( ctx_t* ctx 			){
	int best = -1;

	for ( int i = 0; i < 7 ; i++ ) {  
		if ( pcb[ i ].priority > best /*&& i != current -> pid*/ ) best = i;
	}

	if ( best > (-1) ) { 
		memcpy( &pcb[ current -> pid ].ctx, ctx, sizeof( ctx_t ) );
     	memcpy( ctx, &pcb[ best ].ctx, sizeof( ctx_t ) );
     	current = &pcb[ best ];
	}
}


// Get empty slot for the new Child process 
pid_t get_pcb_slot( pid_t ppid ) {
	pid_t cpid 	= -1;
	int i 		= 0;

	while ( i < total_pcb ){
		if ( pcb[ i ].priority == (-1) && pcb[ i ].pid == 0) {
			pcb[ i ].priority 	= ppid;
			cpid 				= i; 
			i 					= total_pcb;
		}
		i++;
	}

	return cpid;
}

// Create new Process  
pid_t create_pcb( uint32_t pc, uint32_t sp, int priority){
	pid_t pid = get_pcb_slot( 0 );

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
	//stack = stack - (pid - ppid )*0x00001000;need to do some stack  manipulation of some sort ... not sure how ghghhh c
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

// Age the current pcb
void age_process(){
	int pid = current -> pid;

	if ( pcb[ pid ].priority != (-1) && pcb[ pid ].priority > 0 ){
		pcb[ pid ].priority -= 1;
	}
}


// Get information on pcbs
int pcbs_info(){
	int pcbs = 0;

	for (int i = 0; i < total_pcb ; i++){
		if ( pcb[ i ].ctx.sp != 0){
			pcbs ++;
		}
	}
	
	return pcbs;
}

// Initialise the space reserved for ipcs and pcbs
void init_ipcs_pcbs(){
	// Blank every ipcs
	for ( int i = 0; i < 8; i++ ){
		memset( &ipc[ i ], -1, sizeof( ipc ) ); 
	}

	// Blank every process and set the priorities of to -1:
  	for ( int i = 0; i < total_pcb; i++ ) {
  		memset( &pcb[ i ], 0, sizeof( pcb_t ) );
  		pcb[ i ].priority = -1;
  	}
}


// Get empty slot for channel between 2 pcbs
int get_ipc_slot(){
	for ( int i = 0; i < 8; i++ ){
		if ( ipc[ i ].chan_end == -1 ) return i;
	}

	return -1;
}

// Create channel between 2 pcbs
int create_ipc(int chan_start, int chan_end){
	int id 					= get_ipc_slot();
	
	if ( id != -1 ){
		ipc[ id ].chan_start 	= chan_start;
		ipc[ id ].chan_end 	 	= chan_end;
	}

	return id;
}

// Write error msg 
void write_error(char* error_msg, int size){
	char *error = "ERROR ";

	for( int i = 0; i < 6; i++ ) {
			PL011_putc( UART0, *error++ );
	}
	for( int i = 0; i < size; i++ ) {
			PL011_putc( UART0, *error_msg++ );
	}
}