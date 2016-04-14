#include "Philosophers.h"
int     before_forks  = 1;
int     ctrl    = -1; 
int     chan1_2, chan2_3, chan3_4, chan4_5, chan5_1;

typedef struct {
    int id;
    int hungry;
    int left_chopstick;
    int right_chopstick;
    int has_left; //set to: 0 when on the table : -1 when in others hand : 1 when in yours
    int has_right; //set to: 0 when on the table : -1 when in others hand : 1 when in yours
    int chan_right;
    int chan_left;
} phils_t;

int philo[5];
phils_t phils[5];

int stop_hungry( int hungry ){
    return 1; 
}


uint32_t generateRandom(){
    uint32_t random_array[15];
    random_array[0] = 0x00010000;
    random_array[1] = 0x00020000;
    random_array[2] = 0x00030000;
    random_array[3] = 0x00000100;
    random_array[4] = 0x00050000;
    random_array[5] = 0x00060000;
    random_array[6] = 0x00002000;
    random_array[7] = 0x00080000;
    random_array[8] = 0x00090000;
    random_array[9] = 0x00100000;
    random_array[10] = 0x00200000;
    random_array[11] = 0x00300000;
    random_array[12] = 0x00001000;
    random_array[13] = 0x00009000;
    random_array[14] = 0x01000000;

    uint32_t x = random_array[ rand() % 15];
    return x;
}

void eat( int id, int i ){
    uint32_t eat_Time = generateRandom();
    sleep( i, eat_Time); // time to eat  -- should have some place where I say I eat
    write(0, "Philosopher: " , 13); write_numb(i); write(0, " is eating \n", 12);
}

// Function that makes philosopher sleep. This is done by using timers
// Each philosopher has his own thinking timer 
// Timer0,1 := philo[0]  -- Timer1,0 := philo[1] -- Timer1,1 :=  philo[2] -- Timer2,0 := philo[3] -- Timer2,1 := philo[4] 
void think( int id, int i ){
    uint32_t  think_time = generateRandom();
    write(0, "Philosopher: " , 13); write_numb(i + 1); write(0, " is thinking \n", 14);
    sleep( i, think_time ); 
}


int get_phil_pos( int id ){
    for ( int i = 0;  i < 5; i++ ){
        if ( phils[ i ].id == id ) return i;
    }
    
    return -1;
}

void hungry_philo( int id, int i ){
    phils[ i ].hungry = 1;
    write(0, "Philosopher: ", 13); write_numb(i+1); write(0, " is hungry!!\n", 13);
}


void extract_data( char* data, int i ){ // in here also do the has_right and has_left
   // return -1; // nothing was transmitted
}

void get_right( int id, int i ){
    write(0, "Philosopher: ", 13); write_numb(i+1); write(0, " wants right chopstick!!\n", 26);
    // if right chopstick free => ask the other person through channel .. send request get it else wait 
    int free_chop = 0; 
    while ( !free_chop ){
        char* data =  read_channel( phils[ i ].chan_right );//read channel to the right 
        extract_data( data, i );
        if ( phils[i].has_right == 0 ) free_chop = 1;
    }
    //pick up chopstick
    phils[i].has_right = 1;
    //send got chopstick
    write(0, "Philosopher: ", 13); write_numb(i+1); write(0, " has right chopstick!!\n", 24);
    write_channel( phils[ i ].chan_right, "Ltaken\n"); // left taken is the msg 
}

void get_left( int id, int i ){
    write(0, "Philosopher: ", 13); write_numb(i+1); write(0, " wants left chopstick!!\n", 26);
    // if right chopstick free => ask the other person through channel .. send request get it else wait 
    int free_chop = 0; 
    while ( !free_chop ){
        char* data =  read_channel( phils[ i ].chan_left );//read channel  
        extract_data( data, i );
        if ( phils[i].has_left == 0 ) free_chop = 1;
    }
    //pick up chopstick
    phils[i].has_left = 1;
    //send got chopstick
    write(0, "Philosopher: ", 13); write_numb(i+1); write(0, " has left chopstick!!\n", 24);
    write_channel( phils[ i ].chan_left, "Rtaken\n"); // right taken is the msg 
}

