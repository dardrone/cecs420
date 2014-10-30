/*
 ============================================================================
 Name        : spksp.c
 Author      : Darren Jennings
 Version     :
 Copyright   : No copyright
 Description : spksp in C
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "dbg.h"
#include "list.h"
#include "list.c"
#include <pthread.h>
#include <semaphore.h>

#define MAXLINESIZE 1024

// Bounded buffer list to be used by all threads.
List *boundedBuffer;

//Semaphores
sem_t empty;
sem_t full;
sem_t mutex;

typedef struct searchCommand {
	char* keyword; // single word to search for. Doesn't include white space <SPACE>, <TAB>, or <NEWLINE>.
	char* directoryPath; // Full path e.g. /home/naltipar/dirOne
} searchCommand;

typedef struct item {
	char* filename; // single word to search for. Doesn't include white space <SPACE>, <TAB>, or <NEWLINE>.
	char* matchLineNumber; // Full path e.g. /home/naltipar/dirOne
	char* line;
} item;

void List_print_searchCommands(List *list){
	LIST_FOREACH(list, first, next, cur){
	if(cur){
			searchCommand *sc = cur->value;
			debug("%s, %s",sc->keyword,sc->directoryPath);
		}
	}
}

searchCommand * create_SearchCommand(char *, char *);
void free_searchCommand(searchCommand *);
searchCommand * create_SearchCommand(char *kw, char *path)
{
	searchCommand* sc_ptr = (searchCommand *) malloc(sizeof(searchCommand));
	sc_ptr->keyword = strdup(kw);
	sc_ptr->directoryPath = strdup(path);
    return sc_ptr;
}

int main(int argc, char *argv[]) {

	char *commandFile = argv[1];
	FILE *cmdFile;
	cmdFile = fopen(commandFile,"r");

	// initialize the bounded buffer list
	boundedBuffer = List_create();

	// initialize the list of processes
	List *searchCommandList = List_create();

    sem_init(&empty, 0, max); // max are empty
	sem_init(&full, 0, 0);    // 0 are full
	sem_init(&mutex, 0, 1);   // mutex

	// Get the buffer size
	int bufferSize = -1;
	if(argv[2] == NULL || argv[2] <= 0){
		debug("Check format. Should be \":~$ ./spksp commandFile bufferSize\"");
		return exit;
	}else{
		bufferSize = atoi(argv[2]);
		debug("BufferSize: %d", bufferSize);
	}
	debug("MAXLINESIZE: %d", MAXLINESIZE);

	//scan the file
	char *keywrd = malloc(24 * sizeof(keywrd));
	char *path = malloc(24 * sizeof(path));

	while(fscanf(cmdFile, "%s %s", keywrd, path) != EOF){
		searchCommand *sc = create_SearchCommand(keywrd, path);
		List_push(searchCommandList,sc);
	}

	List_print_searchCommands(searchCommandList);

	//DESTROY SHIT
	List_clear_destroy(searchCommandList);
	free(keywrd);
	free(path);
	fclose(cmdFile);

	debug("Finished!");
	return 0;
}
