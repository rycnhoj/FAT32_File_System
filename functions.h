#define SHIFT_AMOUNT 8
#define END_OF_CLUSTER 0x0FFFFFF8
#define DIR_NAME_BYTES 11
#define DIR_ATTR_OFFSET 11
#define DIR_ATTR_BYTES 1
#define DIR_CLUS_HI_OFFSET 20
#define DIR_CLUS_LO_OFFSET 26
#define DIR_CLUS_BYTES 2
#define FILE_SIZE_OFFSET 28
#define FILE_SIZE_BYTES 4
#define EMPTY_FILE_NAME 0000000000
#define MAX_OPEN_FILES 64

#include <ctype.h>

unsigned int bytes_per_sec; // offset = 11; size = 2
unsigned int sec_per_clus;	// offset = 13; size = 1
unsigned int rsvd_sec_cnt;	// offset = 14; size = 2
unsigned int num_fats;		// offset = 16; size = 1
unsigned int fats_z32;		// offset = 36; size = 4
unsigned int root_clus;		// offset = 44; size = 4
unsigned int fds;			// First Data Sector

void ShowBits(unsigned int x);
unsigned int ExtractData(unsigned int pos, unsigned int size);
char* RemoveQuotes(char* str);
void BootSectorInformation();
void PrintBPS();
unsigned int PrintDirectory(unsigned int clus_num, int curr);

// File object stucture
typedef struct File{
	char file_name[12];
	unsigned int file_size;
	unsigned int file_attr;
	unsigned int first_clus_num;
	unsigned int mode;
} File;

// Directory object structure
typedef struct Directory{
	File files[MAX_OPEN_FILES];
	unsigned int num_files;
} Directory;

FILE* fp;					// Global file pointer
unsigned int curr_dir = 0;	// The current directory cluster number
Directory current_directory;	// The Directory object for the current
Directory open_files;			// The Directory object for the open files
unsigned int max_open_files;	// The maximum index of the open files


//////////////////////////////////////////////////////////////////////////////
//======== HELPER FUNCTIONS ===========//

// Checks if the nth bit is set
int CheckBitSet(unsigned char c, int n) {
    return ((1 << n) & c);
}

// Prints out the individual bits of an unsigned int
void ShowBits(unsigned int x) {
    int i; 
    for(i=(sizeof(int)*8)-1; i>=0; i--)
        (x&(1<<i))?putchar('1'):putchar('0');

	printf("\n");
}

// Prints out the individual bit of an unsigned char
void ShowCharBits(unsigned char x) {
    char i; 
    for(i=(sizeof(char)*8)-1; i>=0; i--)
        (x&(1<<i))?putchar('1'):putchar('0');

	printf("\n");
}

// Converts a cstring to all uppercase letters
char* StrUpr(char *str) {
  size_t i;
  size_t len = strlen(str);

  for(i=0; i<len; i++)
  	str[i]=toupper((unsigned char)str[i]);

  return str;
}

// Extracts data in little-endian order from pos to size
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
		//printf("DATA:\t%in", data);
	}
	return data;
}

// Removes quotes from a string
char* RemoveQuotes(char* str){
	char* ret;

	ret = &str[1];
	str[strlen(str)-1] = '\0';

	return ret;
}

// Removes periods from a string
char * RemovePeriods(char * str) {
    int i = 0;

    while (str[i] != '\0'){
        if (str[i] == '.')
            str[i] = ' ';
        i++;
    }

    return str;
}

//////////////////////////////////////////////////////////////////////////////
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

// Returns the Sector number of a cluster in the FAT table
unsigned int ThisFATSecNum(unsigned int clus_num){
	unsigned int FATOff;
	unsigned int tfsn;

	FATOff = clus_num * 4;
	tfsn = rsvd_sec_cnt + (FATOff / bytes_per_sec);
	return tfsn;
}

// Returns the offset of a cluster in the FAT table
unsigned int ThisFATEntOffset(unsigned int clus_num){
	unsigned int FATOff;

	FATOff = clus_num * 4;
	return FATOff % bytes_per_sec;
}

