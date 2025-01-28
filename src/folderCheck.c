#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
int isFolderCreated(const char* restrict directory){
	DIR *dir = opendir(directory);
	if(dir){
		closedir(dir);
		return 1;
	} else {
		closedir(dir);
		return 0;
	}
}
