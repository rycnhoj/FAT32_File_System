
#define SHIFT_AMOUNT 8
//#define END_OF_CLUSTER 131072
#define END_OF_CLUSTER 0x0FFFFFF8

FILE* fp;
unsigned int curr_dir;

unsigned int bytes_per_sec; // offset = 11; size = 2
unsigned int sec_per_clus;	// offset = 13; size = 1
unsigned int rsvd_sec_cnt;	// offset = 14; size = 2
unsigned int num_fats;		// offset = 16; size = 1
unsigned int fats_z32;		// offset = 36; size = 4
unsigned int root_clus;		// offset = 44; size = 4
unsigned int fds;			// First Data Sector

void showbits(unsigned int x);
unsigned int ExtractData(unsigned int pos, unsigned int size);
char* RemoveQuotes(char* str);
void BootSectorInformation();
void PrintBPS();
unsigned int PrintDirectory(unsigned int clus_num);
unsigned int GetSectorAddress(unsigned int sec_num);

unsigned int addToFileList (char * fileName, int mode);
char * strupr(char *str);
unsigned int GetSectorNum(unsigned int sec_adr);
unsigned int getLocation(char * fileName);

void removeFile(unsigned int location, int index);
unsigned int Close(char* fileName);
void printFileTable ();

struct File {
	char fileName[128];
	unsigned int location;
	unsigned int firstCluster;
	int mode;
	// Other relevant information
};

struct File fileTable[128];

int numberOfFiles;

///////////////////////////////////////////////////////////////////////////////
//======== HELPER FUNCTIONS ===========//

void showbits(unsigned int x) {
    int i; 
    for(i=(sizeof(int)*8)-1; i>=0; i--)
        (x&(1<<i))?putchar('1'):putchar('0');

	printf("\n");
}

unsigned int ExtractData(unsigned int pos, unsigned int size){
	unsigned int data = 0, temp;
	int ch;
	int i;

	pos--;

	while(size > 0){
		fseek(fp, pos, SEEK_SET);
		fseek(fp, size, SEEK_CUR);
		size--;
		temp = fgetc(fp);
		data = data | temp;
		if(size != 0)
			data = data << SHIFT_AMOUNT;
		//printf("DATA:\t%i\n", data);
	}
	return data;
}

