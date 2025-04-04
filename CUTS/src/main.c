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
void printListContents(int offset);
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
taggedFile *createHead();
void addTagReccursive(taggedFile *head, char new_tag[], char filePath[]);
void destroyStructs(taggedFile *head);
void removeTag(taggedFile *head, int index);
void removeTagList(taggedFile *head, char tag_to_remove[]);
void removeTagSubDirectories(taggedFile *head, char filePath[], char tag_to_remove[]);
int printFilteredListContents(int offset, char filter[][MAX_SIZE]);
void openList();
void checkList(taggedFile *head);

const char MENUCHOICES[]="\nMAIN MENU:\n(L)oad a list\n(E)dit current list\n(D)elete list\n(V)iew list\n(Q)uit CUTS\n";
char curr_list[MAX_SIZE]="";
char curr_txtList[MAX_SIZE]="";
FILE *loaded_list;
FILE *loaded_listt;
int curr_list_line_num;
const int MAX_ENTRIES = 250;
char main_static[MAX_SIZE];

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
		#ifdef __linux__
			mkdir(utsFilePath, 0700);
		#elif _WIN32
			mkdir(utsFilePath);
		#endif
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
		strcpy(main_static, utsFilePath);
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
			strcpy(main_static, utsFilePath);
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
	int num_lines = 0;
	while(!feof(loaded_list)){
		fread(&lines, sizeof(lines), 1, loaded_list);
		if(!feof(loaded_list)){
			fprintf(loaded_listt, lines);
			num_lines++;
		}
	}
	curr_list_line_num = num_lines;
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
		} else if(toupper(choice)=='D'){
			printf("%s\n%s\n", curr_list, main_static);
			if(strcmp(curr_list, main_static)){
				writeBinaryFile();
				if(remove(curr_list)!=0){
					printf("Major error\n");
				}
				strcpy(curr_list, main_static);
				writeTextFile();
				printf("Removed file, returning to main.tfo\n%s", MENUCHOICES);
			} else {
				printf("Cannot remove main file, returning... \n%s", MENUCHOICES);
			}
		} else if(toupper(choice)=='V'){
			openList();
			printf(MENUCHOICES);
		} else if(toupper(choice)!='Q'){
			printf("Invalid Choice specified, please try again\n%s", MENUCHOICES);
		} else {
			printf("\nClose program. See you!\n");
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
		writeBinaryFile();
		printf("Closing current list...\n");
		printf("Opening list located at %s...\n", filePath);
		strcpy(curr_list, filePath);
		writeTextFile();
		printf("Validating list...\n");
		taggedFile *head = createHead();
		checkList(head);
		destroyStructs(head);
		printf("\n%s", MENUCHOICES);
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

void checkList(taggedFile *head){
	taggedFile *tmp = head;
	while(tmp->next_file != NULL){
		if(tmp->index == 1){
			tmp = tmp->next_file;
		} else {
			if(isFileCreated(tmp->filePath)){
				tmp = tmp->next_file;
			} else {
				int valid_choice=1;
				while(valid_choice == 1){
					printf("\nThis file (%s) seems to be missing or the permissions have been changed. (C)hange the file path or (R)emove the element from the list?: ", tmp->filePath);
					char choice = getchar();
					flush_chars();
					if(toupper(choice) == 'C'){
						printf("\nEnter the new filepath: ");
						char newFilePath[MAX_SIZE];
						fgets(newFilePath, MAX_SIZE, stdin);
						newFilePath[strcspn(newFilePath, "\n")] = 0;
						if(isFileCreated(newFilePath)){
							printf("\nSuccessfully updated this entries filepath\n");
							strcpy(tmp->filePath, newFilePath);
							tmp = tmp->next_file;
							valid_choice = 0;
						} else {
							printf("\nError, could not open the new filepath, check permissions and if file exists\n");
						}
					} else if(toupper(choice) == 'R'){
						printf("\nRemoving entry from list...\n");
						int index = tmp->index;
						tmp = tmp->next_file;
						removeByIndex(head, index);
						valid_choice = 0;
					}
				}
			}
		}
	}
	if(tmp->index == 1){
		printf("Succesfully opened empty list...\n");
		return;
	} else {
		if(isFileCreated(tmp->filePath)){
			printf("\nSuccessfully validated list\n");
			return;
		} else {
			int valid_choice=1;
			while(valid_choice == 1){
				printf("\nThis file (%s) seems to be missing or the permissions have been changed. (C)hange the file path or (R)emove the element from the list?: ", tmp->filePath);
				char choice = getchar();
				flush_chars();
				if(toupper(choice) == 'C'){
					printf("\nEnter the new filepath: ");
					char newFilePath[MAX_SIZE];
					fgets(newFilePath, MAX_SIZE, stdin);
					newFilePath[strcspn(newFilePath, "\n")] = 0;
					if(isFileCreated(newFilePath)){
						printf("\nSuccessfully updated this entries filepath\n");
						strcpy(tmp->filePath, newFilePath);
						valid_choice = 0;
					} else {
						printf("\nError, could not open the new filepath, check permissions and if file exists\n");
					}
				} else if(toupper(choice) == 'R'){
					printf("\nRemoving entry from list...\n");
					removeByIndex(head, tmp->index);
					valid_choice = 0;
				}
			}
			
		}	
	}
	printf("\nSuccessfully validated list\n");
	return;
}

void editList()
{
	char edit_options [] = "\n(A)dd a file to list\n(R)emove a file from the list\n(I)nsert new tag to file\n(D)elete a tag from a file or list\n(N)ame list\n(Q)uit back to main menu\n\nEnter your choice: ";
	printf("\nLoading list contents...\n");
	printListContents(0);
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
	if(strlen(filePath) > MAX_SIZE){
		printf("Error, size of filepath is longer than %i characters and is thus discarded, returning to edit\n");
		return;
	}
	if(curr_list_line_num == MAX_ENTRIES){
		printf("Error, max size of elements reached, discarding file, returning to edit\n");
	}
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
	curr_list_line_num++;
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
	if(curr_list_line_num == MAX_ENTRIES){
		printf("Cannot add anymore entries to this list\n");
		closedir(dir);
		return;
	}

	while((entry = readdir(dir)) != NULL){
		if(strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..") == 0 || strstr(entry->d_name, ".git") != NULL){
			continue;
		}
		char path[MAX_SIZE];
		#ifdef __linux__
			snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
		#elif _WIN32
			snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
		#endif
		char pathToWrite[MAX_SIZE];
		printf("%s\n%s\n%s", directory, entry->d_name, pathToWrite);
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
			curr_list_line_num++;
		}
	}
	closedir(dir);
}

