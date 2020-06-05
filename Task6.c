#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void printHelp();

int main (int argc, char **argv)
{

    bool map = false; //"-m" flag

    char *file_from;
    char *file_to;

    int fd_from;
    int fd_to;

      
    char opt; 
   	while((opt= getopt(argc, argv, "hm")) != -1)
   	{
		switch(opt){
			case 'h':
				printHelp();
				return 0;
			case 'm':
				map = true;
				break;
			case '?':
				fprintf(stderr, "Unknown option -%c", optopt);
				return 1;
			default:
				fprintf(stderr, "getopt() error, exiting.\n");
				return 1;
		}
	}


	// Check user input 
	if((map && argc > 4) || (!map && argc > 3))
	{
		fprintf(stderr, "Too many arguments!\n");
		return 1;
	}
    else if((map && argc < 4) || (!map && argc < 3))
	{
		fprintf(stderr, "Too little arguments!\n");
		return 1;
	}

	// Input correct
	if(map)
	{
		file_from = argv[2];
		file_to = argv[3];
	}
	else
	{
		file_from = argv[1];
		file_to = argv[2];
	}

	
	// Opening files

	// Opening the "copy from" file
	fd_from = open(file_from, O_RDONLY);
	struct stat stats;
	if(fstat(fd_from, &stats) == -1) 
	{
		perror("fstat error\n");
		exit(1);
	}
	size_t size = (size_t)stats.st_size;

	// Opening the "copy to" file
	fd_to = open(file_to, O_RDWR | O_CREAT, S_IRWXU);
	if(fd_to < 0 || fd_to < 0)
	{
		fprintf(stderr, "Couldn't read file. Try again later\n");
		return 1;
	}



/*---------COPYING SCETION*------*/

	// copying using mmap
	if(map)
	{
		char* src; // mapping virtual address space of the source file
		char* dst; // mapping virtual address space of the dest file

		src = mmap(NULL, size, PROT_READ, MAP_SHARED, fd_from, 0);
		if(src == (void*) -1) 
		{
			perror("mmap in error\n");
			exit(1);
		}
		
		// truncate destination file to the source file size
		if(ftruncate(fd_to, size))
		{
			perror("ftruncate error\n");
			exit(1);
		}


		dst = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_to, 0);
		if(dst == (void*) -1) {
			perror("mmap out error\n");
			exit(1);
		}

		// copy to the destination
		dst = memcpy(dst, src, size);
		if(dst == (void*) -1) {
			perror("mcpy error\n");
			exit(1);
		}

	}

	// copying using read()/write() functions
	else
	{
		printf("%ld", size);
		char* buffer = (char*) malloc(size*sizeof(char)); // dynamic memmory allocation 

		int bytes_read;
		do
		{
			bytes_read = read(fd_from, buffer, size);
			if(bytes_read == -1) 
			{
				perror("read error\n");
				exit(1);
			}
		
			if(write(fd_to, buffer, bytes_read) == -1) 
			{
				perror("write error\n");
				exit(1);
			}
		}while(bytes_read != 0);

		free(buffer); // freeing dynamically allocated resources
	}



	// Closing files
	close(fd_from);
	close(fd_to);

    return 0; 
} 


void printHelp()
{
	printf("Usage: copy [OPTION]... [SOURCE FILE]... [DESTINATION FILE]... \n");
	printf("Copy source file to the destination file\n");
	printf("-h                   show help\n");
	printf("-m                   use mmap option\n");
	printf("no option            use read/write functions\n");
	printf("Examples:\n");
	printf("copy -m abc.txt def.txt\n");
	printf("copy abc.txt def.txt\n");
}