//need to check some stuff about the strings I send 
void right_put( int id, int i){
    phils[i].left_chopstick = 0;
    write(0, "Philosopher: ", 13); write_numb(i+1); write(0, " right chopstick on table!!\n", 32);
    //send msg through channel
    write_channel( phils[ i ].chan_right, "Lfree\n"); // left taken is the msg 
}

void left_put( int id, int i){
    phils[i].right_chopstick = 0;
    write(0, "Philosopher: ", 13); write_numb( i+1 ); write(0, " left chopstick on table!!\n", 32);
    //send msg through channel 
    write_channel( phils[ i ].chan_left, "Rfree\n"); // right taken is the msg 
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
        eat( id, i );                  // this has a timer of some sorts
        right_put( id, i );
        left_put( id , i);
    }
    write(0, "Philosopher: ", 13); write_numb(i+1); write(0, " eating procedure success!!\n", 32);
}


void init_phils( int philo_1, int philo_2, int philo_3, int philo_4, int philo_5, int chan1_2, int chan2_3, int chan3_4, int chan4_5, int chan5_1){
    phils[0].id              = philo_1;
    phils[1].id              = philo_2;
    phils[2].id              = philo_3;
    phils[3].id              = philo_4;
    phils[4].id              = philo_5;
    
    phils[0].chan_right      = chan1_2;
    phils[1].chan_right      = chan2_3;
    phils[2].chan_right      = chan3_4;
    phils[3].chan_right      = chan4_5;
    phils[4].chan_right      = chan5_1;
    
    phils[0].chan_left       = chan5_1;   
    phils[1].chan_left       = chan1_2;
    phils[2].chan_left       = chan2_3;
    phils[3].chan_left       = chan3_4;
    phils[4].chan_left       = chan4_5;

    for ( int i = 0; i<5; i++ ){
        int right_chopstick = i + 1;
        int left_chopstick  = i - 1;
        
        if ( left_chopstick == -1 )left_chopstick = 5;  
        
        phils[i].hungry          = 0;
        phils[i].has_right       = 0;
        phils[i].has_left        = 0; 
        phils[i].left_chopstick  = left_chopstick;
        phils[i].right_chopstick = right_chopstick;
    }
}

void Table() {
    // Need some sort of clever way to think about this 
    // Forking the processes to have 5 philosophers
    if ( before_forks != -1 ){
        philo[0] = fork();
    }
    if (before_forks != -1){
        philo[1] = fork(); 
    }  
    if (before_forks != -1){
        philo[2] = fork(); 
    }
    if (before_forks != -1){
        philo[3] = fork(); 
    }
    if (before_forks != -1){
        philo[4] = fork(); 
    }


    // Setting up channels between philosophers so that they can communicate data 
    if (before_forks != -1) { 
        chan1_2 = create_channel( philo[0], philo[1]);
        chan2_3 = create_channel( philo[1], philo[2]);
        chan3_4 = create_channel( philo[2], philo[3]);
        chan4_5 = create_channel( philo[3], philo[4]);
        chan5_1 = create_channel( philo[4], philo[5]);
    }

    if ( before_forks != -1 ) init_phils( philo[0], philo[1], philo[2], philo[3], philo[4], chan1_2, chan2_3, chan3_4, chan4_5, chan5_1);

    // Stop it from froking again 
    before_forks = -1;  
   
    // Give ctrl to the first guest 
    if ( ctrl == -1){
        ctrl = yield( philo[0] ); 
    }
    // Seat the guests at the table and start 
    guest( philo[0] );
    // exit + give control to other process

    //termination msg before_forks exiting the final philosopher process
    write(0, "Philosophical dinner has terminated\n", 36);
    // ctrl should now be given back to the terminal 
}

void (*entry_Philosophers)() = &Table;

/* Logic on channels -- specific to this problem: 
* channels to write to must be:  
* channels to read from must be: 
*/


// unable the timer for this and then enable it again when the whole thing is done