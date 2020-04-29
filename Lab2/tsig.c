#include "stdio.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "stdlib.h"
#include <unistd.h>

//#define WITH_SIGNALS 

// Global flag for determining whether the interrupt signal has been sent.
int interrupt = 0;

#ifdef WITH_SIGNALS
// Empty function used for ignoring all the signals
static void ignore(int bar) 
{
}

// Custom handler for the interruption signal. Used in parent process
static void myHandlerInt(int bar)
{
	if(interrupt == 0)
		printf("received the interrupt signal \n\n");
	interrupt = 1;
}

// Custom handler for the interruption signal. Used in child processes
static void myHandlerTerm(int bar)
{
	interrupt = 1;
	printf("child[%d] has been terminated by the parent[%d]\n", getpid(), getppid());
}
#endif

// Specyfying child process behaviour
void child_process()
{
	#ifdef WITH_SIGNALS
	// Ignoring SIGINT
	signal(SIGINT, ignore);
	// Custom handler for the SIGTERM
	signal(SIGTERM, myHandlerTerm);
	#endif
	printf("child[%d]: PPID=%d \n", getpid(), getppid());
	sleep(10); 
	printf("child[%d]: Processhas been completed \n", getpid());
}

// Specyfying parent process behaviour
void parent_process(int *terminations)
{
	pid_t waitpid;
	waitpid = wait(NULL); // Wait for child processes to terminate
	if(waitpid > 0) // If child process has been encountered
	{
		printf("parent[%d]: End of child[%d] process. \n", getpid(), waitpid);
		++ *terminations; // Increment number of terminated child processes
	}
}


int main(void)
{
	#ifndef WITH_SIGNALS
	printf("Function without signals\n");
	#endif
	
	#ifdef WITH_SIGNALS
	printf("Function with signals\n");
	// Force ignoring of all signals
	for(int i=1; i <= NSIG; i++)
		signal(i, ignore);
	// Restoring handler for the SIGCHLD
	signal(SIGCHLD, SIG_DFL);
	// Custom signal handler for the SIGINT
	signal(SIGINT, myHandlerInt);
	#endif
	
	const int NUM_CHILD = 5; // Number of child processes
	pid_t childProcess;  // PID of fork-created process
	pid_t pidTable[NUM_CHILD]; // Table of created processes' PIDs
	int numOfTerminations = 0; // Number of terminated child processes
	int *numOfTerminationsPtr = &numOfTerminations; //Pointer to number of terminated child processes
	
	// Creating child processes
	int i = 0;
	do
	{
		childProcess = fork(); // Create a new child process and store it's ID
		if(childProcess == 0) // If encountered a child process
			{
				child_process();// Assign the child's behaviour
			}
		sleep(1); 
		
		#ifdef WITH_SIGNALS
		if(interrupt == 1 && childProcess > 0) // Interrupt signal has been sent to the parrent process
		{
			printf("parent[%d]: Creation process has been interrupted\n", getpid());
			for(int j = 0; j < i; j++) // Terminate all the already created processes
			{
				kill(pidTable[j], SIGTERM);
			}
			kill(childProcess, SIGTERM); // Terminate the process which creation has been interrupted
		}
		// Process hasn't been interrupted
		else if( interrupt ==0 )
		{
		#endif 
			//it's a parent process
			if(childProcess > 0)
			{
				printf("parent[%d]: child[%d] has been created \n",getpid(), childProcess );
				pidTable[i] = childProcess; // Add child to the table of created processes
			}
			//child proces not created
			else if(childProcess<0)
			{
				fprintf( stderr, "parent[%d]: Unable to create a child process. \n", getpid());
				for(int j = 0; j < i; j++)
				{
					kill(pidTable[j], SIGTERM); // Terminate all the previous processes
				}
				return 1;
			}
		#ifdef WITH_SIGNALS
		}
		#endif
		
		i++;
	} while(childProcess> 0 && i < NUM_CHILD && interrupt == 0);

	// Last part of the program, kind of a recap
	if(childProcess> 0)
	{
		if(interrupt == 0)
		printf("parent[%d]: All processes have been created \n\n", getpid());
		// wait()'s loop
		for(int i = 0; i < NUM_CHILD; ++i)
		{
			parent_process(numOfTerminationsPtr);
		}
		if(numOfTerminations == NUM_CHILD)
			printf("parent[%d]: No more child processes\n", getpid());
		printf("parent[%d]: Number of terminated kids: %d\n", getpid(), numOfTerminations);
	}
			
}
