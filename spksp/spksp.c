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


// Bounded buffer list to be used by all threads.
List *boundedBuffer;

List *itemList;

int bufferSize = 0;

int threadCount = 0;

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

bool has_txt_extension(char const *);
searchCommand * create_SearchCommand(char *, char *, char *);

void free_searchCommand(searchCommand *);
void free_item(item *);

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
void runSearchCommandForFile(searchCommand *);

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

//helper concat function
char* concat(char *s1, char s2)
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *result = malloc(len1+len2+2);//
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2+2);//
    return result;
}

void addItemtoBoundedBuffer(item *itm){
	sem_wait(&empty);
	sem_wait(&mutex);
	List_push(boundedBuffer,itm);
	debug("added item!");
	sem_post(&mutex);
	sem_post(&full);
}

void runSearchCommandForFile(searchCommand *searchCmd){
	//debug("SEARCH COMMAND: keyword: %s, %s,%s ", searchCmd->keyword, searchCmd->directoryPath, searchCmd->filename);
	//FILE *f = fopen(concat(searchCmd->directoryPath,entry->d_name), "r");

	if(strstr(searchCmd->filename,".txt")){

	//debug("SEARCH COMMAND: keyword: %s, %s,%s ", searchCmd->keyword, searchCmd->directoryPath, searchCmd->d->d_name);
	strcat(searchCmd->directoryPath, "/");
	char* pathtoCopy = strcat(searchCmd->directoryPath,searchCmd->filename);
	FILE *f = fopen(pathtoCopy, "r");

	char str[MAXLINESIZE];
	char fullString[MAXLINESIZE];
	int lineNumber = 0;

	if(f == NULL) {
		  debug("Error opening file: %s", searchCmd->directoryPath);
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
			if(strcmp(pch,searchCmd->keyword) == 0){
				//debug("Keyword found! Line: %s", fullString);
				item *itm = create_Item(searchCmd->filename,lineNumber,fullString);
				List_push(itemList,itm);
				//addItemtoBoundedBuffer(itm);
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
	}else{

	}
}
item *do_GetBoundedBuffer() {
  item *tmp = boundedBuffer->last;
  return tmp;
}

void * printBoundedBuffer(void *arg) {
  int tmp = 0;
  while (tmp != -1) {
    sem_wait(&full);
    sem_wait(&mutex);
    tmp = do_GetBoundedBuffer();
    sem_post(&mutex);
    sem_post(&empty);
    if (tmp != -1) {
      //printf("Consumer%d - Item with line number: %d, is extracted.\n", (*(item *)arg)->matchLineNumber, tmp);
    }
  }
  return NULL;
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

int main(int argc, char *argv[]) {

	if (argc != 3) {
		debug("Check format. Should be \":~$ ./spksp commandFile bufferSize\"");
	    exit(1);
	}

	char *commandFile = argv[1];
	FILE *cmdFile;
	cmdFile = fopen(commandFile,"r");

	// initialize the bounded buffer list
	//boundedBuffer = List_create();
	itemList = List_create();

	// initialize the list of processes
	List *searchCommandList = List_create();

    sem_init(&empty, 0, bufferSize); // buffer is empty
	sem_init(&full, 0, 0);    // 0 are full
	sem_init(&mutex, 0, 1);   // mutex

	//Get BufferSize from argument
	if(argv[2] == NULL || argv[2] <= 0){
		debug("Check format. Should be \":~$ ./spksp commandFile bufferSize\"");
		exit(1);
	}else{
		bufferSize = atoi(argv[2]);
		//debug("BufferSize: %d", bufferSize);
	}

	//scan the file
	char *keywrd = malloc(24 * sizeof(keywrd));
	char *path = malloc(24 * sizeof(path));
	struct dirent * entry = malloc(24* sizeof(entry));

	while(fscanf(cmdFile, "%s %s", path, keywrd) != EOF){

		char *pathtoCopy = strdup(path);
		char *keywrdToCopy = strdup(keywrd);
		DIR *d;
		d = opendir(pathtoCopy);
		if (!d) {
			fprintf (stderr, "Cannot open directory '%s': %s\n", pathtoCopy, strerror(errno));
			exit (EXIT_FAILURE);
		}

		//iterate through the files in the directoryPath of the searchCmd
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

	//pthread_t workerId[bufferSize];
	//Iterate through the searchCommandlist and start the child processes...
	LIST_FOREACH(searchCommandList, first, next, cur){
		if(cur){
			searchCommand *sc = cur->value;
					//debug("Has text %s, %s", sc->d->d_name, sc->directoryPath);
					threadCount++;
					//Create Child process here for this each file.
					//sc->d = entry2;
					//debug("Creating thread with count = %d", count);
					//wait(workerId[count]);
					//printf("count is %d\n", threadCount);
					if(strstr(sc->filename,".txt")){
						runSearchCommandForFile(sc);
					}
					//Pthread_create(&workerId[threadCount], NULL, runSearchCommandForFile, sc);
				}
	}

	List_print_items(itemList);

	//DESTROY SHIT
	/*List_destroy_list_SC(searchCommandList);
	List_destroy_list_Items(itemList);
	free(keywrd);
	free(path);
	fclose(cmdFile);*/

	/*int j=0;
	for(j=0; j < threadCount; ++j){
		debug("%d",threadCount);
	}*/

	//debug("Finished!");
	return 0;
}
