#include "Philosophers.h"
int     before  = 1;
int     ctrl    = -1; 

typedef struct {
    int id;
    int hungry;
    int left_chopstick;
    int right_chopstick;
    int has_left; //set to: 0 when on the table : -1 when in others hand : 1 when in yours
    int has_right; //set to: 0 when on the table : -1 when in others hand : 1 when in yours
} phils_t;

int philo[5];
phils_t phils[5];

int stop_hungry( int hungry ){
    return 1; 
}

void eat( int  id ){
    write(0, "Philosopher: " , 13); write_numb(id); write(0, " is eating \n", 12);
}

// Function that makes philosopher sleep. This is done by using timers
// Each philosopher has his own thinking timer 
// Timer0,1 := philo[0]  -- Timer1,0 := philo[1] -- Timer1,1 :=  philo[2] -- Timer2,0 := philo[3] -- Timer2,1 := philo[4] 
void think( int id, int i ){
    //uint32_t think_time = 
    //sleep( i, think_time ); 
    write(0, "Philosopher: " , 13); write_numb(id); write(0, " is thinking \n", 14);
}

int get_phil_pos( int id ){
    for ( int i = 0;  i < 5; i++ ){
        if ( phils[ i ].id == id ) return i;
    }
    
    return -1;
}

void hungry_philo( int id, int i ){
    phils[ i ].hungry = 1;
    write(0, "Philosopher: ", 13); write_numb( id ); write(0, " is hungry!!\n", 13);
}

void got_right( int id ){
    //send msg through channels 
    write(0, "Philosopher: ", 13); write_numb( id ); write(0, " has right chopstick!!\n", 24);
}

void got_left( int id ){
    //send msg through channels 
    write(0, "Philosopher: ", 13); write_numb( id ); write(0, " has left chopstick!!\n", 24);
}

void get_right( int id, int i ){
    write(0, "Philosopher: ", 13); write_numb( id ); write(0, " wants right chopstick!!\n", 26);
    // if right chopstick free => ask the other person through channel .. send request get it else wait 
    int free_chop = 0; 
    while ( !free_chop ){
        //read_channel
    }
    //pick up chopstick
    phils[i].right_chopstick = 1;
    //send got chopstick
    got_right( id ); 
}

void get_left( int id, int i ){
    write(0, "Philosopher: ", 13); write_numb( id ); write(0, " wants left chopstick!!\n", 26);
    // if right chopstick free => ask the other person through channel .. send request get it else wait 
    int free_chop = 0; 
    while ( !free_chop ){

    }
    //pick up chopstick
    phils[i].left_chopstick = 1;
    //send got chopstick
    got_left( id );
}

void right_put( int id, int i){
    phils[i].left_chopstick = 0;
    write(0, "Philosopher: ", 13); write_numb( id ); write(0, " right chopstick on table!!\n", 32);
    //send msg through channel
}

void left_put( int id, int i){
    phils[i].right_chopstick = 0;
    write(0, "Philosopher: ", 13); write_numb( id ); write(0, " left chopstick on table!!\n", 32);
    //send msg through channel 
}

void guest( int id ){
    int i = get_phil_pos( id );
    
    if ( i == -1 ){
        write(0, "ERROR --running the philos-- \n", 30);
        while( 1 ) {}
    }
    while ( phils[ i ].hungry == 0 ){
        think( id, i);                // this has a timer of some sorts -- ask if they should think or not 
        hungry_philo( id, i );
        get_right( id, i );
        get_left( id, i );
        eat( id );                  // this has a timer of some sorts
        right_put( id, i );
        left_put( id , i);
    }
    write(0, "Philosopher: ", 13); write_numb( id ); write(0, " eating proedure success!!\n", 32);
}


void init_phils( int philo_1, int philo_2, int philo_3, int philo_4, int philo_5){
    phils[0].id              = philo_1;
    phils[0].hungry          = 0;
    phils[0].left_chopstick  = 5;
    phils[0].right_chopstick = 1;

    phils[1].id              = philo_2;
    phils[1].hungry          = 0;
    phils[1].left_chopstick  = 1;
    phils[1].right_chopstick = 2;

    phils[2].id              = philo_3;
    phils[2].hungry          = 0;
    phils[2].left_chopstick  = 2;
    phils[2].right_chopstick = 3;
    
    phils[3].id              = philo_4;
    phils[3].hungry          = 0;
    phils[3].left_chopstick  = 3;
    phils[3].right_chopstick = 4;

    phils[4].id              = philo_5;
    phils[4].hungry          = 0;
    phils[4].left_chopstick  = 4;
    phils[4].right_chopstick = 5;
}

void Table() {
    // Need some sort of clever way to think about this 
    // Forking the processes to have 5 philosophers
    if ( before != -1 ){
        philo[0] = fork();
    }
    if (before != -1){
        philo[1] = fork(); 
    }  
    if (before != -1){
        philo[2] = fork(); 
    }
    if (before != -1){
        philo[3] = fork(); 
    }
    if (before != -1){
        philo[4] = fork(); 
    }


    // Setting up channels between philosophers so that they can communicate data 
    if (before != -1) { 
        int chan1_2 = create_channel( philo[0], philo[1]);
        int chan2_3 = create_channel( philo[1], philo[2]);
        int chan3_4 = create_channel( philo[2], philo[3]);
        int chan4_5 = create_channel( philo[3], philo[4]);
        int chan5_1 = create_channel( philo[4], philo[5]);
    }

    if ( before != -1 ) init_phils( philo[0], philo[1], philo[2], philo[3], philo[4]);

    // Stop it from froking again 
    before = -1;  
   
    // Give ctrl to the first guest 
    if ( ctrl == -1){
        ctrl = yield( philo[0] ); 
    }
    // Seat the guests at the table and start 
    guest( philo[0] );
    // exit + give control to other process

    //termination msg before exiting the final philosopher process
    write(0, "Philosophical dinner has terminated\n", 36);
    // ctrl should now be given back to the terminal 
}

void (*entry_Philosophers)() = &Table;

/* Logic on channels -- specific to this problem: 
* channels to write to must be:  
* channels to read from must be: 
*/


// unable the timer for this and then enable it again when the whole thing is done