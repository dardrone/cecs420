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



// Bounded buffer list to be used by all threads.
List *boundedBuffer;

List *itemList;
int bufferSize;
int threadCount = 0;

int fill = 0;
int use = 0;

//Semaphores
sem_t empty;
sem_t full;
sem_t mutex;

typedef struct searchCommand {
	char* keyword; // single word to search for. Doesn't include white space <SPACE>, <TAB>, or <NEWLINE>.
	char* directoryPath; // Full path e.g. /home/naltipar/dirOne
	char* filename;
} searchCommand;

typedef struct item {
	char* filename; // single word to search for. Doesn't include white space <SPACE>, <TAB>, or <NEWLINE>.
	int matchLineNumber; // Full path e.g. /home/naltipar/dirOne
	char* line;

} item;

//function prototypes
void List_print_searchCommands(List *);
void free_searchCommand(searchCommand *);
void free_item(item *);
searchCommand * create_SearchCommand(char *, char *, char *);
item * create_Item(char*, int, char*);
bool has_txt_extension(char const *);
void List_destroy_list_Items(List *);
void List_destroy_list_SC(List *);
void do_fillBoundedBuffer(item *);
item *do_GetBoundedBuffer();
void * printBoundedBuffer(void *);
void * runSearchCommandForFile(void *);

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
  free(delete_sc->directoryPath);
  free(delete_sc->keyword);
  debug("killing list");
  free(delete_sc->filename);
  free(delete_sc);
}
void free_item(item *itm)
{
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
    //debug("Text? Length: %d, %s",len, name+len - 4);
    return len > 4 && strcmp(name + len - 4, ".txt") == 0;
}

void List_destroy_list_Items(List *list){
	LIST_FOREACH(list, first, next, cur) {
	    if(cur->prev) {

	      item *sc = cur->prev->value;
	      //debug("Freeing %s...", wrdd->text);
	      free_item(sc);
	      free(cur->prev);
	    }
	  }

	  item *itm = list->last->value;
	  free_item(itm);
	  free(list->last);
	  free(list);
}

void List_destroy_list_SC(List *list)
{
  LIST_FOREACH(list, first, next, cur) {
    if(cur) {
    	debug("killing...");
    	searchCommand *sc = cur->value;
    	//debug("Freeing %s...", wrdd->text);
    	free_searchCommand(sc);
    	free(cur);
    }
  }
  debug("killing list");
  free(list);
}

void do_fillBoundedBuffer(item *itm){
  debug("Pushing item to bounded buffer...");
	List_push(boundedBuffer,itm);
  if(boundedBuffer->count == bufferSize) {
	debug("Bounded buffer is full");

  }
}


int do_GetBoundedBuffer() {
  item *tmp = boundedBuffer->last->value;
  if(boundedBuffer->count == bufferSize){
	  tmp->matchLineNumber=-1;
  }
  return tmp->matchLineNumber;
}
/*
int do_get() {
  int tmp = buffer[use];
  use++;
  if (use == max) {
    use = 0;
  }
  return tmp;
}*/

//consumer with arg=processid
void * printBoundedBuffer(void *arg) {
  int tmp = 0;
  debug("Printing!");
  while (tmp != -1) {
    sem_wait(&full);
    sem_wait(&mutex);
    tmp = do_GetBoundedBuffer();
    sem_post(&mutex);
    sem_post(&empty);
    if (stuffIntheBoundedBuffer) {
      printf("Consumer%d - Item with line number: %d, is extracted.\n", (*(int *)arg), itm->matchLineNumber);
    }
  }
  return NULL;
}

