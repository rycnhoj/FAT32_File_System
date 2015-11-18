#include <stdio.h>
#include <stdlib.h>

#define SHIFT_AMOUNT 8

unsigned int bytes_per_sec; // offset = 11; size = 2
unsigned int sec_per_clus;	// offset = 13; size = 1
unsigned int rsvd_sec_cnt;	// offset = 14; size = 2
unsigned int num_fats;		// offset = 16; size = 1
unsigned int fats_z32;		// offset = 36; size = 4
unsigned int root_clus;		// offset = 44; size = 4

void showbits(unsigned int x)
{
    int i; 
    for(i=(sizeof(int)*8)-1; i>=0; i--)
        (x&(1<<i))?putchar('1'):putchar('0');

	printf("\n");
}

unsigned int ExtractData(FILE* fp, int pos, int size){
	unsigned int data = 0, temp;
	int ch;
	int i;

	pos--;

	while(size > 0){
		fseek(fp, pos+size, SEEK_SET);
		size--;
		temp = fgetc(fp);
		data = data | temp;
		if(size != 0)
			data = data << SHIFT_AMOUNT;
	}
	return data;
}

void BootSectorInformation(FILE* fp){
	bytes_per_sec = ExtractData(fp, 11, 2);
	printf("BPS: %i\n", bytes_per_sec);
}

int main(){
	FILE* imgf;
	char* message;
	size_t result;
	int i;

	message = (char*) malloc(513);
	imgf = fopen("fat32.img", "rb");

	BootSectorInformation(imgf);

	fclose(imgf);

	return 0;
}