void printListContents(int offset)
{
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	char line[MAX_SIZE+2048];
	int counter = 1-offset;
	while(fgets(line, sizeof(line), loaded_list) &&  counter < 10){
		if(counter > 0){
			printf("%i: %s", counter, line);
		}
		counter++;
	}
	if(offset != 0){
		printf("<(P)rev\n");
	}
	if(counter+offset < curr_list_line_num){
		printf("(N)ext>\n");
	}
	return;
}

int printFilteredListContents(int offset, char filter[][MAX_SIZE]){
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	char line[MAX_SIZE+2048];
	int counter = 1-offset;
	int line_num = 0;
	int free_pos = 0;
	int end_of_filter = 0;
	char filtered_list[250][MAX_SIZE];
	while(fgets(line, sizeof(line), loaded_list)){
		int has_filters = 1;
		for(int i = 0; i < 9; i++){
			if(filter[i][0] == '\0'){
				break;
			}
			if(strstr(line, filter[i])== NULL){
				has_filters = 0;
				break;
			}
		}
		if(has_filters==1){
			strcpy(filtered_list[free_pos], line);
			free_pos++;
		}
		line_num++;
	}
	for(int i = 0; i < sizeof(filtered_list)/sizeof(filtered_list[0]); i++){
		if(counter > 0 && counter<10){
			if(filtered_list[i][0] == '\0' || i == MAX_SIZE){
				end_of_filter = 1;
				break;
			}
			printf("%i: %s", counter, filtered_list[i]);
			counter++;
		} else {
			counter++;
		}
	}	
	if(offset != 0){
		printf("<(P)rev\n");
	}
	if(end_of_filter != 1){
		printf("(N)ext>\n");
	}
	return end_of_filter;
}

