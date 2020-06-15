 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "stdlib.h"
#include "sys/wait.h"
#include <stdbool.h>

#define N 5 // Number of philosophers
#define LEFT ( i + N - 1 ) % N // Index of a filosopher on the left (from the perspective of the i'th philosopher)
#define RIGHT ( i + 1 ) % N // Index of a filosopher on the right (from the perspective of the i'th philosopher)

// States of the philosophers
#define THINKING 0 
#define HUNGRY 1
#define EATING 2
int state[N]; //initiated to THINKING

// Times
#define THINKING_TIME 2
#define EATING_TIME 1
#define SIMULATION_TIME 10

// Mutexes
pthread_mutex_t  m = PTHREAD_MUTEX_INITIALIZER; // Mutex for state of philosophers
pthread_mutex_t s[N] = PTHREAD_MUTEX_INITIALIZER; // Table of mutexes for meal access of a given philosopher

// Function declarations
void grab_forks( int i );
void put_away_forks( int i );
void test( int i );
void *philosopher(void *x_void_ptr); // Function describing behaviour of a philosopher

void main()
{
	// Lock the access to the meals for all of the philosophers
	for(int i = 0; i < N; ++i)
		pthread_mutex_lock(&s[i]);

	// Unlock the access to the state of philosophers
	pthread_mutex_unlock(&m);

	//-----------Philosopher Threads------------

	pthread_t threads[N]; // Reference to the philosophers threads
	int index[]={1,2,3,4,5}; // Table of philosopher's ids
	for(int i=0; i < N; ++i)
		if(pthread_create(threads + i, NULL, philosopher, &index[i])) 
			fprintf(stderr, "Error creating thread\n");

	sleep(SIMULATION_TIME); // Wait till simulation finishes

	// Send cancellation requests
	for(int i=0; i<N; i++)
	{
		//pthread_kill(threads[i], 9);
		pthread_cancel(threads[i]);
	}
}
 

void grab_forks( int i ) 
{
  pthread_mutex_lock(&m); // Ask for access to the philosopher's information
  /* mutex m critical section starts here*/
  printf("[Philosopher %d]: I'm hungry' \n", i);
  state[i] = HUNGRY;   
  test(i);  // check if neighbours are currently eating
  /* mutex m critical section ends here*/
  pthread_mutex_unlock(&m);  
  /* Mutex s[i] critical section ends here */ 
  pthread_mutex_lock( &s[i] );
  printf("[Philosopher %d]: I've stopped eating \n", i); 
}

 

void put_away_forks( int i ) {  

   pthread_mutex_lock( &m );  
    /* mutex m critical section starts here*/ 
   printf("[Philosopher %d]: I'm putting away forks \n", i);
   state[i] = THINKING;   // Start thinking
   test( LEFT );   // Check your left neighbour
   test( RIGHT );  // Check your right neighbour
    /* mutex m critical section ends here*/
   pthread_mutex_unlock( &m ); 

}

 

void test( int i ) // Check if I'm hungry and my negihbours are currently eating - do I have 2 forks?
{
   if( state[i] == HUNGRY   && state[LEFT] != EATING   && state[RIGHT] != EATING )  
   {
      state[i] = EATING;   
      pthread_mutex_unlock(&s[i]);
      /* Mutex s[i] critical section starts here */ 
    }
 }

 void *philosopher(void *x_void_ptr)
{
	int *x_ptr = (int *)x_void_ptr; // Id of a philosopher
	//printf("--------------------------------------%d----------------------\n", *x_ptr); //debuging purposes
	while(true)
	{
		// Think
		printf("[Philosopher %d]: I'm thinking \n", *x_ptr);
		sleep(THINKING_TIME);
		printf("[Philosopher %d]: I would like to grab forks \n", *x_ptr);
		// Eat
		grab_forks(*x_ptr);
		printf("[Philosopher %d]: I'm eating \n", *x_ptr);
  		sleep(EATING_TIME);
  		// Stop Eating
		put_away_forks(*x_ptr);
	}
	return NULL;
}