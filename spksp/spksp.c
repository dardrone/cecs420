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
#include "list.c"
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>

#define MAXLINESIZE 1024

// Bounded buffer list to be used by all threads.
List *boundedBuffer;

List *itemList;

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
searchCommand * create_SearchCommand(char *, char *);

void free_searchCommand(searchCommand *);
void free_item(item *);

void free_searchCommand(searchCommand* delete_sc)
{
  free(delete_sc->directoryPath);
  free(delete_sc->keyword);
  free(delete_sc);
}
void free_item(item *itm)
{
  free(itm->filename);
  free(itm->line);
  free(itm);
}
searchCommand * create_SearchCommand(char *kw, char *path)
{
	searchCommand* sc_ptr = (searchCommand *) malloc(sizeof(searchCommand));
	sc_ptr->keyword = strdup(kw);
	sc_ptr->directoryPath = concat(strdup(path),"/");
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

//
void runSearchCommandProcesses(searchCommand *searchCmd){
	DIR * d;
	d = opendir (searchCmd->directoryPath);
	//debug("Searching Directory %s", searchCmd->directoryPath);
	if (!d) {
		fprintf (stderr, "Cannot open directory '%s': %s\n", searchCmd->directoryPath, strerror(errno));
		exit (EXIT_FAILURE);
	}
	while (1) {
		struct dirent * entry;
		entry = readdir (d);
		if (!entry) {
			break;
		}
		if(has_txt_extension(entry->d_name)){
			//debug("%s", entry->d_name);
			//Create Child process here for this each file.
			runSearchCommandForFile(searchCmd, entry);
		}
	}
	/* Close the directory. */
	if (closedir(d)){
		fprintf (stderr, "Could not close '%s': %s\n", searchCmd->directoryPath, strerror(errno));
		exit (EXIT_FAILURE);
	}
}



void runSearchCommandForFile(searchCommand *searchCmd, struct dirent * entry){

	//scan the file
	//debug("SEARCH COMMAND: keyword: %s, searchPath: %s", searchCmd->keyword, concat(searchCmd->directoryPath,entry->d_name));
	//FILE *f = fopen(concat(searchCmd->directoryPath,entry->d_name), "r");
	char *fullPath = malloc(strlen(searchCmd->directoryPath) + strlen(entry->d_name) + 2);
	sprintf(fullPath, "%s/%s", searchCmd->directoryPath,entry->d_name);
	FILE *f = fopen(fullPath, "r");
	free(fullPath);
	char str[MAXLINESIZE];
	char fullString[MAXLINESIZE];
	int lineNumber = 0;

	if(f == NULL) {
	      debug("Error opening file");
	      exit(-1);
	   }

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
				item *itm = create_Item(entry->d_name,lineNumber,fullString);
				List_push(itemList,itm);
			}
			pch = strtok_r(NULL, " ",&staticReferencePtr);
		}
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
	      searchCommand *sc = cur->prev->value;
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
    if(cur->prev) {
      searchCommand *sc = cur->prev->value;
      //debug("Freeing %s...", wrdd->text);
      free_searchCommand(sc);
      free(cur->prev);
    }
  }

  searchCommand *scLast = list->last->value;
  free_searchCommand(scLast);
  free(list->last);
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

    //sem_init(&empty, 0, max); // max are empty
	sem_init(&full, 0, 0);    // 0 are full
	sem_init(&mutex, 0, 1);   // mutex

	// Get the buffer size
	int bufferSize = -1;
	if(argv[2] == NULL || argv[2] <= 0){
		debug("Check format. Should be \":~$ ./spksp commandFile bufferSize\"");
		exit(1);
	}else{
		bufferSize = atoi(argv[2]);
		//debug("BufferSize: %d", bufferSize);
	}
	//debug("MAXLINESIZE: %d", MAXLINESIZE);

	//scan the file
	char *keywrd = malloc(24 * sizeof(keywrd));
	char *path = malloc(24 * sizeof(path));

	while(fscanf(cmdFile, "%s %s", path, keywrd) != EOF){
		searchCommand *sc = create_SearchCommand(keywrd, path);
			List_push(searchCommandList,sc);
	}

	//List_print_searchCommands(searchCommandList);

	LIST_FOREACH(searchCommandList, first, next, cur){
		if(cur){
				searchCommand *sc = cur->value;
				runSearchCommandProcesses(sc);
			}
		}
	List_print_items(itemList);

	//DESTROY SHIT
	List_destroy_list_SC(searchCommandList);
	List_destroy_list_Items(itemList);
	free(keywrd);
	free(path);
	fclose(cmdFile);

	//debug("Finished!");
	return 0;
}