void removeFile(){
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	taggedFile *head = createHead();
	int max_index = getMaxListIndex(head);
	int ok_choice = 1;
	int curr_page = 0;
	if(max_index == 1){
		printf("This list has no files...\n");
		return;
	}
	while(ok_choice == 1){
		printListContents(curr_page*9);
		printf("\nChoose index of file to be removed(Use Q to quit): ");
		char choice = getchar();
		flush_chars();
		if(toupper(choice)=='Q'){
			printf("\nNo files being deleted, leaving...\n");
			ok_choice = 0;
		} else if(toupper(choice)=='P'){
			if(curr_page == 0){
				printf("Error, on the first page\n");
				continue;
			}
			curr_page--;
		} else if(toupper(choice)=='N'){
			if(curr_list_line_num<=(curr_page+1)*9){
				printf("Error, on the last page\n");
				continue;
			}
			curr_page++;
		} else if(!isalpha(choice)){;
			int index = (int)choice;
			index = index-48;
			if(index < 0 | index > max_index){
				printf("\nERROR: INVALID CHOICE\n");
			} else if(index == 1){
				printf("Error, cannot remove the head of this file\n");
			} else {
				removeByIndex(head, index+(curr_page*9));
				ok_choice = 0;
			}
		} else {
			printf("\nERROR: Not an index, please choose again\n");
		}	
	}
	destroyStructs(head);
	return;
}

taggedFile *createHead(){
	static taggedFile *head;
	head=malloc(sizeof(taggedFile));
	if(head == NULL){
		printf("ERROR REGARDING CREATING STRUCT\n");
		return head;
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
	return head;
}

void createListStructs(taggedFile *head, int counter, char line[]){
	if(curr_list_line_num == 1){
		head->next_file=NULL;
		return;
	}
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

void destroyStructs(taggedFile *head){
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
			free(head);
		}
		fprintf(loaded_list, new_line);
	}
	char new_line[MAX_SIZE+2028]="";
		if(curr_list_line_num != 1){
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
			strcat(new_line, "\n");
	} else {
		strcat(new_line, tmp->filePath);
	}
	if(tmp != NULL){
		free(tmp);
	}
	fprintf(loaded_list, new_line);
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
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
		prev->next_file = tmp->next_file;
		free(tmp);
		curr_list_line_num--;
	}
	return;

}

void newTag(){
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	taggedFile *head=createHead();
	int max_index = getMaxListIndex(head);
	int ok_choice = 1;
	int curr_page = 0;
	if(max_index == 1){
		printf("This list has no files...\n");
		return;
	}
	while(ok_choice == 1){
		printListContents(curr_page*9);
		printf("\nChoose the index of the file you wish to add a new tag to: ");
		char choice = getchar();
		flush_chars();
		if(toupper(choice) == 'Q'){
			printf("Appending no files... \n");
			ok_choice=0;
		} else if(toupper(choice)=='P'){
			if(curr_page == 0){
				printf("Error, on the first page\n");
				continue;
			}
			curr_page--;
		} else if(toupper(choice)=='N'){
			if(curr_list_line_num<=(curr_page+1)*9){
				printf("Error, on the last page\n");
				continue;
			}
			curr_page++;
		} else if(!isalpha(choice)){
			int index = (int)choice;
			index = index-48;
			if(index <= 0 | index > max_index){
				printf("\nERROR INVALID CHOICE, PLEASE CHOOSE A DIFFERENT ONE\n");
			} else if(index == 1){
				printf("\nError: Head of file does not have any tags\n");
			} else {
				addTag(head, index+(curr_page*9));
				ok_choice = 0;
			}
		} else {
			printf("\nNot an index, please enter a valid index or type q to quit");
		}
	}
	destroyStructs(head);	
	return;
}

