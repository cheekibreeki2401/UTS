#include <stdio.h>
#include <stdlib.h>
#include "headers/fileCheck.h"

int main(){
	if(isFolderCreated("/home/nomed/Documents/test.txt")){
		printf("File has been found\n");
	} else {
		printf("No file found\n");
	}
}
