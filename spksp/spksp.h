/*
 * spksp.h
 *
 *  Created on: Nov 8, 2014
 *      Author: djennings
 */

#ifndef SPKSP_H_
#define SPKSP_H_

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

void List_print_searchCommands(List *);
void free_searchCommand(searchCommand *);
void free_item(item *);
searchCommand * create_SearchCommand(char *, char *, char *);
item * create_Item(char*, int, char*);

#endif /* SPKSP_H_ */
