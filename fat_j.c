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
	if (scanf("%[^\n]%*c", cmdLine) == NULL)
		return NULL;

	// printf("%s\n", cmdLine);

	return cmdLine; // returns cmdLine string
}


char * removePeriods(char * str) {

    int i = 0;

    while (str[i] != '\0'){
        if (str[i] == '.')
            str[i] = ' ';
        i++;
    }

    return str;
}

///////////////////////////////////////////////////////////////////////////////

int main(){


	numberOfFiles = 0;
	
	char *wholeCmd, *cmd, *fileName, *flag;
	char cmdLine [128];
	size_t result;
	bool exitBool = false;

	fp = fopen("fat32.img", "rb");

	BootSectorInformation();
	PrintBPS();
	printf("First Data Sector:\t%i\n", fds);
	printf("Root Directory:\t\t%i\n", LocateFSC(root_clus));

	while(strcpy(cmdLine, getCmd())){
		
		wholeCmd = strtok (cmdLine, " ");
		cmd = wholeCmd;

		wholeCmd = strtok (NULL, " ");
		fileName = wholeCmd;
		
		fileName = removePeriods(fileName);
		printf("%s\n", fileName);

		wholeCmd = strtok (NULL, " ");
		flag = wholeCmd;
			
		if(strcmp(cmd, "open") == 0) {
			if(fileName != NULL && flag != NULL)
				Open(fileName, flag);
			else 
				fprintf(stderr, "ERROR: usage: open <FILENAME> <MODE>.\n", cmd);
		} else if(strcmp(cmd, "close") == 0) {
			if (fileName != NULL)
				Close(fileName);
			else
				fprintf(stderr, "ERROR: usage: close <FILENAME>.\n", cmd);
				continue;
		} else if(strcmp(cmd, "create") == 0) {
		
		} else if(strcmp(cmd, "ls") == 0) {
		
			PrintDirectory(root_clus);
		
		} else if(strcmp(cmd, "exit") == 0) {
		
			break; // breaks out of loop
		
		} else {
		
			fprintf(stderr, "ERROR: '%s' is not a supported command.\n", cmd);
		}
	}

	fclose(fp);

	return 0;
}