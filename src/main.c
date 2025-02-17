#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include "headers/fileCheck.h"
#include "headers/folderCheck.h"
#include "headers/curUser.h"

#ifdef __linux__
#define MAX_SIZE 4096
#elif _WIN32
#define MAX_SIZE  260
#else 
#define MAX_SIZE 256
#endif

typedef struct taggedFile{
	int index;
	char filePath[MAX_SIZE];
	char tags[9][2028];
	struct taggedFile *next_file;
	int num_entries;
}taggedFile;

int menu();
void loadList();
void editList();
void addFile();
void printListContents();
void removeFile();
void newTag();
void deleteTag();
void renameList();
void createListStructs(taggedFile *head, int counter, char line[]);
int getMaxListIndex(taggedFile *head);
void removeByIndex(taggedFile *head, int index);
void writeTextFile();
void writeBinaryFile();
void addTag();
void flush_chars();
void addFileRecursive(char *directory);

const char MENUCHOICES[]="\nMAIN MENU:\n(L)oad a list\n(E)dit current list\n(D)elete list\n(O)pen list\n(Q)uit CUTS\n";
char curr_list[MAX_SIZE]="";
char curr_txtList[MAX_SIZE]="";
FILE *loaded_list;
FILE *loaded_listt;

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
		printf("Current list: %s\n", curr_list);
		writeTextFile();
	} else {
		printf("Have not found the file, creating file now...\n");
		FILE *main_list = fopen(utsFilePath, "wb");
		char header[MAX_SIZE+2048] = "Main\n";
		fwrite(header, sizeof(header), 1, main_list);
		fclose(main_list);
		printf("Checking if file has been created...\n");
		if(isFileCreated(utsFilePath)){
			strcpy(curr_list, utsFilePath);
			printf("Current list: %s\n", curr_list);
			writeTextFile();
			printf("Succesfully created main_list.tfo!\nDo note not to delete this or it will be created again\n");
		} else {
			printf("Nope...\n");
			return 1;
		}
	}
	menu();
	return 0;
}

void writeTextFile(){
	if(loaded_list != NULL){
		fclose(loaded_list);
	}
	printf("Current text list: %s\n", curr_txtList);
	if(remove(curr_txtList)!=0){
		printf("Error removing a file...\n");
	}
	char tmp[MAX_SIZE];
	strcpy(tmp, curr_list);
	strcpy(curr_txtList, strcat(tmp, "t"));
	loaded_list = fopen(curr_list, "rb");
	loaded_listt = fopen(curr_txtList, "w");
	char lines[MAX_SIZE+2048];
	char line[MAX_SIZE+2048];
	if(loaded_list == NULL){
		printf("IT'S RIGHT THERE!\n");
	}
	while(!feof(loaded_list)){
		fread(&lines, sizeof(lines), 1, loaded_list);
		printf("This should have a line: %s\n", lines);
		if(!feof(loaded_list)){
			fprintf(loaded_listt, lines);
		}
	}
	fclose(loaded_listt);
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	return;
}

void writeBinaryFile(){
	fclose(loaded_list);
	char tmp[MAX_SIZE];
	printf("Closing and saving: %s\n", curr_list);
	strcpy(tmp, curr_list);
	strcat(tmp, "t");
	strcpy(curr_txtList, tmp);
	loaded_listt = fopen(curr_txtList, "r");
	loaded_list = fopen(curr_list, "wb");
	char line[MAX_SIZE+2048];	
	while(fgets(line, sizeof(line), loaded_listt)){
		printf("%s\n", line);
		fwrite(line, sizeof(line), 1, loaded_list);
	}
	fclose(loaded_listt);
	if(remove(curr_txtList) != 0){
		printf("DISASTER x2");
		return;
	}
	fclose(loaded_list);
	return;
}

