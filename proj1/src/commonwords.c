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

word * create_word(char *);
void destroy_word(word *);

word * create_word(char * text)
{
  word* word_ptr = (word *) malloc(sizeof(word));
  word_ptr->text = strdup(text);
  word_ptr->count = 1;
  return word_ptr;
}

void addtoWord(word* word){
	word->count++;
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
		//debug("%s %s", (char *)cur->value, (char *)cur->next->value);
	    	return 0;
	}

    }

    return 1;
}

void List_push_word(List *list, word *wrd)
{
  ListNode *node = calloc(1, sizeof(ListNode));
  check_mem(node);
  debug("**PUSHING %s",wrd->text);
  node->value = wrd;
  debug("****node->value:%s",wrd->text);
  if(list->last == NULL) {
    debug("****list is null");
    list->first = node;
    list->last = node;
  } else {
	debug("List is not null");
	LIST_FOREACH(list, first, next, cur){
		if(cur){
			word *tehWord = cur->value;
			debug("Comparing %s with %s",(char *)cur->value, wrd->text);
			if(strcmp(wrd->text,tehWord->text)==0){
				debug("the same!\n");
				wrd->count++;
			}	
						
		}
	}
    debug("******push stuff on the null list");
    list->last->next = node;
    node->prev = list->last;
    list->last = node;
  }

  list->count++;
 error:
  return;
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

/*void FindCommonWords(List *list1, List *list2, List *commonList,FILE *output){
		LIST_FOREACH(list1, first,next,cur1) {
			if(cur1->prev) {	
			LIST_FOREACH(list2,first,next,cur2){
				if(cur2->prev){
					if(strcmp(cur1->value,cur2->value) == 0){
						//debug("%s is shared between the lists...", (char *)cur2->value);
						if(commonList->first != NULL){
							LIST_FOREACH(commonList,first,next,cur3){
								if(cur3->prev){
									word *word = cur3->value;
									//debug("hey: %s",word->text);
									if(strcmp(word->text,cur2->value)==0){
										List_push(commonList,word);
//										debug("pushing to commonlist: %s", word->text);
										break;
									}								
								}else{
									word *word = create_word(strdup(cur2->value));
									word->count++;
//									debug("%s added to the sorted list...", word->text);
									List_push(commonList,word);
									}
							}
						}
						//fprintf(output,"%s, %d",(char *)word->text, word->count);
					}
				}
			}
		}
	}
}*/
void FindCommonWords(List *list1, List *list2, List *commonList,FILE *output){
	debug("*****FIND COMMON WORDS******");
	LIST_FOREACH(list1, first,next,cur1) {
		if(cur1) {
		debug("list1 cur1->value: %s",(char *)cur1->value);	
		LIST_FOREACH(list2,first,next,cur2){
			if(cur2){
				debug("comparing %s with %s",(char *)cur2->value, (char *) cur1->value);
				if(strcmp(cur1->value,cur2->value) == 0){
					debug("%s is shared between the lists...", (char *)cur2->value);
					word *thewrd = create_word(cur2->value);
					List_push_word(commonList,thewrd);
					}
				}
				//fprintf(output,"%s, %d",(char *)word->text, word->count);
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
		//fprintf(outputfile,"%s\n", word1);
	}

	//
	//scan the second file in the arg list.
	//
	char *word2 = malloc(21 * sizeof(word2));
	while(fscanf(thesecondfile, "%s", word2) != EOF) {
		char *wordToCopy = strdup(word2);
		List_push(list2,wordToCopy);
	}
	
	//maList_makeIndices(list);
	//printf("First value: %s\n",(char *)list->first->value);
	//printf("First->Next value: %s\n",(char *)list->first->next->value);
	debug("sorting list1...");
	//insertionSort(list1);
	debug("sorting list2..");
	//insertionSort(list2);
	debug("********LIST1********");	
	List_print(list1);
	debug("*********************");	
	debug("********LIST2********");	
	List_print(list2);
	debug("*********************");	
//	printf("\nlast->prev->value: %s\n",(char *)list->last->prev->value);
//	printf("\nfirst->next: %s\n",(char *)list->first->next->value);
//	printf("%d",is_sorted(list1));

	FindCommonWords(list1,list2,finalList,outputfile);

	debug("PRINTING LIST:");
	debug("list count: %d", finalList->count);	
	LIST_FOREACH(finalList, first, next, cur) {
		  if(cur->prev){
			word *word = cur->value;
			printf("%s, %d\n", (char *)word->text, word->count);
			free_word(word);
		  }
		}
	
	
	List_destroy(list1);
	List_destroy(list2);
	List_destroy(finalList);
	free(word1);
	free(word2);
	fclose(thefirstfile);
	fclose(thesecondfile);
	fclose(outputfile);
	debug("finished!");
	return 0;
}
