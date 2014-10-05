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

typedef enum {
  FCFS,
  SRTF
}algorithm;

typedef struct process {
  unsigned int pid; // positive integer process id
  unsigned int arrival_time; //in milliseconds from 0
  unsigned int burst_time; //in milliseconds
  unsigned int finish_time; //in milliseconds
  unsigned int waiting_time; //in milliseconds
} process;

process * create_process(int, int, int, int, int);
void free_process(process *);

process * create_process(int id, int arrival_time, int burst, int finish_time, int waiting_time)
{
  process* ps_ptr = (process *) malloc(sizeof(process));
  ps_ptr->pid = id;
  ps_ptr->arrival_time = arrival_time;
  ps_ptr->burst_time = burst;
  ps_ptr->finish_time = finish_time;
  ps_ptr->waiting_time = waiting_time;
  return ps_ptr;
}

void schedulePS(List *pslist, algorithm alg){
	if(alg == FCFS){
		debug("FCFS!!!");
		int time_elapsed = 0;
		LIST_FOREACH(pslist, first, next, cur){
			if(cur->prev == NULL){
				process *ps = cur->value;
				ps->finish_time = (ps->burst_time+ps->arrival_time);
				ps->waiting_time = ps->arrival_time;
				time_elapsed += ps->burst_time;
			}else{
				process *ps = cur->value;
				ps->finish_time = 0;
				ps->waiting_time = 0;
			}
		}
	}
	if(alg == SRTF){
		debug("SRTF!!!");
	}
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
			limit = strtol(argv[4], &argv[4], 10);
		}

		//Check that they exist
		if (inputFILE == NULL || algorithm == NULL || outputFILE == NULL) {
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
				process *ps = create_process(strtol(id, NULL, 10),strtol(arrivalt, NULL, 10),strtol(burst, NULL, 10),0,0);
				List_push(pslist,ps);

			}
		}

		if(strcmp(algorithm,"FCFS") == 0){
			schedulePS(pslist, FCFS);
		}
		else if(strcmp(algorithm, "SRTF") == 0){
			schedulePS(pslist,SRTF);
		}


		LIST_FOREACH(pslist, first, next, cur){
					if(cur->next){
						process *ps = cur->value;
						debug("pid: %d, arrival time:%d, cpu burst:%d, finish_time: %d, waiting_time: %d,",ps->pid,ps->arrival_time,ps->burst_time, ps->finish_time, ps->waiting_time);
						fprintf(outputFILE, "pid: %d, arrival time:%d, cpu burst:%d, finish_time: %d, waiting_time: %d, \n",ps->pid,ps->arrival_time,ps->burst_time, ps->finish_time, ps->waiting_time);
					}
					if(cur->next == NULL){
						process *ps = cur->value;
						debug("pid: %d, arrival time:%d, cpu burst:%d, finish_time: %d, waiting_time: %d, \n",ps->pid,ps->arrival_time,ps->burst_time, ps->finish_time, ps->waiting_time);
					}
				}

		/////////////////////////////////
		//
		// Free memory stuff
		//
		/////////////////////////////////

		debug("bout to free some shiz");
		free(id);
		free(arrivalt);
		free(burst);
		debug("bout to close input file");
		fclose(inputFILE);
		debug("bout to close output file");
		fclose(outputFILE);
		debug("bout to destroy");
		List_clear_destroy(pslist);
		debug("Finished....");
	return 0;
}
