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

#ifdef __linux__
	int MAX_SIZE = 4096;
#elif _WIN32
	int MAX_SIZE = 260;
#else 
	int MAX_SIZE = 256;
#endif

typedef struct taggedFile{
	int index;
	char[MAX_SIZE] filePath;
	char* tags[9];
	struct taggedFile *next_file;
}

int menu();
void loadList();
void editList();
void addFile();
void printListContents();
void removeFile();
void newTag();
void deleteTag();
void renameList();

const char MENUCHOICES[]="\n(L)oad a list\n(E)dit current list\n(D)elete list\n(O)pen list\n(Q)uit CUTS\n";
char curr_list[MAX_SIZE]="";
FILE *loaded_list;

int main(){
	printf("Welcome to CUTS!\nJust checking to see if you already have a folder for UTS data...\n");
	char utsFilePath[MAX_SIZE]="";
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
		fprintf(main_list, "Main");
		fwrite();
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
		getchar();
		if(!isalpha(choice)){
			printf("\nCould not read a valid choice, please try again\n%s", MENUCHOICES);
			continue;
		}
		if(toupper(choice)=='L'){
			loadList();
		}else if(toupper(choice)=='E'){
			editList();
		} else if(toupper(choice)!='Q'){
			printf("Invalid Choice specified, please try again\n%s", MENUCHOICES);
		}
	}
	fclose(loaded_list);
	return 0;
}