void addTag(taggedFile *head, int index){
	taggedFile *tmp;
	tmp = malloc(sizeof(taggedFile));
	tmp = head;
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
	if(tmp->num_entries >= 9){
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
			printf("New tag for this file will be %s\n", new_tag);
			tmp->num_entries++;
			strcpy(tmp->tags[tmp->num_entries-1],new_tag);
		}
		if(isFolderCreated(tmp->filePath)){
			char choice;
			int ok_dir=1;
			while(ok_dir==1){	
				printf("\nDo you want to add this new tag to all files and sub-directories in this directory?: ");
				choice = getchar();
				flush_chars();
				if(toupper(choice)!= 'Y' && toupper(choice)!='N'){
					printf("Please type Y or N\n");
					continue;
				} else if (toupper(choice)=='Y'){
					addTagReccursive(head, new_tag, tmp->filePath);
					ok_dir=0;
					printf("Applied tags to all sub-files and directories\n");
				} else {
					printf("Applying change to directory only...");
					ok_dir=0;
				}
			}
		}
	}
	return;
}

void addTagReccursive(taggedFile *head, char new_tag[], char filePath[]){
	taggedFile *tmp = head;
	while(tmp->next_file != NULL){
		if(strstr(tmp->filePath, filePath)!=NULL && strcmp(tmp->filePath, filePath)!=0 && strcmp(tmp->tags[tmp->num_entries-1], new_tag)!=0){
			if(tmp->num_entries < 9){
				tmp->num_entries++;
				strcpy(tmp->tags[tmp->num_entries-1], new_tag);
				printf("Added tag %s to %s\n", new_tag, tmp->filePath);
			} else {
				printf("Cannot add new tag to entry %s, maximum tags reached\n", tmp->filePath);
			}
			if(isFolderCreated(tmp->filePath)){
				addTagReccursive(head, new_tag, tmp->filePath);
			}
		}	
		tmp = tmp->next_file;
	}
	if(strstr(tmp->filePath, filePath)!=NULL && strcmp(tmp->filePath, filePath)!=0 && strcmp(tmp->tags[tmp->num_entries-1], new_tag)!=0){
		if(tmp->num_entries < 9){
			tmp->num_entries++;
			strcpy(tmp->tags[tmp->num_entries-1], new_tag);
		} else {
			printf("Cannot add new tag to entry %s, maximum tags reached\n", tmp->filePath);
		}
		if(isFolderCreated(tmp->filePath)){
			addTagReccursive(head, new_tag, tmp->filePath);
		}
	}
	return;
}

void deleteTag(){
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	taggedFile *head = createHead();
	int max_index = getMaxListIndex(head);
	int filter_size = 1;
	int ok_choice = 1;
	int curr_page = 0;
	if(max_index == 1){
		printf("This list has no files...\n");
		return;
	}
	while(ok_choice == 1){
		printListContents(curr_page*9);
		printf("\nChoose the index of the file you wish to remove a tag from: ");
		char choice = getchar();
		flush_chars();
		if(toupper(choice) == 'Q'){
			printf("Removing no tags... \n");
			ok_choice=0;
		} else if(toupper(choice)=='P'){
			if(curr_page == 0){
				printf("Error, on the first page\n");
				continue;
			}
			curr_page--;
		} else if(toupper(choice)=='N'){
			if(curr_list_line_num<=(curr_page+1)*9){
				printf("Error, on the last page\n");
				continue;
			}
			curr_page++;
		} else if(!isalpha(choice)){
			int index = (int)choice;
			index = index-48;
			if(index <= 0 | index > max_index){
				printf("\nERROR INVALID CHOICE, PLEASE CHOOSE A DIFFERENT ONE\n");
			} else if(index == 1){
				printf("\nError: Head of file does not have any tags\n");
			} else {
				removeTag(head, index+(curr_page*9));
				ok_choice = 0;
			}
		} else {
			printf("\nNot an index, please enter a valid index or type q to quit");
		}
	}
	destroyStructs(head);	
	return;
}

