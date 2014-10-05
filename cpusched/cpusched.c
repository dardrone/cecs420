/*
 ============================================================================
Name	     : proj1.c
 Author      : Darren Jennings
 Version     :
 Copyright   : Copyright 2014
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "dbg.h"
#include "list.h"
#include "list.c"

typedef struct process {
  unsigned int pid;
  unsigned int arrival_time;
  unsigned int burst_time;
} process;

process * create_process(int, int, int);
void free_process(process *);

process * create_process(int id, int arrival_time, int burst)
{
  process* ps_ptr = (process *) malloc(sizeof(process));
  ps_ptr->pid = id;
  ps_ptr->arrival_time = arrival_time;
  ps_ptr->burst_time = burst;
  return ps_ptr;
}

void free_process(process* process)
{
  free(process);
}

int main(int argc, char *argv[]) {

	//////////////////
		//PRINT ARGS
		//////////////////
		int i=0;
		while(i < argc) {
			printf("\narg[%d]: %s\n", i, argv[i]);
			i++;
		}

	////////////////////////////////////
	//INITIALIZE FILES FROM ARGUMENTS
	////////////////////////////////////

		char *inputfilename = argv[1];
		FILE *inputFILE;
		inputFILE = fopen(inputfilename,"r");

		char *outputfilename = argv[2];
		FILE *outputFILE;
		outputFILE = fopen(outputfilename,"w");

		char *algorithm = argv[3];
		int limit = 0;

		if(argv[4] == NULL){
			debug("Limit is infinite!!!");
		}else{
			char *ptr;
			limit = strtol(argv[4], &ptr, 10);
		}

		//Check that they exist
		if (inputFILE == NULL || algorithm == NULL) {
		  fprintf(stderr, "Can't open the files, and algorithm not specified. please check your argument list...\n");
		  exit(1);
		}

		//initialize the list of processes
		List *pslist = List_create();

		//scan the file
		char *id = malloc(21 * sizeof(id));
		char *arrivalt = malloc(21 * sizeof(arrivalt));
		char *burst = malloc(21 * sizeof(burst));
		int lineCount = 0;
		if(limit>0){
			while(fscanf(inputFILE, "%s %s %s", id,arrivalt,burst) != EOF && lineCount != limit) {
				debug("%s, %s, %s",id,arrivalt,burst);
				lineCount++;

				process *ps = create_process(strtol(id, &id, 10),strtol(arrivalt, &arrivalt, 10),strtol(burst, &burst, 10));

				List_push()
				//List_push(list2,);
				//free(wordToCopy);
			}
		}

		/////////////////////////////////
		//
		// Free memory stuff
		//
		/////////////////////////////////
		fclose(inputFILE);
		fclose(outputFILE);
		debug("Finished....");
	return 0;
}
