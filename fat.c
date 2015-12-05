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
	if (scanf("%s", cmdLine) == NULL)
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
	
	// cmd = getCmd();

	while(cmd = getCmd()) {
		if(strcmp(cmd, "open") == 0) {
			
			printf("Open\n");
		
		} else if(strcmp(cmd, "ls") == 0) {
		
			PrintDirectory(root_clus);
		
		} else if(strcmp(cmd, "exit") == 0) {
		
			break; // breaks out of loop
		
		} else {
		
			fprintf(stderr, "ERROR: '%s' is not a supported command.\n", cmd);
		}

	}

	PrintDirectory(root_clus);

	fclose(fp);

	return 0;
}