void removeTag(taggedFile *head, int index){
	taggedFile *tmp;
	tmp = malloc(sizeof(taggedFile));
	tmp = head;
	for(int i = 1; i < index; i++){
		tmp = tmp->next_file;
	}
	if(tmp!= NULL){
		printf("Tags for %s\n", tmp->filePath);
		for(int i = 0; i < 10; i++){
			if(i+1 > tmp->num_entries || tmp->tags[i] == NULL || tmp->tags[i]==""){
				break;	
			} else {
				printf("%i: %s\n", i+1, tmp->tags[i]);
			}
		}
			
	}
	int ok_choice = 1;
	while(ok_choice == 1){
		printf("\nChoose the index of the tag you want removed (Type Q to quit): ");
		char choice = getchar();
		flush_chars();
		if(toupper(choice) == 'Q'){
			printf("Leaving without removing any tags...\n");
			ok_choice = 0;
		} else if(!isalpha(choice)){
			int tag_index = (int)choice;
			tag_index = tag_index - 48;
			if(tag_index <= 1){
				printf("Error: Cannot remove identifier tag\n");
			} else if(tag_index > tmp->num_entries){
				printf("Error: Not a valid index\n");
			} else {
				ok_choice = 0;
				char tag_to_remove[2048];
				strcpy(tag_to_remove, tmp->tags[tag_index-1]);
				for(int i = 0; i < tmp->num_entries; i++){
					if(i < tag_index-1){
						continue;
					} else if(i == tag_index-1){
						tmp->num_entries--;
						if(i!=8){
							strcpy(tmp->tags[i], tmp->tags[i+1]);
						}
					} else {
						if(i != 8){
							strcpy(tmp->tags[i], tmp->tags[i+1]);	
						}
					}
				}
				printf("Successfully removed %s from %s\n", tag_to_remove, tmp->filePath);
				int ok_choice2 = 1;
				while(ok_choice2 == 1){
					printf("Do you wish to remove this tag from (S)ubdirectories, the (L)ist or (N)ot?");
					char choice2 = getchar();
					flush_chars();
					if(toupper(choice2) == 'S'){
						removeTagSubDirectories(head, tmp->filePath, tag_to_remove);
						ok_choice2 = 0;
					} else if(toupper(choice2) == 'L'){
						removeTagList(head, tag_to_remove);
						ok_choice2 = 0;
					} else if(toupper(choice2) == 'N'){
						printf("Returning to edit menu...\n");
						ok_choice2 = 0;
					} else {
						printf("Please provide a valid input of S, L or N\n");
					}
				}
			}
		} else {
			printf("NOT A VALID INDEX, PLEASE PROVIDE AN INDEX OR TYPE Q TO QUIT");
		}
	}
	return;
}

void removeTagSubDirectories(taggedFile *head, char filePath[], char tag_to_remove[]){
	taggedFile *tmp = malloc(sizeof(taggedFile));
	tmp = head;
	while(tmp->next_file != NULL){
		if(strstr(tmp->filePath, filePath)!=NULL && strcmp(tmp->filePath, filePath)!=0){
			int hasTag = 0;
			for(int i = 0; i < tmp->num_entries; i++){
				if(hasTag == 0){
					if(strcmp(tmp->tags[i], tag_to_remove)==0){
						tmp->num_entries--;
						hasTag = 1;
						if(i != 8){
							strcpy(tmp->tags[i], tmp->tags[i+1]);
						}
					} else {
						continue;
					}
				} else {
					if(i != 8){
						strcpy(tmp->tags[i], tmp->tags[i+1]);
					}
				}
			}
			
			if(isFolderCreated(tmp->filePath)){
				removeTagSubDirectories(head, tmp->filePath, tag_to_remove);
			}
		}
		tmp = tmp->next_file;
	}
	if(strstr(tmp->filePath, filePath)!=NULL && strcmp(tmp->filePath, filePath)!=0){
		int hasTag = 0;
		for(int i = 0; i < tmp->num_entries; i++){
			if(hasTag == 0){
				if(strcmp(tmp->tags[i], tag_to_remove)==0){
					tmp->num_entries--;
					hasTag = 1;
					if(i != 8){
						strcpy(tmp->tags[i], tmp->tags[i+1]);
					}
				} else {
					continue;
				}
			} else {
				if(i != 8){
					strcpy(tmp->tags[i], tmp->tags[i+1]);
				}
			}
		}
	}
	if(isFolderCreated(tmp->filePath)){
		removeTagSubDirectories(head, tmp->filePath, tag_to_remove);
	}
	return;
}