void loadList(){
	printf("\nPlease enter the file path to a .tfo file: ");
	char filePath[MAX_SIZE];
	fgets(filePath, MAX_SIZE, stdin);
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
		while(toupper(choice)!='Y' &&  toupper(choice)!='N'){
			printf("\n%c", toupper(choice));
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

void editList()
{
	char edit_options [] = "\n(A)dd a file to list\n(R)emove a file from the list\n(I)nsert new tag to file\n(D)elete a tag from a file or list\n(N)ame list\n(Q)uit back to main menu\n\nEnter your choice: ";
	printf("\nLoading list contents...\n");
	printListContents();
	printf("\nList contents loaded successfully\n");
	printf("\nWhat would you like to do with the contents of the list?\n%s", edit_options);
	char choice='z';
	while(toupper(choice)!='Q'){
		choice=getchar();
		getchar();
		if(toupper(choice) == 'A'){
			addFile();
		} else if(toupper(choice) == 'R'){
			removeFile();
		}else if(toupper(choice) == 'I'){
			newTag();
		} else if(toupper(choice)== 'D'){
			deleteTag();
		} else if(toupper(choice) == 'N'){
			renameList();
		} else if(toupper(choice) != 'Q') {
			printf("Not a valid choice, please try again\n");
		}
		printf("\nWhat would you like to do with the contents of the list?\n%s", edit_options);
	}
	printf("\n%s", MENUCHOICES);
	return;
}

void addFile(){
	fclose(loaded_list);
	loaded_list = fopen(curr_list, "ab");
	printf("What's the filepath of the new file?: ");
	char filePath[MAX_SIZE];
	fgets(filePath, MAX_SIZE, stdin);
	filePath[strcspn(filePath, "\n")] = 0;
	if(isFileCreated(filePath)){
		printf("Adding file to current list...");
		fprintf(loaded_list, strcat(filePath, ", DEFAULTTAG(Will be removed when a tag is given to it)"));
	} else {
		printf("Error, could not open file located at this filepath.\n");
	}
	fclose(loaded_list);
	loaded_list = fopen(curr_list, "rb");
	return;
}

void printListContents()
{
	char line[MAX_SIZE];
	int counter = 1;
	while(fgets(line, sizeof(line), loaded_list)){
		printf("%i: %s", counter, line);
		counter++;
	}
	return;
}

void removeFile(){
	//TODO: Remove file functionality
	fclose(loaded_list);
	loaded_list=fopen(curr_list, "wb");
	taggedFile *head;
	head=malloc(sizeof(taggedFile));
	if(head == NULL){
		printf("ERROR REGARDING CREATING STRUCT\n");
		return;
	}
	char line[MAX_SIZE];
	int counter = 1;
	while(fgets(line, sizeof(line), loaded_list)){
		if(counter == 1){
			head->filePath = line;
			head->index = counter;
					
		} else {
			createListStructs(head, counter, line);
		}
	}
	int max_index = getMaxListIndex(head)
	printListContents();
	int ok_choice = 1;
	if(max_index = 1){
		printf("This list has no files...\n");
		return;
	}
	while(ok_choice == 1){
		printf("Choose index of file to be removed(Use Q to quit): ");
		char choice = getchar();
		if(toupper(choice)='Q'){
			printf("\nNo files being deleted, leaving...\n");
			ok_choice = 0;
		} else if(!isalpha(choice)){
			int index = (int)choice - 48;
			if(index < 0 | index > max_index){
				printf("\nERROR: INVALID CHOICE\n");
			} else if(index == 1){
				printf("Error, cannot remove the head of this file\n");
			} else {
				removeByIndex(head, index);
				ok_choice = 0;
			}
		}	
	}
	taggedFile *tmp;
	tmp = head;
	while(tmp->next_file != NULL){
		char new_line[MAX_SIZE+2028]="";
		strcat(new_line, tmp->filePath);
		if(tmp->index != 1){	
			strcat(new_line, ", ");
			for(int i=0, (sizeof(tmp->tags)/sizeof(tmp->tags[0]))-1, i++){
				if(i != ((sizeof(tmp->tags)/sizeof(tmp->tags[0]))-1)){
					strcat(new_line, tmp->tags[i]);
					strcat(new_line, ", ";
				} else {
					strcat(new_line, tmp->tags[i]);
				}
			}
			taggedFile *tmp2 = tmp;
			tmp = tmp->next_file;
			free(tmp2);
		} else {
			tmp = head->next_file;
			free(head);
		}
		fprintf(loaded_file, new_line);
	}
	char new_line[MAX_SIZE+2028]="";
	strcat(new_line, tmp->next_file);
	strcat(new_line, ", ");
	for(int i=0, (sizeof(tmp->tags)/sizeof(tmp->tags[0]))-1, i++){
		if(i != ((sizeof(tmp->tags)/sizeof(tmp->tags[0]))-1)){
			strcat(new_line, tmp->tags[i]);
			strcat(new_line, ", ");
		} else {
			strcat(new_line, tmp->tags[i]);
		}
	}
	free(tmp);
	fprintf(loaded_list, new_line);
	fclose(loaded_list);
	loaded_list = fopen(curr_list, "rb");
	return;
}

void createListStructs(taggedFile *head, int counter, char[] line){
	char[] line_cpy1 = line;
	taggedFile *new_file;
	taggedFile *tmp;
	new_file=malloc(sizeof(taggedFile));
	tmp=head;
	while(tmp->next_file != NULL){
		tmp = tmp->next_file;
	}
	tmp->next_file  = new_file;
	char* token = strtok(line_cpy1, ",");
	int token_ctr = 1;
	while(token!=NULL){
		if(token_ctr == 1){
			new_file->filePath == token;
		} else if(token_ctr <= 11) {
			new_file->tags[token_ctr - 2]=token;
		} else {
			printf("\nMAX TAGS REACHED, WILL BE TRUNCATED\n");
		}
	}
	return;

}

int getMaxListIndex(taggedFile *head){
	taggedFile *tmp;
	tmp=malloc(sizeof(taggedFile));
	tmp=head;
	while(tmp->next_file != NULL){
		tmp = tmp->next_file;
	}
	return tmp->index;
}

void removeByIndex(taggedFile *head, int index){
	taggedFile *tmp;
	tmp = head;
	while(tmp->next_file->index != index){
		tmp = tmp->next_file;
	}
	taggedFile *tmp2 = tmp->nextFile;
	tmp->nextFile = tmp2->nextFile;
	free(tmp2);
	return;

}

void newTag(){
	//TODO: New tag functionality
	return;
}

void deleteTag(){
	//TODO: Delete tag functionality
	return;
}

void renameList(){
	//TODO: Rename list functionality
	return;
}