// Extracts all vital information from the boot sector
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

// Prints all vital information from the boot sector
void PrintBPS(){
	printf("Bytes Per Sector:\t%in", bytes_per_sec);
	printf("Sectors Per Cluster:\t%in", sec_per_clus);
	printf("Reserved Sector Count:\t%in", rsvd_sec_cnt);
	printf("Number of Fats:\tt%in", num_fats);
	printf("FATS Z32:\tt%in", fats_z32);
	printf("Root Cluster Address:\t%in", root_clus);
}

// Gets the decimal address of a sector
unsigned int GetSectorAddress(unsigned int sec_num){
	return sec_num * (bytes_per_sec * sec_per_clus);
}

// Gets all the clusters associated with a file
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

//////////////////////////////////////////////////////////////////////////////

// Sets the number of files in the current directory container to 0
void ClearCurrentDirectory(){
	current_directory.num_files = 0;
}

// Prints the contents of the passed in Directory object
void PrintDirectoryContents(Directory dir){
	int i;

	if(dir.num_files == 0){
		printf("The directory is empty.\n");
		return;
	}
	puts("================================");	
	for(i = 0; i < dir.num_files; i++){
		File* temp = &dir.files[i];
		if(!CheckBitSet(temp->file_attr, 4))
			printf("%s: ", "F");
		else
			printf("%s: ", "D");
		printf("%s - ", temp->file_name);
		printf("%i bytes ", temp->file_size);
		printf("(Cluster #: %i)\n", temp->first_clus_num);
	}
	puts("================================");	
}

// Prints the contents of the Open Files container
void PrintOpenFiles(Directory dir){
	int i;

	if(dir.num_files == 0){
		printf("There are no open files.\n");
		return;
	}
	puts("================================");	
	for(i = 0; i < max_open_files; i++){
		File* temp = &dir.files[i];
		if(temp->file_name[0] == '\0'){
			continue;
		}
		if(!CheckBitSet(temp->file_attr, 4))
			printf("%s: ", "F");
		else
			printf("%s: ", "D");
		printf("%s - ", temp->file_name);
		printf("%i bytes", temp->file_size);
		char* md;
		switch(temp->mode){
		case 1: md = "r"; break;
		case 2: md = "w"; break;
		case 3: md = "rw"; break;
		default: md = "error"; break;
		}
		printf(", in mode '%s'\n", md);
	}
	puts("================================");	
}

// Extracts contents of a Directory with the passed-in cluster number
Directory GetDirectoryContents(unsigned int clus_num){
	Directory dir_contents;
	unsigned int dir_fsc;
	unsigned int dir_adr;
	unsigned int dir_clus_hi_word;
	unsigned int dir_clus_lo_word;
	unsigned int dir_clus;
	unsigned char dir_name[12];
	unsigned char dir_attr;
	unsigned int i, j, c = 0, dir_size;
	char ch;
	// Locates the first sector of the root directory cluster;
	dir_fsc = LocateFSC(clus_num);
	dir_adr = GetSectorAddress(dir_fsc);

	for(i = 0; i < (bytes_per_sec*sec_per_clus); i+=32){
		File temp_file;
		// Extract Attributes
		dir_attr = ExtractData(dir_adr+i+DIR_ATTR_OFFSET, DIR_ATTR_BYTES);
		// Continues on long-name entries.
		if(dir_attr == 15)
			continue;

		// Extracts File name - max 11 chars
		for(j = 0; j < DIR_NAME_BYTES; j++){
			ch = ExtractData(dir_adr+i+j, 1);
			if(ch != 0)
				dir_name[j] = ch;
			else
				dir_name[j] = ' ';
		}
		dir_name[DIR_NAME_BYTES] = 0;
		if(dir_name[0] == ' ')
			continue;

		// Extracts File Size
		dir_size = ExtractData(dir_adr+i+FILE_SIZE_OFFSET, FILE_SIZE_BYTES);

		dir_clus_hi_word = ExtractData(dir_adr+i+DIR_CLUS_HI_OFFSET,
										DIR_CLUS_BYTES);
		dir_clus_lo_word = ExtractData(dir_adr+i+DIR_CLUS_LO_OFFSET,
										DIR_CLUS_BYTES);

		dir_clus_hi_word << 16;
		dir_clus = dir_clus_hi_word | dir_clus_lo_word;

		strcpy(temp_file.file_name, dir_name);
		temp_file.file_size = dir_size;
		temp_file.file_attr = dir_attr;
		temp_file.first_clus_num = dir_clus;
		dir_contents.files[c++] = temp_file;
	}
	dir_contents.num_files = c;
	return dir_contents;
}

