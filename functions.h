#include <string.h>

//======== HELPER FUNCTIONS ===========//

char* RemoveQuotes(char* str){
	char* ret;

	ret = str[1];
	str[strlen(str)-1] = '\0';

	return ret;
}

//========== MAIN FUNCTIONS ===========//
/* FOR ALL FUNCTIONS: Return 1 on success, 0 on error **/

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

unsigned int Remove(char* file_name){

}

unsigned int PrintSize(char* file_name){

}

unsigned int ChangeDirectory(char* dir_name){

}

unsigned int List(char* dir_name){

}

unsigned int MakeDir(char* dir_name){

}

unsigned int RemoveDir(char* dir_name){

}

unsigned int ReadFile(char* file_name, unsigned int pos, unsigned int size){

}

unsigned int WriteToFile(char* file_name, unsigned int pos, unsigned int size,
	char* msg){

}
