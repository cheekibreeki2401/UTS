#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include "headers/fileCheck.h"
#include "headers/folderCheck.h"
#include "headers/curUser.h"

int menu();

const char MENUCHOICES[]="\n(L)oad a list\n(E)dit current list\n(D)elete list\n(O)pen list\n(Q)uit CUTS\n";

int main(){
	printf("Welcome to CUTS!\nJust checking to see if you already have a folder for UTS data...\n");
	char utsFilePath[256]="";
	#ifdef __linux__
		strcat(utsFilePath, "/home/");
	#elif	_WIN32
		strcat(utsFilePath, "C:/Users/");
	#else
		printf("Unsupported OS, sorry..."\n);
		return 1;
	#endif
	if(getUserName() == "ERR_NO_NAME"){
		printf("Hmmm... seems you aren't a real user...");
		return 1;
	}
	strcat(utsFilePath, getUserName());
	strcat(utsFilePath, "/Documents/UTS Util");
	if(isFolderCreated(utsFilePath)){
		printf("Folder has been found!\n");
	} else {
		printf("No folder has been created yet...\nCreating one now!\n");
		mkdir(utsFilePath, 0700);
		printf("Checking that worked...\n");
		if(isFolderCreated(utsFilePath)){
			printf("Succesfully Created!\n");
		} else {
			printf("DISASTER!\n");
			return 1;
		}
	}
	printf("Checking if the main list exists...\n");
	strcat(utsFilePath, "/main_list.tfo");
	if(isFileCreated(utsFilePath)){
		printf("Succesfully found main_list.tfo\n");
	} else {
		printf("Have not found the file, creating file now...\n");
		FILE *main_list = fopen(utsFilePath, "wb");
		fprintf(main_list, "MAIN\n");
		fclose(main_list);
		printf("Checking if file has been created...\n");
		if(isFileCreated(utsFilePath)){
			printf("Succesfully created main_list.tfo!\nDo note not to delete this or it will be created again\n");
		} else {
			printf("Nope...\n");
			return 1;
		}
	}
	menu();
	return 0;
}

int menu(){
	printf("Main Menu: (Note: Currently loaded list is main_list.tfo. %s", MENUCHOICES);
	char choice;
	while(toupper(choice) != 'Q'){
		printf("Enter your choice: ");
		scanf(" %1c",&choice);
		if(!isalpha(choice)){
			printf("\nCould not read a valid choice, please try again\n%s", MENUCHOICES);
			continue;
		}
		if(toupper(choice)=='L'){
			printf("Reading list\n");
		}else if(toupper(choice)=='Q'){
			return 0;
		} else {
			printf("Invalid Choice specified, please try again\n%s", MENUCHOICES);
		}
	}
}
