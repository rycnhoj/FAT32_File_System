#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

extern FILE* fp;
extern unsigned int curr_dir;

typedef enum { false, true } bool;

/**
GETCMD()
Main function for printing shell prompt and getting
user inputs.
**/
static char* getCmd() {
	static char cmdLine[128];

	printf("%s => ", "FAT");

	// gets input from user and stores in cmdLine or returns NULL
	if (fgets(cmdLine, 128, stdin) == NULL)
		return NULL;

	return cmdLine; // returns cmdLine string
}

///////////////////////////////////////////////////////////////////////////////

int main(){
	char* cmd, temp;
	size_t result;
	bool exitBool = false;

	fp = fopen("fat32.img", "rb");
	if(!fp){
		fputs("ERROR: File not found.\n", stderr);
		exit(1);
	}
	cmd = (char*) malloc (128);

	BootSectorInformation();
	PrintBPS();
	printf("First Data Sector:\t%i\n", fds);
	printf("Root Directory:\t\t%i\n", LocateFSC(root_clus));

	while((cmd = getCmd()) != NULL) {
		cmd[strlen(cmd)-1] = '\0';
		if(strncmp(cmd, "open", 4) == 0) {
			printf("Open\n");
		} else if(strncmp(cmd, "ls", 2) == 0) {
			if(strlen(cmd) < 3){
				fprintf(stderr, "USAGE: ls <directory_name>.\n");
				continue;
			}
			List(&cmd[3]);
		} else if(strncmp(cmd, "exit", 4) == 0) {
		
			break; // breaks out of loop
		
		} else {
		
			fprintf(stderr, "ERROR: '%s' is not a supported command.\n", cmd);
		}
	}

	fclose(fp);
	return 0;
}
