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

//Semaphores
sem_t empty;
sem_t full;
sem_t mutex;

typedef struct searchCommand {
	char* keyword; // single word to search for. Doesn't include white space <SPACE>, <TAB>, or <NEWLINE>.
	char* directoryPath; // Full path e.g. /home/naltipar/dirOne
	struct dirent *d;
} searchCommand;

typedef struct item {
	char* filename; // single word to search for. Doesn't include white space <SPACE>, <TAB>, or <NEWLINE>.
	int matchLineNumber; // Full path e.g. /home/naltipar/dirOne
	char* line;

} item;

//helper concat function
char* concat(char *s1, char *s2)
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *result = malloc(len1+len2+1);//
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2+1);//
    return result;
}

void List_print_searchCommands(List *list){
	LIST_FOREACH(list, first, next, cur){
	if(cur){
			searchCommand *sc = cur->value;
			debug("%s, %s",sc->keyword,sc->directoryPath);
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
searchCommand * create_SearchCommand(char *, char *, struct dirent *);

void free_searchCommand(searchCommand *);
void free_item(item *);

void free_searchCommand(searchCommand* delete_sc)
{
  free(delete_sc->directoryPath);
  free(delete_sc->keyword);
  debug("killing list");
  free(delete_sc->d);
  free(delete_sc);
}
void free_item(item *itm)
{
  free(itm->filename);
  free(itm->line);
  free(itm);
}
void runSearchCommandForFile(searchCommand *);

searchCommand * create_SearchCommand(char *kw, char *path, struct dirent *d)
{
	searchCommand* sc_ptr = (searchCommand *) malloc(sizeof(searchCommand));
	sc_ptr->keyword = strdup(kw);
	sc_ptr->directoryPath = strdup(path);
	sc_ptr->d = d;
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

void addItemtoBoundedBuffer(List *boundedBuffer, item *itm){

}

void runSearchCommandForFile(searchCommand *searchCmd){
	//debug("SEARCH COMMAND: keyword: %s, %s,%s ", searchCmd->keyword, searchCmd->directoryPath, searchCmd->d->d_name);
	//FILE *f = fopen(concat(searchCmd->directoryPath,entry->d_name), "r");
	char *fullPath = malloc(strlen(searchCmd->directoryPath) + strlen(searchCmd->d->d_name) + 2);

	if(has_txt_extension(searchCmd->d->d_name)){
		sprintf(fullPath, "%s/%s", searchCmd->directoryPath,searchCmd->d->d_name);
		//debug("FullPath: %s", fullPath);
		FILE *f = fopen(fullPath, "r");

		char str[MAXLINESIZE];
		char fullString[MAXLINESIZE];
		int lineNumber = 0;

		if(f == NULL) {
			  debug("Error opening file: %s", fullPath);
			  exit(-1);
		}

		//go through the file and find the matches
		while(fgets(str, MAXLINESIZE, f) != NULL) {
			//debug("STR IS: %s", str);
			lineNumber++;
			char * pch;
			char * staticReferencePtr;

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
					item *itm = create_Item(searchCmd->d->d_name,lineNumber,fullString);
					List_push(itemList,itm);
					addItemtoBoundedBuffer(boundedBuffer,itm);
					//debug("Item: %s has been added", itm->filename);
				}
				pch = strtok_r(NULL, " ",&staticReferencePtr);
			}
		}
	}else{
		debug("%s doesn't have text extension, don't do anything...", fullPath);
		//free(fullPath);
	}
}

bool has_txt_extension(char const *name)
{
    size_t len = strlen(name);
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

	while(fscanf(cmdFile, "%s %s", path, keywrd) != EOF){
		struct dirent * entry1;
		DIR *d;
		d = opendir(path);
		entry1 = readdir (d);
		searchCommand *sc = create_SearchCommand(keywrd, path, entry1);
		List_push(searchCommandList,sc);
	}

	//List_print_searchCommands(searchCommandList);
	pthread_t workerId[bufferSize];
	//Iterate through the searchCommandlist and start the child processes...
	LIST_FOREACH(searchCommandList, first, next, cur){
		if(cur){
			searchCommand *sc = cur->value;
			DIR * d;
			d = opendir (sc->directoryPath);
			int count=0;

			//debug("Searching Directory %s", sc->directoryPath);
			if (!d) {
				fprintf (stderr, "Cannot open directory '%s': %s\n", sc->directoryPath, strerror(errno));
				exit (EXIT_FAILURE);
			}

			//iterate through the files in the directoryPath of the searchCmd
			while (1) {
				struct dirent * entry2;
				entry2 = readdir (d);
				if (!entry2) {
					break;
				}
				//debug("looking at file: %s",entry2->d_name);
				if(has_txt_extension(entry2->d_name)){
					//debug("%s, %s", entry2->d_name, sc->directoryPath);
					count++;
					//Create Child process here for this each file.
					sc->d = entry2;
					//debug("Creating thread with count = %d", count);
					//wait(workerId[count]);
					runSearchCommandForFile(sc);
					//debug("count=%d",count);
				}
			}
			/* Close the directory. */
			if (closedir(d)){
				fprintf (stderr, "Could not close '%s': %s\n", sc->directoryPath, strerror(errno));
				exit (EXIT_FAILURE);
			}
		}
	}

	List_print_items(itemList);

	//DESTROY SHIT
	/*List_destroy_list_SC(searchCommandList);
	List_destroy_list_Items(itemList);
	free(keywrd);
	free(path);
	fclose(cmdFile);*/

	debug("Finished!");
	return 0;
}
