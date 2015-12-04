#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

FILE* fp;

///////////////////////////////////////////////////////////////////////////////

int main(){
	char* message;
	size_t result;
	int i;

	message = (char*) malloc(513);
	fp = fopen("fat32.img", "rb");

	BootSectorInformation(imgf);
	PrintBPS();

	printf("First Data Sector:\t%i\n", fds);
	printf("Root Directory:\t\t%i\n", LocateFSC(root_clus));

	PrintDirectory(imgf, root_clus);

	fclose(imgf);

	return 0;
}