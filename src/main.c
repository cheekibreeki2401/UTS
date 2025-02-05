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
void load_list();

const char MENUCHOICES[]="\n(L)oad a list\n(E)dit current list\n(D)elete list\n(O)pen list\n(Q)uit CUTS\n";
char curr_list[256]="";
FILE *loaded_list;

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
		strcpy(curr_list, utsFilePath);
		loaded_list=fopen(curr_list, "rb");
	} else {
		printf("Have not found the file, creating file now...\n");
		FILE *main_list = fopen(utsFilePath, "wb");
		fprintf(main_list, "MAIN\n");
		fclose(main_list);
		printf("Checking if file has been created...\n");
		if(isFileCreated(utsFilePath)){
			strcpy(curr_list, utsFilePath);
			loaded_list = fopen(curr_list, "rb");
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
	int choice=0;
	while(choice != 'Q'){
		printf("Enter your choice: ");
		choice=getchar();
		if(!isalpha(choice)){
			printf("\nCould not read a valid choice, please try again\n%s", MENUCHOICES);
			continue;
		}
		if(toupper(choice)=='L'){
			getchar();
			load_list();
		}else if(toupper(choice)=='Q'){
			fclose(loaded_list);
			return 0;
		} else {
			printf("Invalid Choice specified, please try again\n%s", MENUCHOICES);
		}
	}
}

void load_list(){
	printf("\nPlease enter the file path to a .tfo file: ");
	char filePath[256];
	fgets(filePath, 256, stdin);
	char *point = filePath + strlen(filePath);
	if((point = strchr(filePath, '.'))!= NULL){
		if(!strcmp(point, ".tfo")){
			printf("Error: %s does not seem to be a .tfo (Tagging File Object) file, please try again\n%s", filePath, MENUCHOICES);
		return;	
		}
	}else{
		printf("\nError: Not a filepath, please enter one in\n%s", MENUCHOICES);
		return;
	}
	filePath[strcspn(filePath, "\n")] = 0;
	printf("%s\n", filePath);
	printf("%s\n", curr_list);
	printf("%i\n", strcmp(curr_list, filePath));
	if(strcmp(curr_list, filePath)==0){
		printf("Already loaded file located at %s.\n%s", filePath, MENUCHOICES);
		return;
	}
	if(isFileCreated(filePath)){
		printf("Closing current list...\n");
		fclose(loaded_list);
		printf("Opening list located at %s...\n", filePath);
		strcpy(curr_list, filePath);
		loaded_list = fopen(curr_list, "rb");
		return;
	} else {
		printf("\nError: %s is not a valid .tfo file. Would you like to make it?\n(Y/N): ", filePath);
		char choice = getchar();
		while(toupper(choice)!='Y' || toupper(choice)!='N'){
			getchar();
			printf("Error: Not a valid choice, please choose type Y or N.\n");
			printf("Make a new file at %s?: ", filePath);
			choice = getchar();
		}
		if(toupper(choice)=='Y'){
			fclose(loaded_list);
			FILE* temp_list = fopen(filePath, "wb");
			fprintf(temp_list, "NEW LIST\n");
			fclose(temp_list);
			if(isFileCreated(filePath)){
				printf("Succesfully created new list at %s\nLoading list...\n", filePath);
				strcpy(curr_list, filePath);
			} else {
				printf("Could not create list...\n");
			}
			getchar();
			loaded_list = fopen(curr_list, "rb");
		} else {
			printf("List not created, defaulting back to %s\n", curr_list);
			getchar();
		}
		printf("\n%s", MENUCHOICES);
		return;
	}
	printf("We got to the end for some reason...\n");
}
