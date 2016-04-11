#include "Philosophers.h"
#define PHILOS 5
#define DELAY 5000
#define FOOD 50

void grab_chopstick( int phil, int c, char* hand );
void down_chopsticks( int c1, int c2 );
int food_on_table();
int sleep_seconds = 0;

void Philosophers(){

}

void* philosophers( void *num) {
	int id; 
	int i, left_chopstick, right_chopstick, f;

	id = ( int ) num;
	write(0, "Philosopher ", 12); write_numb( id ); write(0, " is done thinking and now ready to eat.\n", 42);
	right_chopstick = id;
	left_chopstick 	= id+1;

	/* Wrap around the chopsticks. */
    if (left_chopstick == PHILOS)
        left_chopstick = 0;

    while (f = food_on_table ()) {

        /* Thanks to philosophers #1 who would like to take a nap
         * before picking up the chopsticks, the other philosophers
         * may be able to eat their dishes and not deadlock.  
         */
        //if (id == 1) sleep (sleep_seconds);

        grab_chopstick (id, right_chopstick, "right ");
        grab_chopstick (id, left_chopstick, "left");

        write (0, "Philosopher ", 12); write_numb( id ); write(0, ": eating.\n", 10);
       // usleep (DELAY * (FOOD - f + 1));
        down_chopsticks (left_chopstick, right_chopstick);
    }

    write (0, "Philosopher ", 12); write_numb( id ); write(0, " is done eating.\n", 18);
	return (NULL);
} 

int food_on_table() {
	static int food = FOOD;
	int myfood; 

	//..

	return myfood;
}

void grab_chopstick( int phil, int c, char* hand ){
	// ..
	write(0, "Philosopher ", 12);write_numb( phil ); write(0, " got ", 5); write(0, hand, sizeof(hand)); write(0, " chopstick ", 11); write_numb( c );write(0, "\n", 1);
}

void down_chopsticks( int c1, int c2 ){

}

//void (*entry_Philosophers)() = &dphilosophers;