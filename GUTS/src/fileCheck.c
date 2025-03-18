#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
int isFileCreated(const char* restrict filepath){
	FILE *file = fopen(filepath, "rb");
	DIR *dir = opendir(filepath);
	if(file == NULL){
		if(dir){
			closedir(dir);
			return 1;
		}
		return 0;
	} else {
		fclose(file);
		return 1;
	}
}