//producer
void *runSearchCommandForFile(void *searchCmd){
	struct searchCommand *searchCommd = searchCmd;
	if(strstr(searchCommd->filename,".txt")){
		//debug("SEARCH COMMAND: keyword: %s, %s,%s ", searchCommd->keyword, searchCommd->directoryPath, searchCommd->filename);

		char* directoryPathcpy = strdup(searchCommd->directoryPath);
		strcat(directoryPathcpy,"/");
		char* filenamecpy = strdup(searchCommd->filename);
		char* keywordcpy = strdup(searchCommd->keyword);
		char* pathtoCopy = strcat(directoryPathcpy,filenamecpy);

		debug("Path to copy: %s", pathtoCopy);
		FILE *f = fopen(pathtoCopy, "r");
		char str[MAXLINESIZE];
		char fullString[MAXLINESIZE];
		int lineNumber = 0;

		if(f == NULL) {
			  debug("Error opening file: %s", searchCommd->directoryPath);
			  exit(-1);
		}

		//go through the file and find the matches
		while(fgets(str, MAXLINESIZE, f) != NULL) {
			//debug("STR IS: %s", str);
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
				//debug("%s",pch);
				if(strcmp(pch,keywordcpy) == 0){
					//debug("Keyword found! Line: %s", fullString);
					item *itm = create_Item(filenamecpy,lineNumber,fullString);
					//List_push(itemList,itm);
					//debug("before sem_wait... empty=%ld", empty.__size);
					sem_wait(&empty);
					sem_wait(&mutex);
					do_fillBoundedBuffer(itm);
					debug("added item!");
					sem_post(&mutex);
					sem_post(&full);
					found = 1;
					//debug("Item: %s has been added", itm->filename);
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
		debug("BufferSize: %d", bufferSize);
	}

	sem_init(&empty, 0, bufferSize); // buffer is empty
	sem_init(&full, 0, 0);    // 0 are full
	sem_init(&mutex, 0, 1);   // mutex

	char *commandFile = argv[1];
	FILE *cmdFile;
	cmdFile = fopen(commandFile,"r");

	// initialize the bounded buffer list
	boundedBuffer = List_create();
	itemList = List_create();

	// initialize the list of processes
	List *searchCommandList = List_create();

	//scan the file
	char *keywrd = malloc(24 * sizeof(keywrd));
	char *path = malloc(24 * sizeof(path));
	struct dirent * entry = malloc(24* sizeof(entry));


	//Go through the commandfile to get all the search commands
	while(fscanf(cmdFile, "%s %s", path, keywrd) != EOF){

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
				//debug("Has text extension: %s", entry->d_name);
				searchCommand *sc = create_SearchCommand(keywrdToCopy, pathtoCopy, entry->d_name);
				List_push(searchCommandList,sc);
			}
		}
		/* Close the directory. */
		if (closedir(d)){
			fprintf (stderr, "Could not close '%s': %s\n", pathtoCopy, strerror(errno));
			exit (EXIT_FAILURE);
		}
	}

	//List_print_searchCommands(searchCommandList);




	//debug("%d",searchCommandList->count);

	pthread_t
		workerId[searchCommandList->count + 1],
	    printerId;

	//Iterate through the searchCommandlist and start the child processes...
	LIST_FOREACH(searchCommandList, first, next, cur){
		if(cur){
			searchCommand *sc = cur->value;
			//debug("Has text %s, %s", sc->directoryPath, sc->filename);
			threadCount++;
			//debug("%d", threadCount);
			//debug("SEARCH COMMAND:%d keyword: %s,%s,%s ", threadCount, sc->keyword, sc->directoryPath, sc->filename);
			//Create Child process here for this each file.
			//runSearchCommandForFile(sc);
			//debug("bufferSize: %d, threadCount: %d",bufferSize, threadCount);
			if(threadCount <= searchCommandList->count){
				//debug("SEARCH COMMAND:%d keyword: %s,%s,%s ", threadCount, sc->keyword, sc->directoryPath, sc->filename);
				Pthread_create(&workerId[threadCount], NULL, runSearchCommandForFile, sc);
			}
		}
	}

	Pthread_create(&printerId, NULL, printBoundedBuffer, (void*)printerId);

	List_print_items(itemList);

	//int i;
	//scanf ("%d",&i);
	//DESTROY SHIT
	/*List_destroy_list_SC(searchCommandList);
	List_destroy_list_Items(itemList);
	free(keywrd);
	free(path);
	fclose(cmdFile);*/

	int j=0;
	for(j=0; j < threadCount; ++j){
		pthread_join(workerId[j],NULL);
		debug("%d",j);
	}

	debug("Finished!");
	return 0;
}