int menu(){
	printf("(Note: Currently loaded list is main_list.tfo.) %s", MENUCHOICES);
	int choice=0;
	while(choice != 'Q'){
		printf("Enter your choice: ");
		choice=getchar();
		flush_chars();
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
	writeBinaryFile();
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
		printf("Opening list located at %s...\n%s", filePath, MENUCHOICES);
		strcpy(curr_list, filePath);
		writeTextFile();
		return;
	} else {
		printf("\nError: %s is not a valid .tfo file. Would you like to make it?\n(Y/N): ", filePath);
		char choice = getchar();
		while(toupper(choice)!='Y' &&  toupper(choice)!='N'){
			flush_chars();
			printf("Error: Not a valid choice, please choose type Y or N.\n");
			printf("Make a new file at %s?: ", filePath);
			choice = getchar();
		}
		if(toupper(choice)=='Y'){
			FILE* temp_list = fopen(filePath, "wb");
			char line[MAX_SIZE+2048] = "NEW TAGGING FILE\n";
			fwrite(&line, sizeof(line), 1, temp_list);
			fclose(temp_list);
			if(isFileCreated(filePath)){
				printf("Succesfully created new list at %s\nLoading list...\n", filePath);
				strcpy(curr_list, filePath);
			} else {
				printf("Could not create list...\n");
			}
			flush_chars();
			writeTextFile();
		} else {
			printf("List not created, defaulting back to %s\n", curr_list);
			flush_chars();
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
		flush_chars();
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
	loaded_list = fopen(curr_txtList, "a");
	printf("What's the filepath of the new file or directory?: ");
	char filePath[MAX_SIZE];
	fgets(filePath, MAX_SIZE, stdin);
	filePath[strcspn(filePath, "\n")] = 0;
	if(isFileCreated(filePath)){
		if(isFolderCreated(filePath)){
			printf("Adding directory to current list...");
			char copyPath[MAX_SIZE];
			strcpy(copyPath, filePath);
			fprintf(loaded_list, strcat(copyPath,",dir\n"));
			printf("\nDo you want to recusively add all files and subdirectories?: ");
			char choice=getchar();
			flush_chars();
			while(toupper(choice) != 'Y' && toupper(choice) != 'N'){
				printf("\nERROR: Please, type Y or N");
				printf("\nDo you want to recusively add all files and subdirectories?: ");
				choice=getchar();
				flush_chars();
			}
			if(toupper(choice) == 'Y'){
				addFileRecursive(filePath);
			} else {
				printf("Returning to edit menu...\n");
			}
		} else {
			printf("Adding file to current list...");
			fprintf(loaded_list, strcat(filePath, ",file\n"));
		}
	} else {
		printf("\n ERROR: NO SUCH FILE OR DIRECTORY, RETURNING TO EDIT...\n");
	}
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	return;
}

void addFileRecursive(char *directory){
	struct dirent *entry;
	printf("%s\n", directory);
	DIR *dir = opendir(directory);
	if(dir == NULL){
		printf("\nError opening directory, unable to reccursively add files\n");
		return;
	}

	while((entry = readdir(dir)) != NULL){
		if(strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..") == 0 || strstr(entry->d_name, ".git") != NULL){
			continue;
		}
		char path[MAX_SIZE];
		snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
		char pathToWrite[MAX_SIZE];
		strcpy(pathToWrite, path);
		struct stat statbuf;
		if(stat(path, &statbuf) == 0){
			if(S_ISDIR(statbuf.st_mode)){
				printf("\nAdding directory: %s\n", path);
				fprintf(loaded_list, strcat(pathToWrite, ",dir\n"));
				addFileRecursive(path);
			} else {
				printf("\nAdding directory: %s\n", path);
				fprintf(loaded_list, strcat(pathToWrite, ",file\n"));
			}
		}
	}
	closedir(dir);
}

void printListContents()
{
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	char line[MAX_SIZE+2048];
	int counter = 1;
	while(fgets(line, sizeof(line), loaded_list)){
		printf("%i: %s", counter, line);
		counter++;
	}
	return;
}

void removeFile(){
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	taggedFile *head;
	head=malloc(sizeof(taggedFile));
	if(head == NULL){
		printf("ERROR REGARDING CREATING STRUCT\n");
		return;
	}
	head->next_file = NULL;
	char line[MAX_SIZE];
	int counter = 1;
	while(fgets(line, sizeof(line), loaded_list)){
		if(counter == 1){
			strcpy(head->filePath, line);
			head->index = counter;
					
		} else {
			if(line==""){
				continue;
			}
			createListStructs(head, counter, line);
		}
		counter++;
	}
	int max_index = getMaxListIndex(head);
	printListContents();
	int ok_choice = 1;
	if(max_index == 1){
		printf("This list has no files...\n");
		return;
	}
	while(ok_choice == 1){
		printf("Choose index of file to be removed(Use Q to quit): ");
		char choice = getchar();
		flush_chars();
		if(toupper(choice)=='Q'){
			printf("\nNo files being deleted, leaving...\n");
			ok_choice = 0;
		} else if(!isalpha(choice)){;
			int index = (int)choice;
			index = index-48;
			if(index < 0 | index > max_index){
				printf("\nERROR: INVALID CHOICE\n");
			} else if(index == 1){
				printf("Error, cannot remove the head of this file\n");
			} else {
				removeByIndex(head, index);
				ok_choice = 0;
			}
		} else {
			printf("\nERROR: Not an index, please choose again\n");
		}	
	}
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "w");
	printf("OPEN FOR WRITING!\n");
	taggedFile *tmp;
	tmp = head;
	while(tmp->next_file != NULL){
		char new_line[MAX_SIZE+2028]="";
		strcat(new_line, tmp->filePath);
		if(tmp->index != 1){	
			strcat(new_line, ",");
			for(int i=0; i<tmp->num_entries; i++){
				if(tmp->tags[i] == NULL || tmp->tags[i]=="" || i+1>tmp->num_entries){
					break;
				}
				if(i+1 != tmp->num_entries){
					strcat(new_line, tmp->tags[i]);
					strcat(new_line, ",");
				} else {
					strcat(new_line, tmp->tags[i]);
				}
			}
			strcat(new_line, "\n");
			taggedFile *tmp2 = tmp;
			tmp = tmp->next_file;
			free(tmp2);
		} else {
			tmp = head->next_file;
		}
		fprintf(loaded_list, new_line);
	}
	free(head);
	char new_line[MAX_SIZE+2028]="";
	strcat(new_line, tmp->filePath);
	strcat(new_line, ",");
	for(int i=0; i<tmp->num_entries; i++){
		if(tmp->tags[i]==NULL || tmp->tags[i] == "" || i+1>tmp->num_entries){
			strcat(new_line, "\n");
			break;
		}
		if(i+1 != tmp->num_entries){
			strcat(new_line, tmp->tags[i]);
			strcat(new_line, ",");
		}else {
			strcat(new_line, tmp->tags[i]);
		}
	}
	if(tmp != NULL){
		free(tmp);
	}
	strcat(new_line, "\n");
	fprintf(loaded_list, new_line);
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	return;
}

void createListStructs(taggedFile *head, int counter, char line[]){
	taggedFile *new_file;
	taggedFile *tmp;
	tmp=malloc(sizeof(taggedFile));
	new_file=malloc(sizeof(taggedFile));
	tmp=head;
	while(tmp->next_file != NULL){
		tmp = tmp->next_file;
	}
	tmp->next_file = new_file;
	new_file->next_file = NULL;
	new_file->num_entries = 0;
	char* token = strtok(line, ",");
	int token_ctr = 1;
	printf("\n%s\n", token);
	new_file->index = counter;
	while(token!=NULL){
		if(token_ctr == 1){
			strcpy(new_file->filePath, token);
		} else if(token_ctr <= 11) {
			token[strcspn(token, "\n")] = 0;
			strcpy(new_file->tags[token_ctr-2], token);
			new_file->num_entries++;
		} else {
			printf("\nMAX TAGS REACHED\n");
		}
		token=strtok(NULL, ",");
		token_ctr++;
	}
	return;

}

int getMaxListIndex(taggedFile *head){
	printf("\n%i\n", head->index);
	taggedFile *tmp;
	tmp=malloc(sizeof(taggedFile));
	tmp=head;
	while(tmp->next_file != NULL){
		tmp = tmp->next_file;
	}
	return tmp->index;
}

void removeByIndex(taggedFile *head, int index){
	if(index < 0){
		return;
	}
	taggedFile *tmp;
	tmp = malloc(sizeof(taggedFile));
	tmp = head;
	taggedFile *prev = malloc(sizeof(taggedFile));
	for(int i = 1; tmp != NULL && i < index; i++){
		prev = tmp;
		tmp = tmp->next_file;
	}
	if(tmp != NULL){
		printf("Index to be deleted: %i\n", tmp->index);
		printf("Previous index: %i\n", prev->index);
		prev->next_file = tmp->next_file;
		free(tmp);
	}
	return;

}

void newTag(){
	//TODO: New tag functionality
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	taggedFile *head;
	head=malloc(sizeof(taggedFile));
	if(head == NULL){
		printf("ERROR REGARDING CREATING STRUCT\n");
		return;
	}
	head->next_file = NULL;
	char line[MAX_SIZE];
	int counter = 1;
	while(fgets(line, sizeof(line), loaded_list)){
		if(counter == 1){
			strcpy(head->filePath, line);
			head->index = counter;
					
		} else {
			if(line==""){
				continue;
			}
			createListStructs(head, counter, line);
		}
		counter++;
	}
	int max_index = getMaxListIndex(head);
	printListContents();
	int ok_choice = 1;
	if(max_index == 1){
		printf("This list has no files...\n");
		return;
	}
	while(ok_choice == 1){
		printf("\nChoose the index of the file you wish to add a new tag to: ");
		char choice = getchar();
		flush_chars();
		if(toupper(choice) == 'Q'){
			printf("Appending no files... \n");
			ok_choice=0;
		} else if(!isalpha(choice)){
			int index = (int)choice;
			index = index-48;
			if(index <= 0 || index > max_index){
				printf("\nERROR INVALID CHOICE, PLEASE CHOOSE A DIFFERENT ONE\n");
			} else if(index == 1){
				printf("\nError: Head of file does not have any tags\n");
			} else {
				addTag(head, index);
				ok_choice = 0;
			}
		} else {
			printf("\nNot an index, please enter a valid index or type q to quit");
		}
	}	
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "w");
	printf("OPEN FOR WRITING!\n");
	taggedFile *tmp;
	tmp = head;
	while(tmp->next_file != NULL){
		char new_line[MAX_SIZE+2028]="";
		strcat(new_line, tmp->filePath);
		if(tmp->index != 1){	
			strcat(new_line, ",");
			for(int i=0; i<tmp->num_entries; i++){
				if(tmp->tags[i] == NULL || tmp->tags[i]=="" || i+1>tmp->num_entries){
					strcat(new_line, "\n");
					break;
				}
				if(i+1 != tmp->num_entries){
					strcat(new_line, tmp->tags[i]);
					strcat(new_line, ",");
				} else {
					strcat(new_line, tmp->tags[i]);
				}
			}
			strcat(new_line, "\n");
			taggedFile *tmp2 = tmp;
			tmp = tmp->next_file;
			free(tmp2);
		} else {
			tmp = head->next_file;
		}
		fprintf(loaded_list, new_line);
	}
	free(head);
	char new_line[MAX_SIZE+2028]="";
	strcat(new_line, tmp->filePath);
	strcat(new_line, ",");
	for(int i=0; i<tmp->num_entries; i++){
		if(tmp->tags[i]==NULL || tmp->tags[i] == "" || i+1>tmp->num_entries){
			break;
		}
		if(i+1 != tmp->num_entries){
			strcat(new_line, tmp->tags[i]);
			strcat(new_line, ",");
		}else {
			strcat(new_line, tmp->tags[i]);
		}
	}
	if(tmp != NULL){
		free(tmp);
	}
	strcat(new_line, "\n");
	fprintf(loaded_list, new_line);
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	return;
}