void removeTagList(taggedFile *head, char tag_to_remove[]){
	taggedFile *tmp= malloc(sizeof(taggedFile));
	tmp = head;
	while(tmp->next_file != NULL){
		if(tmp->index == 1){
			tmp=tmp->next_file;
			continue;
		}
		int hasTag = 0;
		for(int i = 0; i<tmp->num_entries; i++){
			if (hasTag == 0){
				if(strcmp(tmp->tags[i], tag_to_remove)==0){
					tmp->num_entries--;
					hasTag = 1;
					if(i != 8){
						strcpy(tmp->tags[i], tmp->tags[i+1]);
					}
				} else {
					continue;
				}
			} else {
				if(i != 8){
					strcpy(tmp->tags[i], tmp->tags[i+1]);
				}
			}
		}
		tmp=tmp->next_file;
	}
	int hasTag = 0;
	for(int i = 0; i<tmp->num_entries; i++){
		if (hasTag == 0){
			if(strcmp(tmp->tags[i], tag_to_remove)==0){
				tmp->num_entries--;
				hasTag = 1;
				if(i != 8){
					strcpy(tmp->tags[i], tmp->tags[i+1]);
				}
			} else {
				continue;
			}
		} else {
			if(i != 8){
				strcpy(tmp->tags[i], tmp->tags[i+1]);
			}
		}
	}
	return;
}

void renameList(){
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	taggedFile *head = createHead();
	printf("Current list name: %s\n ", head->filePath);
	int ok_choice = 1;
	while(ok_choice == 1){
		printf("What would you like to change the current list's name to? (NOTE: You can type QUIT to exit): ");
		char new_name[256];
		fgets(new_name, 256, stdin);
		if(strcmp(new_name, "QUIT\n")!= 0){
			printf("Saving the new file name to %s\n", new_name);
			strcpy(head->filePath, new_name);
			ok_choice = 0;
		} else {
			printf("Keeping the name the same...\n");
			ok_choice = 0;
		}
	}
	destroyStructs(head);
	return;
}

void flush_chars(){
	int c;
	while((c = getchar()) != '\n' && c != EOF){

	}
	return;
}

void openList(){
	char open_list_options[] = "Opened for review.\n(F)ilter the list based on a tag\n(R)emove filters\n(Q)uit";
	fclose(loaded_list);
	loaded_list = fopen(curr_txtList, "r");
	taggedFile *head = createHead();
	int hasFilters = 0;
	char filterList[9][MAX_SIZE];
	int ok_choice = 1;
	int curr_page = 0;
	int end_of_filter = 0;
	int filter_size = 1;
	while(ok_choice == 1){
		printf("\n");
		if(hasFilters == 0){
			printListContents(curr_page*9);
			end_of_filter = 0;
		} else {
			end_of_filter=printFilteredListContents(curr_page*9, filterList);
		}
		printf("%s\nMake your choice: ", open_list_options);
		char choice = getchar();
		flush_chars();
		if(toupper(choice) == 'F'){
			printf("\nPlease type in the tag you wish to filter by: ");
			char new_filter[256];
			fgets(new_filter, 256, stdin);
			new_filter[strcspn(new_filter, "\n")] = 0;
			for(int i = 0; i < 10; i++){
				if(i-1 < filter_size){
					strcpy(filterList[i], new_filter);
					printf(filterList[i]);
					filter_size++;
					break;
				}
			}
			printf("Added %s to filter\n", new_filter);
			hasFilters = 1;
			curr_page = 0;
		}else if(toupper(choice) == 'R'){
			for(int i = 0; i < 10; i++){
				filterList[i][0]='\0';
			}
			hasFilters = 0;
			curr_page = 0;
			printf("\nRemoved all filters\n");
			strcpy(open_list_options, "Opened for review\n(F)ilter the list based on a tag\n(R)emove filters\n(Q)uit");//TODO: Find out what overflows the buffer to force me to do this in the first place.	
		}else if(toupper(choice) == 'Q'){
			printf("Exiting list...\n");
			ok_choice = 0;
		} else if(toupper(choice)=='P'){
			if(curr_page == 0){
				printf("Error, on the first page\n");
				continue;
			}
			curr_page--;
		} else if(toupper(choice)=='N'){
			if(curr_list_line_num<=(curr_page+1)*9 || end_of_filter != 0){
				printf("Error, on the last page\n");
				continue;
			}
			curr_page++;
		}
	}
	destroyStructs(head);
	return;
}
