/*
 ============================================================================
 Name        : spksp.c
 Author      : Darren Jennings
 Version     :
 Copyright   : No copyright
 Description : spksp in C
 ============================================================================
 */

#define MAXLINESIZE 1024
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include "dbg.h"
#include "list.c"
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include "mythreads.h"
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "spksp.h"


// Bounded buffer list to be used by all threads.
List *boundedBuffer;

int bufferSize;
int threadCount = 0;
int scCount = 0;

int fill = 0;

//Semaphores
sem_t empty;
sem_t full;
sem_t mutex;

//function prototypes
void do_fillBoundedBuffer(item *);
item * do_GetBoundedBuffer();
void * printBoundedBuffer(void *);
void * runSearchCommandForFile(void *);
bool has_txt_extension(char const *);

void List_print_searchCommands(List *list){
	LIST_FOREACH(list, first, next, cur){
	if(cur){
			searchCommand *sc = cur->value;
			debug("%s, %s, %s",sc->keyword,sc->directoryPath, sc->filename);
		}
	}
}

void List_print_items(List *list){
	LIST_FOREACH(list, first, next, cur){
	if(cur){
			item *itm = cur->value;
			printf("%s:%d:%s\n",itm->filename, itm->matchLineNumber, itm->line);
		}
	}
}

void free_searchCommand(searchCommand* delete_sc)
{
	//debug("delete_sc %s, %s, %s", delete_sc->directoryPath,delete_sc->filename,delete_sc->keyword);
	free(delete_sc->directoryPath);
	free(delete_sc->keyword);
	free(delete_sc->filename);

	free(delete_sc);
}

void free_item(item *itm)
{
  //debug("Freeing that shit");
  free(itm->filename);
  free(itm->line);
  free(itm);
}
//void runSearchCommandForFile(void *);

searchCommand * create_SearchCommand(char *kw, char *path, char *filename)
{
	searchCommand* sc_ptr = (searchCommand *) malloc(sizeof(searchCommand));
	sc_ptr->keyword = strdup(kw);
	sc_ptr->directoryPath = strdup(path);
	sc_ptr->filename = strdup(filename);
    return sc_ptr;
}

item * create_Item(char* filename, int matchLineNumber, char* lineItself)
{
	item* itm_ptr = (item *) malloc(sizeof(item));
	itm_ptr->filename = strdup(filename);
	itm_ptr->matchLineNumber = matchLineNumber;
	itm_ptr->line = strdup(lineItself);

    return itm_ptr;
}

bool has_txt_extension(char const *name)
{
    size_t len = strlen(name);
    ////debug("Text? Length: %d, %s",len, name+len - 4);
    return len > 4 && strcmp(name + len - 4, ".txt") == 0;
}



void do_fillBoundedBuffer(item *itm){

	List_push(boundedBuffer,itm);

	if(boundedBuffer->count == bufferSize+1){
		int k=0;
		for(k=0; k < boundedBuffer->count; k++){
			List_pop_SC(boundedBuffer);
		}
	}
}


item * do_GetBoundedBuffer() {
	item * itm;
	if(boundedBuffer->last){
		itm = boundedBuffer->last->value;
	}else{
		itm = boundedBuffer->first->value;
	}
	return itm;
}

void * printBoundedBuffer(void *arg) {

	debug("printerid: %d starting...", (*(int *)arg));
	sem_wait(&full);
	sem_wait(&mutex);
	item * tmp = do_GetBoundedBuffer();
	if (tmp != NULL){
				//PRINT THE LINE TO THE OUTPUT!!!
				printf("%s:%d:%s\n",tmp->filename, tmp->matchLineNumber, tmp->line);
			}
	List_pop(boundedBuffer);

	sem_post(&mutex);
	sem_post(&empty);
	debug("printerid: %d finished...", (*(int *)arg));
	return NULL;
}

