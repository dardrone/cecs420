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

typedef struct word {
  char * text;
  int count;
} word;

word * create_word(char *, int);
void destroy_word(word *);

word * create_word(char * text, int count)
{
  word* word_ptr = (word *) malloc(sizeof(word));
  word_ptr->text = strdup(text);
  word_ptr.count = count;
  return word_ptr;
}

void free_word(word* delete_word)
{
  free(delete_word->text);
  free(delete_word);
}

int is_sorted(List *words)
{
    LIST_FOREACH(words, first, next, cur) {
	if(cur->next && strcmp(cur->value, cur->next->value) > 0) {
	    debug("%s %s", (char *)cur->value, (char *)cur->next->value);
	    return 0;
	}

    }

    return 1;
}

void insertionSort(List *list)
{
	int sorted = 1;
	do {
	sorted = 1;
        LIST_FOREACH(list, first, next, cur) {
            if(cur->next) {
                if(strcmp(cur->value, cur->next->value) > 0) {
                    ListNode_swap(cur, cur->next);
                    sorted = 0;
                }
            }
        }
    } while(!sorted);
}

void FindCommonWords(List *list1, List *list2, List *commonList,FILE *output){
	LIST_FOREACH(list1, first,next,cur) {
		if(cur->next){
			LIST_FOREACH(list2,first,next,cur) {
				if(cur->next){
					if(strcmp(cur1->value,cur1->next1->value) == 0){
						List_push(commonList,cur1->value);
						fprintf(output,"%s",cur1->value);
					}
				}
			}
		}
	}
}


int main(int argc, char *argv[]) {
	
	static List *list1 = NULL;
	static List *list2 = NULL;
	static List *finalList = NULL;
	
	list1 = List_create();
	list2 = List_create();
	finalList = List_create();
	//int i=0;
	/*while(i < argc) {
		printf("\narg[%d]: %s\n", i, argv[i]);
		i++;
	}*/

	char *filename1 = argv[1];
	FILE *thefirstfile;
	thefirstfile = fopen(filename1,"r");
	
	char *filename2 = argv[2];
	FILE *thesecondfile;
	thesecondfile = fopen(filename2,"r");
	
	char *filename3 = argv[3];
	FILE *outputfile;
	outputfile = fopen(filename3,"w"); // "w" - writing to the output


	if (thefirstfile == NULL || thesecondfile == NULL) {
	  fprintf(stderr, "Can't open the files!\n");
	  exit(1);
	}

	//
	//scan the first file in the arg list.
	//
	char *word1 = malloc(21 * sizeof(word1));
	while(fscanf(thefirstfile, "%s", word1) != EOF) {
		char *wordToCopy = strdup(word1);
		List_push(list1,wordToCopy);
//		free(wordToCopy);
		//fprintf(outputfile,"%s\n", word1);
	}

	//
	//scan the second file in the arg list.
	//
	char *word2 = malloc(21 * sizeof(word2));
	while(fscanf(thesecondfile, "%s", word2) != EOF) {
		char *wordToCopy = strdup(word2);
		List_push(list2,wordToCopy);
//		free(wordToCopy);
		//fprintf(outputfile,"%s\n", word);
	}
	
	//maList_makeIndices(list);
	//printf("First value: %s\n",(char *)list->first->value);
	//printf("First->Next value: %s\n",(char *)list->first->next->value);
	debug("sorting list1...");
	//insertionSort(list1);
	debug("sorting list2..");
	//insertionSort(list2);
	
	List_print(list1);
//	printf("\nlast->prev->value: %s\n",(char *)list->last->prev->value);
//	printf("\nfirst->next: %s\n",(char *)list->first->next->value);
	printf("%d",is_sorted(list1));

	FindCommonWords(list1,list2,finalList,outputfile);
	
	fprintf(	
	List_destroy(list1);
	List_destroy(list2);
	free(word1);
	free(word2);
	fclose(thefirstfile);
	fclose(thesecondfile);
	fclose(outputfile);
	debug("finished!");
	return 0;
}