// Searches for the first empty slot in a directory
File* SearchForFirstEmptyFileInDirectory(Directory dir){
	File* temp;
	int i;

	if(open_files.num_files == 0){
		return &open_files.files[0];
	}
	for(i = 0; i < 128; i++){
		temp = &dir.files[i];
		if(temp->file_name == NULL){
			return temp;
		}
	}
	return NULL;
}

// Returns the index for a certain file in the open files table
int SearchForFileInOpenFiles(char* file_name, Directory dir){
	File* temp;
	int i;
	for(i = 0; i < max_open_files; i++){
		temp = &dir.files[i];
		if(strncmp(file_name, temp->file_name, strlen(file_name)) == 0){
			return i;
		}
	}
	return -1;
}

// Searches for a file name in the current directory
File* SearchForFileInCurrentDirectory(char* file_name){
	File* temp;
	int i;
	for(i = 0; i < current_directory.num_files; i++){
		temp = &current_directory.files[i];
		if(strncmp(file_name, temp->file_name, strlen(file_name)) == 0){
			return temp;
		}
	}
	return NULL;
}

// Returns the address of the slot of a short-name file entry in a directory
int GetFileSlot(unsigned int clus_num, char* file_name){
	unsigned int dir_fsc;
	unsigned int dir_adr;
	unsigned char dir_name[12];
	unsigned int slot;
	int i, j;
	char ch;

	dir_fsc = LocateFSC(clus_num);
	dir_adr = GetSectorAddress(dir_fsc);

	for(i = 0; i < (bytes_per_sec*sec_per_clus); i+=32){
		// Extracts File name - max 11 chars
		for(j = 0; j < DIR_NAME_BYTES; j++){
			ch = ExtractData(dir_adr+i+j, 1);
			if(ch != 0)
				dir_name[j] = ch;
			else
				dir_name[j] = ' ';
		}
		dir_name[DIR_NAME_BYTES] = 0;
		if(strncmp(file_name, dir_name, strlen(file_name)) == 0)
			return dir_adr+i;
	}
	return -1;
}

