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

	return cmdLine;
}

///////////////////////////////////////////////////////////////////////////////

int main(){
	char* cmd, temp;
	size_t result;
	bool exitBool = false;

	fp = fopen("fat32.img", "r+b");
	if(!fp){
		fputs("ERROR: File not found.\n", stderr);
		exit(1);
	}
	cmd = (char*) malloc (128);

	// This prints Boot Sector Information, and is commented
	// out for debugging.

	BootSectorInformation();
	// puts("*****************************");
	// 
	// PrintBPS();
	// printf("First Data Sector:\t%i\n", fds);
	// printf("Root Directory:\t\t%i\n", LocateFSC(root_clus));
	// puts("*****************************");


	// Main while loop
	while((cmd = getCmd()) != NULL) {
		cmd[strlen(cmd)-1] = '\0';
		// OPEN
		if(strncmp(cmd, "open", 4) == 0) {
			if(strlen(cmd) < 5){
				fprintf(stderr, "USAGE: open <FILE_NAME> <MODE>\n");
				continue;
			}
			char fileName[12];
			char mode[3];
			sscanf(cmd, "%*s %s %s", fileName, mode);
			// printf("FN: %s\tMODE: %s\n", fileName, mode);
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
		// CLOSE
		else if(strncmp(cmd, "close", 5) == 0) {
			if(strlen(cmd) < 5){
				fprintf(stderr, "USAGE: close <FILE_NAME>\n");
				continue;
			}
			char fileName[12];
			sscanf(cmd, "%*s %s", fileName);
			Close(fileName);
		}
		// CREATE
		else if(strncmp(cmd, "create", 6) == 0) {
			if(strlen(cmd) < 7){
				fprintf(stderr, "USAGE: create <FILE_NAME>\n");
				continue;
			}
			Create(&cmd[7]);
		}
		// REMOVE
		else if(strncmp(cmd, "rm", 2) == 0) {
			if(strlen(cmd) < 3){
				fprintf(stderr, "USAGE: rm <FILE_NAME>\n");
				continue;
			}
			Remove(&cmd[3]);
		}
		// SIZE
		else if(strncmp(cmd, "size", 4) == 0) {
			if(strlen(cmd) < 5){
				fprintf(stderr, "USAGE: size <FILE_SIZE>\n");
				continue;
			}
			PrintSize(&cmd[5]);
		}
		// CD
		else if(strncmp(cmd, "cd", 2) == 0) {
			if(strlen(cmd) < 3){
				fprintf(stderr, "USAGE: cd <DIR_NAME>\n");
				continue;
			}
			ChangeDirectory(&cmd[3]);
		}
		// LS
		else if(strncmp(cmd, "ls", 2) == 0) {
			if(strlen(cmd) < 3){
				fprintf(stderr, "USAGE: ls <DIR_NAME>\n");
				continue;
			}
			List(&cmd[3]);
		} 
		// MKDIR
		else if(strncmp(cmd, "mkdir", 5) == 0) {
			if(strlen(cmd) < 6){
				fprintf(stderr, "USAGE: mkdir <DIR_NAME>\n");
				continue;
			}
			MakeDir(&cmd[6]);
		}
		// RMDIR
		else if(strncmp(cmd, "rmdir", 5) == 0) {
			if(strlen(cmd) < 6){
				fprintf(stderr, "USAGE: rmdir <DIR_NAME>\n");
				continue;
			}
			RemoveDir(&cmd[6]);
		}
		// READ
		else if(strncmp(cmd, "read", 4) == 0) {
			if(strlen(cmd) < 5){
				fprintf(stderr, 
					"USAGE: read <FILE_NAME> <POS> <NUM_BYTES>\n");
				continue;
			}
			char fileName[12];
			int pos, size;
			sscanf(cmd, "%*s %s %i %i", fileName, &pos, &size);
			ReadFile(fileName, pos, size);
		}
		// WRITE
		else if(strncmp(cmd, "write", 5) == 0) {
			if(strlen(cmd) < 5){
				fprintf(stderr,
				 "USAGE: write <FILE_NAME> <POS> <NUM_BYTES> <STRING>\n");
				continue;
			}
			char fileName[12], msg[1024];
			int pos, size;
			sscanf(cmd, "%*s %s %i %i %s", fileName, &pos, &size, msg);
			WriteToFile(fileName, pos, size, msg);
		}
		// EXIT
		else if(strncmp(cmd, "exit", 4) == 0) {
			break; // breaks out of loop
		} 
		// ALL OTHERS
		else {
			fprintf(stderr, "ERROR: '%s' is not a supported command.\n", cmd);
		}
	}

	fclose(fp);
	return 0;
}