//producer
void *runSearchCommandForFile(void *searchCmd){
	struct searchCommand *searchCommd = searchCmd;
	if(strstr(searchCommd->filename,".txt")){
		////debug("SEARCH COMMAND: keyword: %s, %s,%s ", searchCommd->keyword, searchCommd->directoryPath, searchCommd->filename);


		char* directoryPathcpy = strdup(searchCommd->directoryPath);
		strcat(directoryPathcpy,"/");
		char* filenamecpy = strdup(searchCommd->filename);
		char* keywordcpy = strdup(searchCommd->keyword);
		char* pathtoCopy = malloc(sizeof(directoryPathcpy)+sizeof(filenamecpy)+2);
		pathtoCopy = strcat(directoryPathcpy,filenamecpy);

		FILE *f = fopen(pathtoCopy, "r");
		char str[MAXLINESIZE];
		char fullString[MAXLINESIZE];
		int lineNumber = 0;

		if(f == NULL) {
			  //debug("Error opening file: %s", pathtoCopy);
			  exit(-1);
		}

		//go through the file and find the matches
		while(fgets(str, MAXLINESIZE, f) != NULL) {
			////debug("STR IS: %s", str);
			lineNumber++;
			char * pch;
			char * staticReferencePtr;
			int found = 0;

			//remove newline
			if( str[strlen(str)-1] == '\n' ){
				str[strlen(str)-1] = 0;
			}

			strcpy(fullString, str);

			pch = strtok_r(str," ", &staticReferencePtr);//, &staticReferencePtr);

			while (pch != NULL)
			{
				////debug("%s",pch);
				if(strcmp(pch,keywordcpy) == 0){

					item *itm = create_Item(filenamecpy,lineNumber,fullString);

					sem_wait(&empty);
					sem_wait(&mutex);

					do_fillBoundedBuffer(itm);

					sem_post(&mutex);
					sem_post(&full);
					found = 1;

				}
				if(found==1){
					break;
				}else{
					pch = strtok_r(NULL, " ",&staticReferencePtr);
				}
			}
		}
	}
	return NULL;
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		debug("Check format. Should be \":~$ ./spksp commandFile bufferSize\"");
	    exit(1);
	}
	//Get BufferSize from argument
	if(argv[2] == NULL || argv[2] <= 0){
		debug("Check format. Should be \":~$ ./spksp commandFile bufferSize\"");
		exit(1);
	}else{
		bufferSize = atoi(argv[2]);
		//debug("BufferSize: %d", bufferSize);
	}

	sem_init(&empty, 0, bufferSize); // buffer is empty
	sem_init(&full, 0, 0);    // 0 are full
	sem_init(&mutex, 0, 1);   // mutex

	char *commandFile = argv[1];
	FILE *cmdFile;
	cmdFile = fopen(commandFile,"r");

	// initialize the bounded buffer list
	boundedBuffer = List_create();

	// initialize the list of processes
	List *searchCommandList = List_create();

	//scan the file
	char *keywrd = malloc(24 * sizeof(keywrd));
	char *path = malloc(24 * sizeof(path));
	struct dirent * entry = malloc(24* sizeof(entry));

	int commandLineCount = 0;

	//Go through the commandfile to get all the search commands
	while(fscanf(cmdFile, "%s %s", path, keywrd) != EOF){
		commandLineCount++;
		char *pathtoCopy = strdup(path);
		char *keywrdToCopy = strdup(keywrd);
		DIR *d;
		d = opendir(pathtoCopy);
		if (!d) {
			fprintf (stderr, "Cannot open directory '%s': %s\n", pathtoCopy, strerror(errno));
			exit (EXIT_FAILURE);
		}

		//iterate through the files in the directoryPath of the searchCmd to build a list of
		//threads that we will be creating.
		while ((entry = readdir(d)) != NULL) {
			if(strstr(entry->d_name,".txt") && (strcmp(entry->d_name, ".") != 0 || strcmp(entry->d_name, "..") != 0 )){
				searchCommand *sc = create_SearchCommand(keywrdToCopy, pathtoCopy, entry->d_name);
				List_push(searchCommandList,sc);
			}
		}
		/* Close the directory. */
		if (closedir(d)){
			fprintf (stderr, "Could not close '%s': %s\n", pathtoCopy, strerror(errno));
			exit (EXIT_FAILURE);
		}
		free(pathtoCopy);
		free(keywrdToCopy);
		//free(d);
	}

	pthread_t
		workerId[searchCommandList->count + 1],
	    printerId[searchCommandList->count + 1];

	//Iterate through the searchCommandlist and start the child processes...
	LIST_FOREACH(searchCommandList, first, next, cur){
		if(cur){
			searchCommand *sc = cur->value;
			////debug("Has text %s, %s", sc->directoryPath, sc->filename);
			scCount++;
			////debug("%d", threadCount);
			////debug("SEARCH COMMAND:%d keyword: %s,%s,%s ", threadCount, sc->keyword, sc->directoryPath, sc->filename);

			//Create Child process here for each searchCommand.

			//This command is for non-thread usage
			//runSearchCommandForFile(sc);

			if(scCount <= searchCommandList->count){
				threadCount++;
				//debug("threadCount= %d",threadCount);
				Pthread_create(&workerId[threadCount], NULL, runSearchCommandForFile, sc);
				Pthread_create(&printerId[threadCount], NULL, printBoundedBuffer, &printerId[threadCount]);
			}
		}
	}

	//List_print_items(itemList);

	//debug("%d", threadCount);

	int j=0;
	for(j=1; j < threadCount+1; ++j){
		//debug("%d", j);
		pthread_join(workerId[j],NULL);
		//pthread_join(printerId[j],NULL);
		//debug("%d worker[%d] ended",pthread_join(workerId[j],NULL),j);
		//debug("%d printer[%d] ended",pthread_join(printerId[j],NULL),j);
	}

	List_destroy_list_SC(searchCommandList);
	List_destroy(boundedBuffer);
	//int i;
	//scanf ("%d",&i);

	free(keywrd);
	free(path);
	fclose(cmdFile);

	debug("Finished!");
	return 0;
}