void addTag(taggedFile *head, int index){
	taggedFile *tmp;
	tmp = malloc(sizeof(taggedFile));
	tmp = head;
	getchar();
	for(int i = 1; tmp!=NULL &&  i < index; i++){
		tmp = tmp->next_file;
	}
	if(tmp != NULL){
		printf("\nTags for %s\n", tmp->filePath);
		for(int i = 0; i < 10; i++){
			if(i+1 > tmp->num_entries || tmp->tags[i] == NULL || tmp->tags[i]==""){
				break;	
			} else {
				printf("%i: %s\n", i+1, tmp->tags[i]);
			}
		}
	}
	if(tmp->num_entries > 9){
		printf("\nERROR: Cannot add a new tag, maximum tag limit reached, please remove a tag to add a new one\n");
		return;
	}
	int ok_tag = 1;
	while(ok_tag == 1){
		printf("Add your new tag: ");
		char new_tag[256];
		fgets(new_tag, 256, stdin);
		new_tag[strcspn(new_tag, "\n")] = 0;
		if(new_tag == NULL){
			printf("\nError, no tag entered, please enter in a proper tag\n");
		} else {
			ok_tag = 0;
			printf("%s\n", tmp->tags[0]);
			printf("New tag for this file will be %s\n", new_tag);
			if(strcmp(tmp->tags[0],"DEFAULTTAG(Will be removed when a tag is given to it)")==0){
				strcpy(tmp->tags[0],new_tag);
			} else {
				tmp->num_entries++;
				strcpy(tmp->tags[tmp->num_entries-1],new_tag);
			}
		}
	}
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

void flush_chars(){
	int c;
	while((c = getchar()) != '\n' && c != EOF){

	}
	return;
}
