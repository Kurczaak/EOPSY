#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include "stdlib.h"
#include "sys/wait.h"

//--------KEYS-----------
#define KEY_CUST 0x1111
#define KEY_ACCESS_WR_SEATS 0x1122
#define KEY_BARBER 0x1133
#define KEY_MEM 0x1144 // shared memory key

//----------Waiting Room and Clients-----------
#define M 10 // number of seats in the waiting room
#define NC 10 //number of clients

//------Behavior-------
void barber(int _barberReady, int _custReady, int _accessWRseats);
void customer(int _barberReady, int _custReady, int _accessWRseats);

//-------Shared Memory---------
int * create_shared_memory(); // create shared memory instance neat and clean
int * get_shared_memory(); // get the shared memory

//------Semaphores------------
struct Semaphores
{
	int barberReady;
	int custReady;
	int accessWRseats;
};
struct Semaphores create_semaphores(int _keyBarber,int _keyCustomer,int _keyAccessWRSeats); // function handling neat Semaphore structure initialization
// operations on semaphores
void sem_call(int sem_id, int op); // auxilary function for neat implementation of P(S) & V(S)
void P (int sem_id); // wait/down semaphore function
void V (int sem_id); // signal/up semaphore function

void main ()
{
    // variables for storing process IDs
    pid_t barberPID, customerPID;
    pid_t pidTable[NC + 1]; //NC clients + 1 barber

	// create shared memory variable (number of seats)
    int* shared = create_shared_memory(M);

    // create semaphores
    struct Semaphores semaphores= create_semaphores(KEY_BARBER, KEY_CUST, KEY_ACCESS_WR_SEATS);

    // create barber
	barberPID = fork(); // Create a new child process and store it's ID
	if(barberPID== 0) // If encountered a child process
		{
			barber(semaphores.barberReady, semaphores.custReady, semaphores.accessWRseats);// Assign the child's behaviour
		}
	else if(barberPID > 0)
		pidTable[0]=barberPID;

	// create cusmoters

	int i = 0;
	if(barberPID>0)
	do
	{
		customerPID = fork();
		if(customerPID == 0) //encountered a child process
		{
			customer(semaphores.barberReady, semaphores.custReady, semaphores.accessWRseats);
		}
		else if(customerPID>0)
			pidTable[i+1]=customerPID;
		else
			{
				fprintf( stderr, "parent[%d]: Unable to create a child process. \n", getpid());
				for(int j = 0; j < i; j++)
				{
					kill(pidTable[j], SIGTERM); // Terminate all the previous processes
				}
			}
			i++;
	}while(customerPID > 0 && i < NC);
	if(barberPID > 0 && customerPID > 0)
		for(int i = 0; i < NC +1; ++i)
			wait(NULL);

}//----------------------END OF MAIN--------------------------

struct Semaphores create_semaphores(int _keyBarber,int _keyCustomer,int _keyAccessWRSeats)
{
	struct Semaphores semaphores;
	semaphores.barberReady = semget(_keyBarber, 1, 0660 | IPC_CREAT);
	semaphores.custReady = semget(_keyCustomer, 1, 0660 | IPC_CREAT );
	semaphores.accessWRseats = semget(_keyAccessWRSeats, 1, 0660 | IPC_CREAT );

	if(semaphores.custReady < 0 || semaphores.barberReady < 0 || semaphores.accessWRseats < 0)
        printf("semget error\n");

	 union semun 
	 {
		int              val;    
		struct semid_ds *buf;    
		unsigned short  *array;  
      };
      union semun arg;

    // waiting room mutex
    arg.val = 1;
    if(semctl(semaphores.accessWRseats, 0, SETVAL, arg) < 0)
        printf("semctl error for waiting room mutex");

    // barber
    arg.val = 0;
    if(semctl(semaphores.barberReady, 0, SETVAL, arg) < 0)
        printf("semctl error for waiting room mutex");

    //customer
    arg.val = 0;
    if(semctl(semaphores.custReady, 0, SETVAL, arg) < 0)
        printf("semctl error for waiting room mutex");

    return semaphores;
}

int * create_shared_memory(int seats)
{
	// create shared memory
    int shmid = shmget(KEY_MEM, sizeof(int), 0660 | IPC_CREAT);
    if(shmid < 0)
        printf("shmget error");

    // get shared memory
    int* shared = shmat(shmid, NULL, 0);

    if(shared == (void*) -1)
        printf("shmat error");
    // initialize shared variable
    *shared = seats;
	return shared;
}


int * get_shared_memory()
{
	int shmid = shmget(KEY_MEM, sizeof(int), 0660);
	if(shmid < 0)
        printf("shmget error when using memory_attach()");
    int *shared;
    shared = shmat(shmid, NULL, 0);
    if(shared == (void*) -1)
    	printf("shmat erro\n");
    return shared;
}

void customer(int _barberReady, int _custReady, int _accessWRseats)
{
	int *_WRseats = get_shared_memory();
	printf("[Customer %d]: Czekam na dostep do pamieci\n", getpid());
	P(_accessWRseats);
	printf("[Customer %d] Dostalem dostep do pamieci\n", getpid());
	if(*_WRseats > 0)
	{
		printf("[Customer %d] Sa wolne miejsca\n", getpid());
		*_WRseats = *_WRseats - 1;
		printf("[Customer %d] Zajmuje jedno miejsce %d\n", getpid(),* _WRseats);

		V(_custReady);
		printf("[Customer %d] Jestem gotowy\n", getpid());
		V(_accessWRseats);
		printf("[Customer %d] Oddalem dostep do pamieci\n", getpid());
		P(_barberReady);
		printf("[Customer %d] Jestem Ciety\n", getpid());
	}
	else
		V(_accessWRseats);
}

void barber(int _barberReady, int _custReady, int _accessWRseats)
{
	int *_WRseats = get_shared_memory();
	for(int i=0; i<NC; ++i)
	{
		printf("[Barber] Czekam na klienta\n");
		P(_custReady);
		printf("[Barber] Klient gotowy\n");
		P(_accessWRseats);
		printf("[Barber] Czekam na dostep do pamieci\n");
		*_WRseats = (*_WRseats + 1);
		printf("[Barber] Zwalniam miejsce %d\n", *_WRseats);
		V(_barberReady);
		printf("[Barber] Jestem gotowy\n");
		V(_accessWRseats);
		printf("[Barber] Tne klienta\n");
		sleep(1);
	}
	printf("Koniec pracy!\n");
	// remove semaphores
	semctl(_barberReady, 0, IPC_RMID);
    semctl(_custReady, 0, IPC_RMID);
    semctl(_accessWRseats, 0, IPC_RMID);

    // get shared memory id
    int shmid = shmget(KEY_MEM, sizeof(int), 0660);
    // remove memory
    if(shmctl(shmid, IPC_RMID, 0) < 0)
        printf("shmctl error\n");
}

void sem_call(int sem_id, int op)
{
	struct sembuf sb;

	sb.sem_num = 0;
	sb.sem_op = op; //operation to be perfomed on a semaphore (+1/-1)
	sb.sem_flg = 0;
	if(semop (sem_id, &sb, 1) == -1)
	{
		printf("Blad w operacji\n");
		int errnum = errno;
      fprintf(stderr, "Value of errno: %d\n", errno);

		printf("sem Id = %d", sem_id);
		printf("%d", op);
	}
}

//Wait/ Down
void P (int sem_id)
{
	sem_call(sem_id, -1);
}

//Signal/Up
void V (int sem_id)
{
	sem_call(sem_id, 1);
}
