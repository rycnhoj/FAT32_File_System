#include <stdio.h>
#include <stdlib.h>

#define SHIFT_AMOUNT 8
//#define END_OF_CLUSTER 131072
#define END_OF_CLUSTER 0x0FFFFFF8

unsigned int bytes_per_sec; // offset = 11; size = 2
unsigned int sec_per_clus;	// offset = 13; size = 1
unsigned int rsvd_sec_cnt;	// offset = 14; size = 2
unsigned int num_fats;		// offset = 16; size = 1
unsigned int fats_z32;		// offset = 36; size = 4
unsigned int root_clus;		// offset = 44; size = 4
unsigned int fds;			// First Data Sector

void showbits(unsigned int x) {
    int i; 
    for(i=(sizeof(int)*8)-1; i>=0; i--)
        (x&(1<<i))?putchar('1'):putchar('0');

	printf("\n");
}

///////////////////////////////////////////////////////////////////////////////

unsigned int ExtractData(FILE* fp, unsigned int pos, unsigned int size){
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

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

void BootSectorInformation(FILE* fp){
	unsigned int rds;	// Root Directory Sectors

	bytes_per_sec = ExtractData(fp, 11, 2);
	sec_per_clus = ExtractData(fp, 13, 1);
	rsvd_sec_cnt = ExtractData(fp, 14, 2);
	num_fats = ExtractData(fp, 16, 1);
	fats_z32 = ExtractData(fp, 36, 4);
	root_clus = ExtractData(fp, 44, 4);

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

///////////////////////////////////////////////////////////////////////////////


unsigned int GetSectorAddress(unsigned int sec_num){
	return sec_num * (bytes_per_sec * sec_per_clus);
}

unsigned int GetAllClustersOfDirectory(unsigned int cluster_num, FILE* fp){
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
		next_clus = ExtractData(fp, GetSectorAddress(tfsn)+tfeo, 8);
	}
}

unsigned int PrintDirectory(FILE* fp, unsigned int clus_num){
	unsigned int dir_fsc;
	unsigned int dir_adr;
	unsigned int dir_name;
	unsigned int dir_attr;

	// Gets all the clusters of the root directory
	GetAllClustersOfDirectory(clus_num, fp);

	// Locates the first sector of the root directory cluster;
	dir_fsc = LocateFSC(clus_num);
	dir_adr = GetSectorAddress(dir_fsc);

	/* DOES NOT WORK YET */
	/* NEED TO STUDY PRINTING NAME */
	/* CHECK EXTRACT DATA IF EXTRACTING CORRECT DATA */
	/* POSSIBLY MAKE PRINT BIT BY BIT */
	dir_name = ExtractData(fp, dir_adr, 11);
	dir_attr = ExtractData(fp, dir_adr+11, 1);

	printf("DIR NAME:\t%i\n", dir_name);
	printf("DIR ATTR:\t%i\n", dir_attr);

}

///////////////////////////////////////////////////////////////////////////////

int main(){
	FILE* imgf;
	char* message;
	size_t result;
	int i;

	message = (char*) malloc(513);
	imgf = fopen("fat32.img", "rb");

	BootSectorInformation(imgf);
	PrintBPS();

	printf("First Data Sector:\t%i\n", fds);
	printf("Root Directory:\t\t%i\n", LocateFSC(root_clus));

	PrintDirectory(imgf, root_clus);

	fclose(imgf);

	return 0;
}