// Returns the address of the next free slot of a directory
int GetNextOpenSlot(unsigned int clus_num){
	unsigned int dir_fsc;
	unsigned int dir_adr;
	unsigned int slot;
	int i;

	dir_fsc = LocateFSC(clus_num);
	dir_adr = GetSectorAddress(dir_fsc);

	for(i = 0; i < (bytes_per_sec*sec_per_clus); i+=32){
		slot = ExtractData(dir_adr+i, 1);
		if(slot == 0){
			return dir_adr+i;
		}
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////////
//========== MAIN FUNCTIONS ===========//
/* FOR ALL FUNCTIONS: Return 1 on success, 0 on error **/

// Adds a file to the open file table with the passed-in mode
unsigned int Open(char* file_name, char* mode){
	File *f, *empty;
	int temp;
	unsigned int modeInt; 
	if(curr_dir == 0){
		curr_dir = root_clus;
		current_directory = GetDirectoryContents(curr_dir);
	}
	file_name = StrUpr(file_name);
	if (strcmp(mode, "r") == 0) { modeInt = 1; }
	else if (strcmp(mode, "w") == 0) { modeInt = 2;}
	else if (strcmp(mode, "rw") == 0 || strcmp(mode, "wr") == 0) {
		modeInt = 3;
	}

	temp = SearchForFileInOpenFiles(file_name, open_files);
	if(temp != -1){
		char* md;
		switch(open_files.files[temp].mode){
		case 1: md = "r"; break;
		case 2: md = "w"; break;
		case 3: md = "rw"; break;
		default: md = "error"; break;
		}
		fprintf(stderr, 
			"ERROR: '%s' is already opened, with mode '%s'.\n",
			 file_name, md);
		return 0;		
	}

	f = SearchForFileInCurrentDirectory(file_name);
	if(f == NULL){
		fprintf(stderr, 
			"ERROR: '%s' does not exist in the current directory.\n",
			 file_name);
		return 0;
	}

	if(open_files.num_files == MAX_OPEN_FILES){
		fprintf(stderr, 
			"ERROR: You have reached the maximum number of open files.\n");
		return 0;
	}

	empty = &open_files.files[max_open_files];
	strcpy(empty->file_name, f->file_name);
	empty->file_attr = f->file_attr;
	empty->first_clus_num = f->first_clus_num;
	empty->mode = modeInt;
	open_files.num_files++; 
	max_open_files++;

	printf("'%s' has been successfully opened.\n", file_name);
	PrintOpenFiles(open_files);
}

// Removes a file from the open file table.
unsigned int Close(char* file_name){
	File* f;
	int temp;
	file_name = StrUpr(file_name);
	if(open_files.num_files == 0){
		fprintf(stderr, "ERROR: '%s' is not an open file.\n", file_name);
		return 0;
	}
	temp = SearchForFileInOpenFiles(file_name, open_files);
	if(temp == -1){
		fprintf(stderr, "ERROR: '%s' is not an open file.\n", file_name);
		return 0;
	}
	f = &open_files.files[temp];
	f->file_name[0] = '\0';
	f->file_size = 0;
	f->file_attr = 0;
	f->mode = -1;
	open_files.num_files--;

	printf("'%s' has been successfully closed.\n", file_name);
	PrintOpenFiles(open_files);
}

// Creates a file in the current directory
// NOTE: Does not properly assign values
unsigned int Create(char* file_name){
	File* f;
	char buffer[32];
	int slot_adr;

	if(curr_dir == 0){
		curr_dir = root_clus;
		current_directory = GetDirectoryContents(curr_dir);
	}
	file_name = StrUpr(file_name);
	file_name = RemovePeriods(file_name);
	
	f = SearchForFileInCurrentDirectory(file_name);

	if(f != NULL){
		fprintf(stderr,
			"ERROR: '%s' already exists in the current directory.\n",
			 file_name);
		return 0;
	}

	// sprintf(buffer, "%-11s%1c%s%s%s%s%s", file_name, '.',
	// 		"\00000000", "\00", "\0000","\05", "\0000");
	
	// slot_adr = GetNextOpenSlot(curr_dir);
	// if(slot_adr == -1){
	// 	fprintf(stderr, "ERROR: Directory full.\n");
	// 	return 0;
	// }

	// fseek(fp, slot_adr, SEEK_SET);
	// fwrite(buffer, 1, 32, fp);
	// fflush(fp);
}

// Deletes a file from the current working directory.
// NOTE: INCOMPLETE: Does not actually remove the file entry from the directory
unsigned int Remove(char* file_name){
	File* f;
	char buffer[32];
	int slot_adr;

	if(curr_dir == 0){
		curr_dir = root_clus;
		current_directory = GetDirectoryContents(curr_dir);
	}
	file_name = StrUpr(file_name);
	file_name = RemovePeriods(file_name);
	f = SearchForFileInCurrentDirectory(file_name);
	if(f == NULL){
		fprintf(stderr,
			"ERROR: '%s' does not exist in the current directory.\n",
			 file_name);
		return 0;
	}

	printf("Theoretically, there is where we would remove the file.\n");

	// Our strategy here was to overwrite the short-name entry with all 0s,
	// but we have commented out the code in order to avoid corrupting 
	// the image file.

	// sprintf(buffer, 
	// 	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
	
	// slot_adr = GetFileSlot(curr_dir, file_name);
	// if(slot_adr == -1){
	// 	fprintf(stderr, "ERROR: File not found.\n");
	// 	return 0;
	// }

	// fseek(fp, slot_adr, SEEK_SET);
	// fwrite(buffer, 1, 32, fp);
	// fflush(fp);
}

// Prints the size of a file to screen
unsigned int PrintSize(char* file_name){
	File* f;
	if(curr_dir == 0){
		curr_dir = root_clus;
		current_directory = GetDirectoryContents(curr_dir);
	}
	file_name = StrUpr(file_name);
	f = SearchForFileInCurrentDirectory(file_name);
	if(f == NULL){
		fprintf(stderr,
			"ERROR: '%s' could not be found in the current directory.\n",
			 file_name);
		return 0;
	}
	printf("Size of file '%s': %i bytes.\n", file_name, f->file_size);
	return 1;
}

// Changes the current working directory if it is a child of the
// the previous working directory
unsigned int ChangeDirectory(char* dir_name){
	File* next_dir;
	if(curr_dir == 0){
		curr_dir = root_clus;
		current_directory = GetDirectoryContents(curr_dir);
	}
	dir_name = StrUpr(dir_name);
	next_dir = SearchForFileInCurrentDirectory(dir_name);
	if(next_dir == NULL){
		fprintf(stderr,
			"ERROR: '%s' could not be found in the current directory.\n",
			 dir_name);
		return 0;
	}
	else if(!CheckBitSet(next_dir->file_attr, 4)){
		fprintf(stderr, "ERROR: '%s' is not a directory.\n", dir_name);
		return 0;
	}

	curr_dir = next_dir->first_clus_num;
	ClearCurrentDirectory();
	current_directory = GetDirectoryContents(curr_dir);
}

// Performs the ls function on the passed-in directory name
unsigned int List(char* dir_name){
	int i;
	Directory dir;
	File* next_dir;

	if(curr_dir == 0){
		curr_dir = root_clus;
		current_directory = GetDirectoryContents(curr_dir);
	}
	fflush(fp);
	dir_name = StrUpr(dir_name);
	if(current_directory.num_files == 0){
		fprintf(stderr, "There are no files in the current directory.\n");
		return 0;
	}
	if(curr_dir = root_clus && strcmp(dir_name, ".") == 0){
		PrintDirectoryContents(current_directory);
		return 1;
	}
	next_dir = SearchForFileInCurrentDirectory(dir_name);
	if(next_dir == NULL){
		fprintf(stderr,
			"ERROR: '%s' could not be found in the current directory.\n",
			 dir_name);
		return 0;
	}
	else if(!CheckBitSet(next_dir->file_attr, 4)){
		fprintf(stderr, "ERROR: '%s' is not a directory.\n", dir_name);
		return 0;
	}
	PrintDirectoryContents(GetDirectoryContents(next_dir->first_clus_num));
}

// Creates a Directory in the current working directory
// INCOMPLETE: Does not actually add entries to the file system
// Only throws error in the case that the entry already exists
unsigned int MakeDir(char* dir_name){
	File* new_dir;
	if(curr_dir == 0){
		curr_dir = root_clus;
		current_directory = GetDirectoryContents(curr_dir);
	}
	dir_name = StrUpr(dir_name);
	new_dir = SearchForFileInCurrentDirectory(dir_name);
	if(new_dir != NULL)	{
		fprintf(stderr,
			"ERROR: '%s' already exists in the current directory.\n",
			 dir_name);
		return 0;
	}

	// ADD DIRECTORY ENTRY IN HERE
}

// Removes an empty directory from the current working directory
// INCOMPLETE: Does not actually remove entries. 
// Currently only checks if the passed-in file name exists
// or is a file.
unsigned int RemoveDir(char* dir_name){
	File* new_dir;
	if(curr_dir == 0){
		curr_dir = root_clus;
		current_directory = GetDirectoryContents(curr_dir);
	}
	dir_name = StrUpr(dir_name);
	new_dir = SearchForFileInCurrentDirectory(dir_name);
	if(new_dir == NULL)	{
		fprintf(stderr,
			"ERROR: '%s' does not exist in the current directory.\n",
			 dir_name);
		return 0;
	}
	else if(!CheckBitSet(new_dir->file_attr, 4)){
		fprintf(stderr, "ERROR: '%s' is not a directory.\n", dir_name);
		return 0;
	}
	// UP TO HERE, CHECKS FOR EXISTENCE AND IF IS DIRECTORY
	// NEED TO CHECK IF DIRECTORY IS EMPTY

	// REMOVE DIRECTORY HERE
}

// Reads a file with name 'file_name' at position 'pos' with size 'size'.
// INCOMPLETE: If spanning more than one sector, does not currently jump 
// to next sector.
unsigned int ReadFile(char* file_name, unsigned int pos, unsigned int size){
	File *temp;
	char msg[size+1];
	int open, file_begin;

	if(curr_dir == 0){
		curr_dir = root_clus;
		current_directory = GetDirectoryContents(curr_dir);
	}

	if(pos > 512){
		fprintf(stderr,
			"ERROR: '%d' is outside the 'read' limitations of 512 bytes.\n",
			 pos);
		return 0;
	}

	file_name = StrUpr(file_name);
	temp = SearchForFileInCurrentDirectory(file_name);
	if(temp == NULL){
		fprintf(stderr,
			"ERROR: '%s' does not exist in the current directory.\n",
			 file_name);
		return 0;
	}
	open = SearchForFileInOpenFiles(file_name, open_files);
	if(open == -1){
		fprintf(stderr,
			"ERROR: '%s' has not been opened.\n", file_name);
		return 0;
	}

	if(open_files.files[open].mode == 2){
		fprintf(stderr,
			"ERROR: '%s' has not been opened in read-mode.\n", file_name);
		return 0;
	}

	LocateFSC(temp->first_clus_num);
	file_begin = GetSectorAddress(LocateFSC(temp->first_clus_num));

	if(pos+size > 512){
		fprintf(stderr, "WARNING: Read bound limit reached.\n");
		size = 512 - pos;
	}

	fseek(fp, file_begin+pos, SEEK_SET);

	fread(msg, 1, size, fp);

	printf("%s\n", msg);
}

// Writes 'msg' to address associated with passed-in 'file_name'
// INCOMPLETE: Does not account for message sizes greater than one cluster.
unsigned int WriteToFile(char* file_name, unsigned int pos, unsigned int size,
	char* msg){

	File *temp, *open_temp;
	int open, file_begin;

	if(curr_dir == 0){
		curr_dir = root_clus;
		current_directory = GetDirectoryContents(curr_dir);
	}

	if(pos > 512){
		fprintf(stderr,
			"ERROR: '%d' is outside the 'Write' limitations of 512 bytes.\n",
			 pos);
		return 0;
	}

	file_name = StrUpr(file_name);
	temp = SearchForFileInCurrentDirectory(file_name);
	if(temp == NULL){
		fprintf(stderr,
			"ERROR: '%s' does not exist in the current directory.\n",
			 file_name);
		return 0;
	}
	open = SearchForFileInOpenFiles(file_name, open_files);
	if(open == -1){
		fprintf(stderr,
			"ERROR: '%s' has not been opened.\n", file_name);
		return 0;
	}

	if(open_files.files[open].mode == 1){
		fprintf(stderr,
			"ERROR: '%s' has not been opened in read-mode.\n", file_name);
		return 0;
	}

	msg = RemoveQuotes(msg);

	LocateFSC(temp->first_clus_num);

	file_begin = GetSectorAddress(LocateFSC(temp->first_clus_num));

	if(pos+size > 512){
		fprintf(stderr, "WARNING: 'Write' bound limit reached.\n");
		size = 512 - pos;
	}

	fseek(fp, file_begin+pos, SEEK_SET);
	fwrite(msg, 1, size, fp);
}
