#include "stdio.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "stdlib.h"
#include <unistd.h>

//#define WITH_SIGNALS 

#ifndef WITH_SIGNALS
void child_process()
{
	printf("PPID=%d \n", getppid());
	sleep(10); //change later for 10 
	printf("Process %d has been completed \n", getpid());
}

void parent_process(int *terminations)
{
	pid_t waitpid;
	waitpid = wait(NULL);
	printf("End of child processes. \n");
	if(waitpid > 0)
		++ *terminations;
}


int main(void)
{
	printf("Function without signals\n");
	const int NUM_CHILD = 5;
	pid_t childProcess; 
	pid_t pidTable[5];
	int numOfTerminations = 0;
	int *numOfTerminationsPtr = &numOfTerminations;
	
	//Creating chidl processes
	int i = 0;
	do
	{
		childProcess = fork();
		sleep(1);

		//child process encountered
		if(childProcess == 0)
		{
			child_process();
		}
		//it's a parent process
		else if(childProcess > 0)
		{
			printf("Child process %d has been created \n", childProcess );
			pidTable[i] = childProcess;
		}
		//child proces not created
		else
		{
			perror("Unable to create a child process. \n");
			for(int j = 0; j <= i; j++)
				kill(pidTable[j], 15);
			exit(1);
		}
		
		i++;
	} while(childProcess> 0 && i < NUM_CHILD);
	
	
	if(childProcess> 0)
	{
		printf("All processes have been created \n");
		for(int i = 0; i < NUM_CHILD; ++i)
		{
			parent_process(numOfTerminationsPtr);
		}
		if(numOfTerminations == NUM_CHILD)
			printf("No more child processes\n");
		printf("Number of terminated kids: %d\n", numOfTerminations);
	}
		
	
	return EXIT_SUCCESS;
}
#endif

#ifdef WITH_SIGNALS

int interrupt = 0;

static void ignore(int bar)
{
}

static void myHandlerInt(int bar)
{
	if(interrupt == 0)
		printf("received the interrupt signal \n");
	interrupt = 1;
}

static void myHandlerTerm(int bar)
{
	interrupt = 1;
	printf("child[%d] has been terminated by the parent[%d]\n", getpid(), getppid());
}

void child_process()
{
	// Ignoring SIGINT
	signal(SIGINT, ignore);
	// Custom handler for the SIGTERM
	signal(SIGTERM, myHandlerTerm);
	if(interrupt == 0)
		printf("child[%d]: PPID=%d \n", getpid(), getppid());
	if(interrupt == 0)
		sleep(10); //change later for 10 
	if(interrupt == 0)
		printf("child[%d]: Processhas been completed \n", getpid());
	if(interrupt == 1)
		printf("child[%d]: Process has been interrupted \n", getpid());
}

void parent_process(int *terminations)
{
	pid_t waitpid;
	waitpid = wait(NULL);
	if(waitpid > 0)
	{
		printf("parent[%d]: End of child[%d] process. \n", getpid(), waitpid);
		++ *terminations;
	}
}


int main(void)
{
	printf("Function with signals\n");
	// Force ignoring of all signals
	for(int i=1; i <= 31; i++)
		signal(i, ignore);
	// Restoring handler for the SIGCHLD
	signal(SIGCHLD, SIG_DFL);
	// Custom signal handler for the SIGINT
	signal(SIGINT, myHandlerInt);
	
	const int NUM_CHILD = 5;
	pid_t childProcess; 
	pid_t pidTable[NUM_CHILD];
	int numOfTerminations = 0;
	int *numOfTerminationsPtr = &numOfTerminations;
	int fail = 0;
	
	//Creating child processes
	int i = 0;
	do
	{
		childProcess = fork();
		sleep(1);
	
		if(interrupt == 1)
		{
			if(childProcess == 0)
			{
				child_process();
			}
			else if(childProcess > 0)
			{
				printf("parent[%d]: Creation process of child[%d] has been interrupted\n", getpid(), childProcess);
				for(int j = 0; j < i; j++)
				{
					kill(pidTable[j], 15);
				}
				
				kill(childProcess, 15);
			}
		}
		
		else
		{
			//child process encountered
			if(childProcess == 0)
			{
				child_process();
			}
			//it's a parent process
			else if(childProcess > 0)
			{
				printf("parent[%d]: child[%d] has been created \n",getpid(), childProcess );
				pidTable[i] = childProcess;
			}
			//child proces not created
			else
			{
				fprintf( stderr, "parent[%d]: Unable to create a child process. \n", getpid());
				for(int j = 0; j < i; j++)
				{
					kill(pidTable[j], 15);
				}
				return 1;
			}
		}
		
		i++;
	} while(childProcess> 0 && i < NUM_CHILD && interrupt == 0);

	
	if(childProcess> 0)
	{
		if(interrupt == 0)
		printf("parent[%d]: All processes have been created \n", getpid());
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

#endif