char* RemoveQuotes(char* str){
	char* ret;

	ret = &str[1];
	str[strlen(str)-1] = '\0';

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//======= FILE SYSTEM FUNCTIONS =======//

/** 
* LocateFSC
* Locates and returns the address of the first data sector of the passed-in
* cluster.
*/
unsigned int LocateFSC(unsigned int clus_num){
	unsigned int fsc;
	fsc = ((clus_num - 2) * sec_per_clus) + fds;
	return fsc;
}

unsigned int ThisFATSecNum(unsigned int clus_num){
	unsigned int FATOff;
	unsigned int tfsn;

	FATOff = clus_num * 4;	// This is due to FAT32; check pg. 15 in MS writeup
	tfsn = rsvd_sec_cnt + (FATOff / bytes_per_sec);
	return tfsn;
}

unsigned int ThisFATEntOffset(unsigned int clus_num){
	unsigned int FATOff;

	FATOff = clus_num * 4;	// This is due to FAT32; check pg. 15 in MS writeup
	return FATOff % bytes_per_sec;
}

void BootSectorInformation(){
	unsigned int rds;	// Root Directory Sectors

	bytes_per_sec = ExtractData(11, 2);
	sec_per_clus = ExtractData(13, 1);
	rsvd_sec_cnt = ExtractData(14, 2);
	num_fats = ExtractData(16, 1);
	fats_z32 = ExtractData(36, 4);
	root_clus = ExtractData(44, 4);

	// This calculates root directory sectors and first data sector
	rds = 0;	// 0 for FAT32
	fds = rsvd_sec_cnt + (num_fats * fats_z32) + rds;
}

//////////////////////////////////////////////////////////////////////////////
// Prints output with data related to the FAT32 img
void PrintBPS(){
	printf("Bytes Per Sector:\t%i\n", bytes_per_sec);
	printf("Sectors Per Cluster:\t%i\n", sec_per_clus);
	printf("Reserved Sector Count:\t%i\n", rsvd_sec_cnt);
	printf("Number of Fats:\t\t%i\n", num_fats);
	printf("FATS Z32:\t\t%i\n", fats_z32);
	printf("Root Cluster Address:\t%i\n", root_clus);
}

//////////////////////////////////////////////////////////////////////////////
// calculates the byte location based on the sector number passed in 
// the sec_num is multiplied by both the num of bytes per sector and cluster
unsigned int GetSectorAddress(unsigned int sec_num){
	return sec_num * (bytes_per_sec * sec_per_clus);
}

unsigned int GetSectorNum(unsigned int sec_adr){
	return (sec_adr / bytes_per_sec) / sec_per_clus;
}

unsigned int GetAllClustersOfDirectory(unsigned int cluster_num){
	unsigned int next_clus;
	unsigned int tfsn;
	unsigned int tfeo;

	next_clus = cluster_num;

	while(next_clus != END_OF_CLUSTER){
		// Returns the FAT sector number of this cluster
		tfsn = ThisFATSecNum(root_clus);

		// Returns the FAT Offset of this cluster
		tfeo = ThisFATEntOffset(root_clus);

		// Extracts the data for the next cluster of this directory
		next_clus = ExtractData(GetSectorAddress(tfsn)+tfeo, 8);
	}
}

unsigned int PrintDirectory(unsigned int clus_num){
	unsigned int dir_fsc;
	unsigned int dir_adr;
	unsigned char dir_name[12];
	unsigned char dir_attr;
	unsigned int i, j, c = 0;
	char ch;

	// Gets all the clusters of the root directory
	GetAllClustersOfDirectory(clus_num);
	// Locates the first sector of the root directory cluster;
	dir_fsc = LocateFSC(clus_num);
	dir_adr = GetSectorAddress(dir_fsc);

	puts("================================");
	printf("%u\n", dir_adr);

	for(i = 0; i < 512; i+=32){
		dir_attr = ExtractData(dir_adr+i+11, 1);
		if(dir_attr == 15)
			continue;
		for(j = 0; j < 11; j++){
			ch = ExtractData(dir_adr+i+j, 1);
			if(ch != 0)
				dir_name[j] = ch;
			else
				dir_name[j] = ' ';
		}
		dir_name[11] = 0;
		if(dir_name[0] == ' ')
			continue;
			
		printf("%s\n", dir_name);

		// printf("ATTR:\t");
		
		// printf("NAME:\t%s\n", dir_name);
		// printf("ATTR:\t");
		
		// showbits(dir_attr);
	}

	puts("================================");
}

///////////////////////////////////////////////////////////////////////////////
//========== MAIN FUNCTIONS ===========//
/* FOR ALL FUNCTIONS: Return 1 on success, 0 on error **/

////////////// ALL JOHN

//////////////////////////////////////////////////////////////////////////////
// open <FILE_NAME> <MODE>
//////////////////////////////////////////////////////////////////////////////
// A file can only be read from or written to if it is opened first.
// You may need to maintain a table of opened files and add
// FILE_NAME to it when open function is called.
//////////////////////////////////////////////////////////////////////////////

// TO DO 

// Write a helper functions call fileExists() that returns 1 or 0 true false 


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void printFileTable () {
		printf("-------------------------------\n");
		for (int i = 0; i < numberOfFiles; ++i) {
			if(fileTable[i].fileName != NULL && 
			   fileTable[i].location != NULL)
				printf("%s\t: %u \t: %d\n",
					fileTable[i].fileName, 
					fileTable[i].location, 
					fileTable[i].mode);
		}
		printf("-------------------------------\n");
}


unsigned int Open(char* file_name, char* mode){
	int modeInt = -1;

	if (mode == NULL) { modeInt = 0; } else {
		if (strcmp(mode, "r") == 0) { modeInt = 1; }
		if (strcmp(mode, "w") == 0) { modeInt = 2;}
		if (strcmp(mode, "rw") == 0 || strcmp(mode, "wr") == 0) 
			{ modeInt = 3;}
	}

	// if(modeInt <= 0){
	// 	fprintf(stderr, "ERROR: %s - invalid mode.\n", mode);
	// 	return 0;
	// }

	// Opens a file named FILE_NAME if in the present working directory.
	// Check if file exists in current directory
	int results = addToFileList(file_name, modeInt);
	
	if( results == 1) { 
		printFileTable();
	} else if (results == 2) {
		fprintf(stderr,
			"ERROR: %s - file is already open.\n",
			file_name);
	} else if (results == 0) {
		fprintf(stderr,
		"ERROR: %s - file not in current directory.\n",
		file_name);
	}
}

unsigned int addToFileList (char * fileName, int mode) {
	unsigned int dir_adr;
	unsigned char dir_name[12];
	unsigned int location;
	unsigned char dir_attr;
	unsigned int i, j, c = 0;
	char ch;

	char temp_name[12] = "           ";

	fileName = strupr(fileName); // UPPERCASE 

	//////////////////////////////////////////////////////////////////////////
	// HARD CODED TO LOOK AT ROOT
	//////////////////////////////////////////////////////////////////////////

	dir_adr = GetSectorAddress(2050);

	for(i = 0; i < 512; i+=32) {
		dir_attr = ExtractData(dir_adr+i+11, 1);
		if(dir_attr == 15)
			continue;
		for(j = 0; j < 11; j++) {
			ch = ExtractData(dir_adr+i+j, 1); 
			if(ch != 0) dir_name[j] = ch;
			else dir_name[j] = ' ';
		}
		dir_name[11] = 0; // add null character to end of dirname
		
		if(dir_name[0] == ' ') continue;

		///////////////////////////////////////////////////////////////
		
		location = dir_adr + i; // Get the offset value 

		strncpy(temp_name, fileName, strlen(fileName));

		if (strcmp(dir_name, temp_name) == 0) {
			// look at the location of file store information
			for (int i = 0; i < numberOfFiles; ++i) {
				if (location == fileTable[i].location) {
					return 2; // exit function
				}
			}

			strcpy(fileTable[numberOfFiles].fileName, temp_name);

			fileTable[numberOfFiles].location = location;
			fileTable[numberOfFiles].mode = mode;
			numberOfFiles++; // increments the number of files open 

			return 1;
		} 

		///////////////////////////////////////////////////////////////
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// Pass in a string and returns an uppercase version
char * strupr(char *str) {
  size_t i;
  size_t len = strlen(str);

  for(i=0; i<len; i++)
  	str[i]=toupper((unsigned char)str[i]);

  return str;
}

//////////////////////////////////////////////////////////////////////////////
// close <FILE_NAME>
//////////////////////////////////////////////////////////////////////////////
// Closes a file. Return an error if the file is already closed.
// Depending on your implementation, you may need to remove
// FILE_NAME from the table of opened files.
//////////////////////////////////////////////////////////////////////////////
unsigned int Close(char* fileName) {
	int fileClosed = 0;

	// get the location of file ---- SHOULD BE BASED ON THE CURRENT DIRRR
	unsigned int location = getLocation(fileName);
	// Check if Open
	if (location != NULL) { // The File does not exist in directory
		for (int i = 0; i < numberOfFiles; ++i) {
			if (location == fileTable[i].location) {
				removeFile(location, i);
				fileClosed = 1;
			}
		}
	}

	if (fileClosed) {
		printf("%s was successfully closed.\n", fileName);
		printFileTable();
	} else 
		printf("%s is not open.\n", fileName);
}

void removeFile(unsigned int location, int index) {
	numberOfFiles--;

	printf("%s\n", fileTable[index].fileName);
	for ( int i = index ; i < numberOfFiles; i++ ) {
		strcpy(fileTable[i].fileName, fileTable[i+1].fileName);
		fileTable[i].location = fileTable[i+1].location;
		fileTable[i].mode = fileTable[i+1].mode;
	}	
}

unsigned int getLocation(char * fileName) {
	unsigned int dir_adr;
	unsigned char dir_name[12];
	unsigned int location;
	unsigned char dir_attr;
	unsigned int i, j, c = 0;
	char ch;


	dir_adr = GetSectorAddress(2050);

	for(i = 0; i < 512; i+=32) {
		dir_attr = ExtractData(dir_adr+i+11, 1);
		if(dir_attr == 15)
			continue;
		for(j = 0; j < 11; j++) {
			ch = ExtractData(dir_adr+i+j, 1); 
			if(ch != 0) dir_name[j] = ch;
			else dir_name[j] = ' ';
		}
		dir_name[11] = 0; // add null character to end of dirname
		
		if(dir_name[0] == ' ') continue;

		if (strncmp(dir_name, fileName, strlen(fileName)) == 0)
			return dir_adr+i;
	}
}
//////////////////////////////////////////////////////////////////////////////
// create <FILE_NAME>
//////////////////////////////////////////////////////////////////////////////
// Creates a file in the present working directory of 
// 0 bytes with name FILE_NAME.
// Return an error if a file with that name already exists.
//////////////////////////////////////////////////////////////////////////////
unsigned int Create(char* file_name){
	printf("Create\n");

}
