#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

extern FILE* fp;
extern unsigned int curr_dir;

/**
GETCMD()
Main function for printing shell prompt and getting
user inputs.
**/
static char* getCmd() {
	static char cmdLine[1024];

	printf("%s => ", "FAT");

	// gets input from user and stores in cmdLine or returns NULL
	if (fgets(cmdLine, sizeof(cmdLine), stdin) == NULL)
		return NULL;

	// checks and adds Null character if needed
	if (cmdLine[strlen(cmdLine) - 1] == '\n')
		cmdLine[strlen(cmdLine) - 1] = 0;

	return cmdLine; // returns cmdLine string
}

///////////////////////////////////////////////////////////////////////////////

int main(){
	char* message, cmdline, cmd, temp;
	size_t result;
	int i;

	message = (char*) malloc(513);
	fp = fopen("fat32.img", "rb");

	BootSectorInformation();
	PrintBPS();
	printf("First Data Sector:\t%i\n", fds);
	printf("Root Directory:\t\t%i\n", LocateFSC(root_clus));

	// while((cmdline = getCmd()) != NULL){
	// 	if(strcmp(cmd, "open")){
	// 		// OPEN
	// 	}
	// 	else if(strcmp(cmd, "ls")){
	// 		PrintDirectory(root_clus);
	// 	}
	// 	else if(strcmp(cmd, "exit")){
	// 		break;
	// 	}
	// 	else{
	// 		fprintf(stderr, "ERROR: '%s' is not a supported command.\n", cmd);
	// 	}
	// }

	PrintDirectory(root_clus);

	fclose(fp);

	return 0;
}