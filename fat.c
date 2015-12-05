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

	puts("*****************************");
	BootSectorInformation();
	PrintBPS();
	printf("First Data Sector:\t%i\n", fds);
	printf("Root Directory:\t\t%i\n", LocateFSC(root_clus));
	puts("*****************************");

	while((cmd = getCmd()) != NULL) {
		cmd[strlen(cmd)-1] = '\0';
		if(strncmp(cmd, "open", 4) == 0) {
			if(strlen(cmd) < 5){
				fprintf(stderr, "USAGE: open <FILE_NAME> <MODE>\n");
				continue;
			}
			char fileName[12];
			char mode[3];
			sscanf(cmd, "%*s %s %s", fileName, mode);
			printf("FN: %s\tMODE: %s\n", fileName, mode);
			if (strpbrk(mode, "rw") == NULL){
				fprintf(stderr, "ERROR: '%s' is an invalid mode.\n", mode);
				fprintf(stderr, "Available modes:\n");
				fprintf(stderr, "\tr - Read-only.\n");
				fprintf(stderr, "\tw - Write-only.\n");
				fprintf(stderr, "\trw/wr - Read & Write.\n");
				continue;
			}

			Open(fileName, mode);
		} 
		else if(strncmp(cmd, "close", 5) == 0) {
			if(strlen(cmd) < 5){
				fprintf(stderr, "USAGE: open <FILE_NAME> <MODE>\n");
				continue;
			}
			char fileName[12];
			sscanf(cmd, "%*s %s", fileName);
			Close(fileName);
		}
		else if(strncmp(cmd, "create", 6) == 0) {}
		else if(strncmp(cmd, "rm", 2) == 0) {}
		else if(strncmp(cmd, "size", 4) == 0) {
			if(strlen(cmd) < 5){
				fprintf(stderr, "USAGE: size <FILE_SIZE>\n");
				continue;
			}
			PrintSize(&cmd[5]);
		}
		else if(strncmp(cmd, "cd", 2) == 0) {
			if(strlen(cmd) < 3){
				fprintf(stderr, "USAGE: cd <DIR_NAME>\n");
				continue;
			}
			ChangeDirectory(&cmd[3]);
		}
		else if(strncmp(cmd, "ls", 2) == 0) {
			if(strlen(cmd) < 3){
				fprintf(stderr, "USAGE: ls <DIR_NAME>\n");
				continue;
			}
			List(&cmd[3]);
		} 
		else if(strncmp(cmd, "mkdir", 5) == 0) {
			if(strlen(cmd) < 6){
				fprintf(stderr, "USAGE: mkdir <DIR_NAME>\n");
				continue;
			}
			MakeDir(&cmd[6]);
		}
		else if(strncmp(cmd, "rmdir", 5) == 0) {
			if(strlen(cmd) < 6){
				fprintf(stderr, "USAGE: rmdir <DIR_NAME>\n");
				continue;
			}
			RemoveDir(&cmd[6]);
		}
		else if(strncmp(cmd, "read", 4) == 0) {}
		else if(strncmp(cmd, "write", 5) == 0) {}
		else if(strncmp(cmd, "exit", 4) == 0) {
			break; // breaks out of loop
		} 
		else {
			fprintf(stderr, "ERROR: '%s' is not a supported command.\n", cmd);
		}
	}

	fclose(fp);
	return 0;
}
