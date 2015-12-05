#define SHIFT_AMOUNT 8
#define END_OF_CLUSTER 0x0FFFFFF8
#define DIR_NAME_BYTES 11
#define DIR_ATTR_OFFSET 11
#define DIR_ATTR_BYTES 1
#define DIR_CLUS_HI_OFFSET 20
#define DIR_CLUS_LOW_OFFSET 26
#define DIR_CLUS_BYTES 2
#define FILE_SIZE_OFFSET 28
#define FILE_SIZE_BYTES 4

FILE* fp;
unsigned int curr_dir;

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
unsigned int PrintDirectory(unsigned int clus_num);


///////////////////////////////////////////////////////////////////////////////
//======== HELPER FUNCTIONS ===========//

int CheckBitSet(unsigned char c, int n) {
    return ((1 << n) & c);
}

void ShowBits(unsigned int x) {
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

void PrintBPS(){
	printf("Bytes Per Sector:\t%i\n", bytes_per_sec);
	printf("Sectors Per Cluster:\t%i\n", sec_per_clus);
	printf("Reserved Sector Count:\t%i\n", rsvd_sec_cnt);
	printf("Number of Fats:\t\t%i\n", num_fats);
	printf("FATS Z32:\t\t%i\n", fats_z32);
	printf("Root Cluster Address:\t%i\n", root_clus);
}

unsigned int GetSectorAddress(unsigned int sec_num){
	return sec_num * (bytes_per_sec * sec_per_clus);
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
	unsigned int i, j, c = 0, dir_size;
	char ch;

	// Gets all the clusters of the root directory
	GetAllClustersOfDirectory(clus_num);

	// Locates the first sector of the root directory cluster;
	dir_fsc = LocateFSC(clus_num);
	dir_adr = GetSectorAddress(dir_fsc);

	puts("================================");

	for(i = 0; i < (bytes_per_sec*sec_per_clus); i+=32){
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

		if(!CheckBitSet(dir_attr, 4))
			printf("%s: ", "F");
		else
			printf("%s: ", "D");
		printf("%s\n", dir_name);

		dir_size = ExtractData(dir_adr+i+FILE_SIZE_OFFSET, FILE_SIZE_BYTES);
	}

	puts("================================");
}

///////////////////////////////////////////////////////////////////////////////
//========== MAIN FUNCTIONS ===========//
/* FOR ALL FUNCTIONS: Return 1 on success, 0 on error **/

////////////// ALL JOHN
unsigned int Open(char* file_name, char* mode){
	char flags[3] = "rw";
	int r = 0;
	int w = 0;

	if(strpbrk(mode, flags) == NULL){
		fprintf(stderr, "ERROR: %s - invalid mode.\n", mode);
		return 0;
	}
	if(strchr(mode, 'r') != NULL)
		r = 1;
	if(strchr(mode, 'w') != NULL)
		w = 1;

}

unsigned int Close(char* file_name){

}

unsigned int Create(char* file_name){

}

//////////////

// ABE
unsigned int Remove(char* file_name){

}

////////////// ALL EVAN

unsigned int PrintSize(char* file_name){

}

unsigned int ChangeDirectory(char* dir_name){

}

unsigned int List(char* dir_name){
	unsigned int cluster;
	// Parse name
	// Name to cluster
	cluster = root_clus;
	PrintDirectory(cluster);
}

unsigned int MakeDir(char* dir_name){

}

unsigned int RemoveDir(char* dir_name){

}

//////////////

// ABE
unsigned int ReadFile(char* file_name, unsigned int pos, unsigned int size){

}

// ABE
unsigned int WriteToFile(char* file_name, unsigned int pos, unsigned int size,
	char* msg){
}
