#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
int isFileCreated(const char* restrict filepath){
	FILE *file = fopen(filepath, "rb");
	if(file == NULL){
		return 0;
	} else {
		fclose(file);
		return 1;
	}
}
