#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iup.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include "headers/curUser.h"
#include "headers/folderCheck.h"
#include "headers/fileCheck.h"

#ifdef __linux__
#define MAX_SIZE 4096
#elif _WIN32
#define MAX_SIZE 260
#else
#define MAX_SIZE 256
#endif
#define MAX_ENTRIES 250

char curr_list[MAX_SIZE]="";
char curr_txtList[MAX_SIZE]="";
FILE *loaded_list;
FILE *loaded_listt;
int curr_list_line_num;
char *listContents;
char list_name[MAX_SIZE];
Ihandle *dlg, *button, *button_dir, *button_list_name, *button_remove_entries, *vbox, *heading, *list_head, *list_content, *item_open, *file_menu, *sub1_menu, *menu, *item_save, *button_add_tag;

typedef struct{
	Ihandle *toggles[MAX_ENTRIES];
	int count;
	Ihandle *list_container;
	Ihandle *vbox;
}ToggleManager;

ToggleManager tm = {0};

void returnListEntries(){
	if(loaded_list != NULL){
		fclose(loaded_list);	
	}
	if(tm.toggles){
		for(int i=0; i< tm.count; i++){
			if(tm.toggles[i] == NULL){
				continue;
			}
			printf("Does it crash at this moment or after trying to access a toggle?\n");
			printf("Removing %s toggle\n", IupGetAttribute(tm.toggles[i], "TITLE"));
			IupDestroy(tm.toggles[i]);
			printf("It's... good?\n");
			tm.toggles[i] = NULL;
		}
	}
	tm.count = curr_list_line_num;
	loaded_list = fopen(curr_txtList, "r");
	char line[MAX_SIZE+2048];
	int counter = 0;
	while(fgets(line, sizeof(line), loaded_list)){
		line[strcspn(line, "\n")] = 0;
		if(counter == 0){
			printf("%s\n", line);
			strcpy(list_name, line);
			IupSetAttribute(list_head, "TITLE", line);
		} else {
			tm.toggles[counter-1] = IupToggle(line, NULL);
			IupAppend(tm.vbox, tm.toggles[counter-1]);
			printf("%s\n", IupGetAttribute(tm.toggles[counter-1], "TITLE"));
			IupMap(tm.toggles[counter-1]);
		}
		counter++;
	}
	IupMap(tm.vbox);
	IupRefresh(tm.vbox);
	IupMap(tm.list_container);
	IupRefresh(tm.list_container);
	IupMap(dlg);
	IupRefresh(dlg);
	return;
}

void returnFilteredListEntries(){
	//TODO: Code to make a filtered list to explore through
	return;
}

void addFileToList(char *new_entry){
	if(loaded_list != NULL){
		fclose(loaded_list);
	}
	loaded_list = fopen(curr_txtList, "a");
	strcat(new_entry, ",file\n");
	if(curr_list_line_num < MAX_ENTRIES){
		fprintf(loaded_list, new_entry);
		printf("Added file\n");
	}
	return;
}

void addReccursiveFilesToList(char *directory){
	if(loaded_list != NULL){
		fclose(loaded_list);
	}
	printf("Adding files from this directory: %s\n", directory);
	struct dirent *entry;
	loaded_list = fopen(curr_txtList, "a");
	DIR *dir = opendir(directory);
	if(dir == NULL){
		return;
	}
	if(curr_list_line_num == MAX_ENTRIES){
		closedir(dir);
		return;
	}
	while((entry = readdir(dir)) != NULL){
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strstr(entry->d_name, ".git")!=NULL){
			continue;
		}
		char path[MAX_SIZE];
		snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
		char pathToWrite[MAX_SIZE];
		strcpy(pathToWrite, path);
		struct stat statbuf;
		if(stat(path, &statbuf) == 0 && curr_list_line_num < MAX_ENTRIES){
			if(S_ISDIR(statbuf.st_mode)){
				fprintf(loaded_list, strcat(pathToWrite, ",dir\n"));
				addReccursiveFilesToList(path);
			} else {
				addFileToList(pathToWrite);
			}
			curr_list_line_num++;
		}
	}
	closedir(dir);
	return;
}

void addDirectoryToList(char *new_entry){
	if(loaded_list != NULL){
		fclose(loaded_list);
	}
	loaded_list = fopen(curr_txtList, "a");	
	char directory[MAX_SIZE];
	strcpy(directory, new_entry);
	if(curr_list_line_num == MAX_ENTRIES){
		return;
	}
	fprintf(loaded_list, strcat(new_entry, ",dir\n"));

}

int btn_chooseFileToAdd(Ihandle *self){
	Ihandle *file_dlg;
	Ihandle *temp_label = (Ihandle *)IupGetAttribute(self, "FILE");
	Ihandle *parent = (Ihandle *)IupGetAttribute(self, "PAR_DLG");
	file_dlg = IupFileDlg();
	IupSetAttributes(file_dlg, "DIALOGTYPE=OPEN, TITLE=\"Choose your file\"");
	IupPopup(file_dlg, IUP_CENTER, IUP_CENTER);
	switch(IupGetInt(file_dlg, "STATUS")){
		case 0:
			const char *file_path = IupGetAttribute(file_dlg, "VALUE");
			IupSetAttribute(temp_label, "TITLE", file_path);
			printf("%s\n", file_path);
			IupSetAttribute(parent, "FILE", strdup(file_path));
			printf("[DEBUG] filepath should be readable: %s\n", IupGetAttribute(parent, "FILE"));
		default:
			break;
	}
	IupMap(parent);
	IupRefresh(parent);
	IupDestroy(file_dlg);
	return IUP_DEFAULT;
}

int btn_add_file(Ihandle *self){
	Ihandle *temp_dlg = (Ihandle *)IupGetAttribute(self, "PAR_DLG");
	char *file_path = IupGetAttribute(temp_dlg, "FILE");
	printf("[DEBUG] Retrieved filepath: %p\n", (void *)file_path);
	if(file_path){
		printf("[DEBUG]File path is: %s\n", file_path);
	} else {
		printf("FILE PATH IS NULL!");
	}
	printf("%i\n", isFileCreated(file_path));
	printf("%s\n", file_path);
	if(isFileCreated(file_path)){
		printf("I am adding something to the list...\n");
		addFileToList(file_path);
		returnListEntries();
		IupSetAttribute(temp_dlg, "FILE", NULL);
	}
	printf("Preparing to destroy...\n");
	IupDestroy(temp_dlg);
	printf("Destroyed\n");
	return IUP_DEFAULT;
}

int btn_addFileToList(Ihandle *self){
	Ihandle *temp_dlg, *temp_vbox, *temp_label, *btn_add, *btn_accept, *btn_cancel;
	printf("Opening file picker\n");
	temp_label = IupLabel("");
	btn_accept = IupButton("Add file to list", NULL);
	btn_cancel = IupButton("Cancel", NULL);
	btn_add = IupButton("Choose file", NULL);
	temp_vbox = IupVbox(temp_label, btn_add, btn_accept, btn_cancel, NULL);
	temp_dlg = IupDialog(temp_vbox);
	IupSetAttribute(temp_dlg, "TITLE", "Add File");
	IupSetAttribute(temp_dlg, "SIZE", "QUARTERxQUARTER");
	IupSetAttribute(temp_vbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(temp_vbox, "GAP", "10");
	IupSetAttribute(temp_vbox, "MARGIN", "10x10");
	IupSetAttribute(btn_accept, "PAR_DLG", (char *)temp_dlg);
	IupSetAttribute(btn_add, "PAR_DLG", (char *)temp_dlg);
	IupSetAttribute(btn_accept, "FILE", (char *)temp_label);
	IupSetAttribute(btn_add, "FILE", (char *)temp_label);
	IupSetCallback(btn_accept, "ACTION", (Icallback) btn_add_file);
	IupSetCallback(btn_add, "ACTION", (Icallback) btn_chooseFileToAdd);
	IupShowXY(temp_dlg, IUP_CENTER, IUP_CENTER);
}

int btn_chooseDirectoryToAdd(Ihandle *self){
	Ihandle *file_dlg;
	Ihandle *temp_label = (Ihandle *)IupGetAttribute(self, "FILE");
	Ihandle *parent = (Ihandle *)IupGetAttribute(self, "PAR_DLG");
	file_dlg = IupFileDlg();
	IupSetAttributes(file_dlg, "DIALOGTYPE=DIR, TITLE=\"Choose your file\"");
	IupPopup(file_dlg, IUP_CENTER, IUP_CENTER);
	switch(IupGetInt(file_dlg, "STATUS")){
		case 0:
			const char *file_path = IupGetAttribute(file_dlg, "VALUE");
			IupSetAttribute(temp_label, "TITLE", file_path);
			printf("%s\n", file_path);
			IupSetAttribute(parent, "FILE", strdup(file_path));
			printf("[DEBUG] filepath should be readable: %s\n", IupGetAttribute(parent, "FILE"));
		default:
			break;
	}
	IupMap(parent);
	IupRefresh(parent);
	IupDestroy(file_dlg);
	return IUP_DEFAULT;
}

int btn_add_directory(Ihandle *self){
	Ihandle *temp_dlg = (Ihandle *)IupGetAttribute(self, "PAR_DLG");
	char *file_path = IupGetAttribute(temp_dlg, "FILE");
	Ihandle *tgl_recur = (Ihandle *)IupGetAttribute(self, "TOG");
	printf("[DEBUG] Retrieved filepath: %p\n", (void *)file_path);
	if(file_path){
		printf("[DEBUG]File path is: %s\n", file_path);
	} else {
		printf("FILE PATH IS NULL!");
	}
	printf("%i\n", isFileCreated(file_path));
	printf("%s\n", file_path);
	if(isFolderCreated(file_path)){
		printf("I am adding something to the list...\n");
		char temp_file_path[MAX_SIZE];
		strcpy(temp_file_path, file_path);
		addDirectoryToList(file_path);
		printf("%s\n", IupGetAttribute(tgl_recur, "VALUE"));
		if(strcmp(IupGetAttribute(tgl_recur, "VALUE"), "ON") == 0){
			printf("Let's do this\n");
			addReccursiveFilesToList(temp_file_path);	
		}
		returnListEntries();
		IupSetAttribute(temp_dlg, "FILE", NULL);
	}
	printf("Preparing to destroy...\n");
	IupDestroy(temp_dlg);
	printf("Destroyed\n");
	return IUP_DEFAULT;
}

int btn_addDirectoryToList(Ihandle *self){
	Ihandle *temp_dlg, *temp_vbox, *temp_label, *btn_add, *btn_accept, *btn_cancel, *tgl_recur;
	printf("Opening directory picker\n");
	temp_label = IupLabel("");
	btn_accept = IupButton("Add directory to list", NULL);
	btn_cancel = IupButton("Cancel", NULL);
	btn_add = IupButton("Choose file", NULL);
	tgl_recur = IupToggle("Recursively add all files and directories?\n", NULL);
	temp_vbox = IupVbox(temp_label, tgl_recur, btn_add, btn_accept, btn_cancel, NULL);
	temp_dlg = IupDialog(temp_vbox);
	IupSetAttribute(temp_dlg, "TITLE", "Add directory");
	IupSetAttribute(temp_dlg, "SIZE", "QUARTERxQUARTER");
	IupSetAttribute(temp_vbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(temp_vbox, "GAP", "10");
	IupSetAttribute(temp_vbox, "MARGIN", "10x10");
	IupSetAttribute(btn_accept, "PAR_DLG", (char *)temp_dlg);
	IupSetAttribute(btn_add, "PAR_DLG", (char *)temp_dlg);
	IupSetAttribute(btn_add, "FILE", (char *)temp_label);
	IupSetAttribute(btn_accept, "TOG", (char *)tgl_recur);
	IupSetCallback(btn_accept, "ACTION", (Icallback) btn_add_directory);
	IupSetCallback(btn_add, "ACTION", (Icallback) btn_chooseDirectoryToAdd);
	IupShowXY(temp_dlg, IUP_CENTER, IUP_CENTER);
}

void writeTextFile(){
	if(remove(curr_txtList)!=0){
		printf("Error removing\n");
	}
	char tmp[MAX_SIZE];
	strcpy(tmp, curr_list);
	strcpy(curr_txtList, strcat(tmp, "t"));
	loaded_list = fopen(curr_list, "rb");
	loaded_listt = fopen(curr_txtList, "w");
	char lines[MAX_SIZE+2048];
	char line[MAX_SIZE+2048];
	int num_lines = 0;
	if(loaded_list == NULL){
		printf("This is fishy...\n");
	}
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
	if(remove(curr_txtList)!= 0){
		return;
	}
	fclose(loaded_list);
	return;
}

int btn_open_file(Ihandle *self){
	Ihandle *filedlg;
	filedlg= IupFileDlg();
	IupSetAttributes(filedlg, "DIALOGTYPE=OPEN, TITLE=\"Open .tfo file\"");
	IupSetAttributes(filedlg, "FILTER=\"*.tfo\", FILTERINFO=\"Tagged File Output Files\"");
	IupPopup(filedlg, IUP_CENTER, IUP_CENTER);
	switch(IupGetInt(filedlg, "STATUS")){
		case 0:
			writeBinaryFile();
			strcpy(curr_list, IupGetAttribute(filedlg, "VALUE"));
			printf("Current list: %s\n", curr_list);
			writeTextFile();
			returnListEntries();
			break;
		default:
			break;
	}
	IupDestroy(filedlg);
	return IUP_DEFAULT;
}

int btn_save_file(Ihandle *self){
	Ihandle *file_dlg;
	file_dlg = IupFileDlg();
	IupSetAttributes(file_dlg, "DIALOGTYPE=SAVE, TITLE=\"Save .tfo file\"");
	IupSetAttributes(file_dlg, "FILTER = \"*.tfo\", FILTERINFO=\"Tagged File Object\"");
	IupPopup(file_dlg, IUP_CENTER, IUP_CENTER);
	switch(IupGetInt(file_dlg, "STATUS")){
		case 1:
			if(loaded_list != NULL){
				fclose(loaded_list);
			}
			strcpy(curr_list,IupGetAttribute(file_dlg, "VALUE"));
			char line[MAX_SIZE+2048];
			printf("We make our stand\n");
			strcpy(line, list_name);
			strcat(line, "\n");
			FILE *temp_list = fopen(curr_list, "wb");
			printf("File is opened\n");
			fwrite(&line, sizeof(line), 1, temp_list);
			printf("Head is written\n");
			if(tm.toggles){
				for(int i=0; i<tm.count; i++){
					if(tm.toggles[i] != NULL){
						char next_line[MAX_SIZE+2048];
						strcpy(next_line, IupGetAttribute(tm.toggles[i], "TITLE"));
						printf("Got line %s\n", next_line);
						strcat(next_line, "\n");
						printf("Preparing to write line which is %s\n", next_line);
						fwrite(next_line, sizeof(next_line), 1, temp_list);
						printf("Line written esucessfully\n");
					}
				}
			}
			fclose(temp_list);
			if(isFileCreated(curr_list)){
				writeTextFile();
				returnListEntries();
			}
			break;
		case 0:
			writeBinaryFile();
			writeTextFile();
			returnListEntries();
			break;
		default:
			printf("Something went horribly wrong\n");
			return IUP_CLOSE;
	}
	IupDestroy(file_dlg);
	return IUP_DEFAULT;
}

int btn_accept_list_name(Ihandle *self){
	Ihandle *entered_text = (Ihandle *)IupGetAttribute(self, "ENTERTEXT");
	Ihandle *tmp_dlg = (Ihandle *)IupGetAttribute(self, "PAR_DLG");
	strcpy(list_name, IupGetAttribute(entered_text, "VALUE"));
	strcat(list_name, "\n");
	if(loaded_list != NULL){
		fclose(loaded_list);
	}
	loaded_list = fopen(curr_txtList, "w");
	fprintf(loaded_list, list_name);
	if(tm.toggles){
		for(int i = 0; i<tm.count; i++){
			if(tm.toggles[i] != NULL){
				char next_line[MAX_SIZE+2048];
				strcpy(next_line, IupGetAttribute(tm.toggles[i], "TITLE"));
				strcat(next_line, "\n");
				fprintf(loaded_list, next_line);
			}
		}
	}
	IupDestroy(tmp_dlg);
	returnListEntries();
	return IUP_DEFAULT;
}

int btn_name_list(Ihandle *self){
	Ihandle *namer, *temp_vbox, *temp_hbox, *namer_label, *namer_text, *btn_temp_accept;
	char text_size[4];
	sprintf(text_size, "%d", MAX_SIZE);
	namer_label = IupLabel("List name: ");
	namer_text = IupText(NULL);
	IupSetAttribute(namer_text, "NC", text_size);
	IupSetAttribute(namer_text, "CUEBANNER", "NEW_TAGGING_FILE");
	temp_hbox = IupHbox(namer_label, namer_text, NULL);
	btn_temp_accept = IupButton("Rename list", NULL);
	temp_vbox = IupVbox(temp_hbox, btn_temp_accept, NULL);
	namer = IupDialog(temp_vbox);
	IupSetAttribute(namer, "TITLE", "Rename list");
	IupSetAttribute(namer, "SIZE", "QUARTERxQUARTER");
	IupSetAttribute(temp_hbox, "GAP", "10");
       	IupSetAttribute(temp_hbox, "MARGIN", "10x10");
	IupSetAttribute(temp_vbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(temp_vbox, "GAP", "10");
	IupSetAttribute(temp_vbox, "MARGIN", "10x10");
	IupSetAttribute(namer_text, "SIZE", "100");
	IupSetCallback(btn_temp_accept, "ACTION", (Icallback) btn_accept_list_name);
	IupSetAttribute(btn_temp_accept, "ENTERTEXT", (char *)namer_text);
	IupSetAttribute(btn_temp_accept, "PAR_DLG", (char *)namer);
	IupShowXY(namer, IUP_CENTER, IUP_CENTER);
	return IUP_DEFAULT;
}

int btn_remove_entries(Ihandle *self){
	Ihandle *par = (Ihandle *)IupGetAttribute(self, "PAR_DLG");
	if(tm.toggles){
		for(int i = 0; i < tm.count; i++){
			if(tm.toggles[i] == NULL){
				continue;
			}
			if(strcmp(IupGetAttribute(tm.toggles[i], "VALUE"), "ON") == 0){
				IupDestroy(tm.toggles[i]);
				tm.toggles[i] = NULL;				
			}
		}
		if(loaded_list != NULL){
			fclose(loaded_list);
		}
		loaded_list = fopen(curr_txtList, "w");
		strcat(list_name, "\n");
		fprintf(loaded_list, list_name);
		if(tm.toggles){
			for(int i = 0; i<tm.count; i++){
				if(tm.toggles[i] != NULL){
					char next_line[MAX_SIZE+2048];
					strcpy(next_line, IupGetAttribute(tm.toggles[i], "TITLE"));
					strcat(next_line, "\n");
					fprintf(loaded_list, next_line);
				}
			}
		}
	}
	IupDestroy(par);
	returnListEntries();
	return IUP_DEFAULT;
}

int btn_cancel(Ihandle *self){
	Ihandle *par = (Ihandle *)IupGetAttribute(self, "PAR_DLG");
	IupDestroy(par);
	return IUP_DEFAULT;
}

int btn_remove_item(Ihandle *self){
	Ihandle *confirm, *confirm_msg, *btn_yes, *btn_no, *tmp_vbox, *tmp_hbox;
	confirm_msg = IupLabel("Do you wish to delete these items from the list?");
	btn_yes = IupButton("Yes", NULL);
	btn_no = IupButton("No", NULL);
	tmp_hbox = IupHbox(btn_yes, btn_no, NULL);
	tmp_vbox = IupVbox(confirm_msg, tmp_hbox, NULL);
	confirm = IupDialog(tmp_vbox);
	IupSetAttribute(confirm, "TITLE", "Confirm deleting list entries");
	IupSetAttribute(confirm, "SIZE", "QUARTERxQUARTER");
	IupSetAttribute(tmp_vbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(tmp_vbox, "GAP", "10");
	IupSetAttribute(tmp_vbox, "MARGIN", "10x10");
	IupSetAttribute(btn_yes, "PAR_DLG", (char *)confirm);
	IupSetAttribute(btn_no, "PAR_DLG", (char *)confirm);
	IupSetCallback(btn_yes, "ACTION", (Icallback) btn_remove_entries);
	IupSetCallback(btn_no, "ACTION", (Icallback) btn_cancel);
	IupShowXY(confirm, IUP_CENTER, IUP_CENTER);
	return IUP_DEFAULT;
}

int btn_addToEntries(Ihandle *self){
	Ihandle *par = (Ihandle *)IupGetAttribute(self, "PAR_DLG");
	Ihandle *tag = (Ihandle *)IupGetAttribute(self, "NEW_TAG");
	if(tm.toggles){
		for(int i = 0; i<tm.count; i++){
			if(tm.toggles[i] == NULL){
				continue;
			}
			if(strcmp(IupGetAttribute(tm.toggles[i], "VALUE"), "ON") == 0){
				char edited_line[MAX_SIZE+2048];
				printf("Found a file to add tag to\n");
				char curr_line[MAX_SIZE+2048];
				strcpy(curr_line, IupGetAttribute(tm.toggles[i], "TITLE"));
				char *token = strtok(curr_line, ",");
				int token_ctr = 0;
				int max_tags = 0;
				while(token!=NULL){
					if(token_ctr <= 9){
						token_ctr++;
					} else {
						max_tags = 1;
						break;
					}
					token=strtok(NULL,",");
				}
				if(max_tags != 1){
					strcpy(edited_line, IupGetAttribute(tm.toggles[i], "TITLE"));
					strcat(edited_line, ",");
					strcat(edited_line, IupGetAttribute(tag, "VALUE"));
					printf("Saving %s\n", edited_line);
					IupSetAttribute(tm.toggles[i], "TITLE", edited_line);
				}
			}
		}
	}
	if(loaded_list != NULL){
		fclose(loaded_list);
	}
	loaded_list = fopen(curr_txtList, "w");
	strcat(list_name, "\n");
	fprintf(loaded_list, list_name);
	if(tm.toggles){
		for(int i = 0; i<tm.count; i++){
			if(tm.toggles[i] == NULL){
				continue;
			}
			char new_line[MAX_SIZE+2048];
			strcpy(new_line, IupGetAttribute(tm.toggles[i], "TITLE"));
			strcat(new_line, "\n");
			fprintf(loaded_list, new_line);
		}
	}
	IupDestroy(par);
	returnListEntries();
	return IUP_DEFAULT;
}


int btn_add_tag(Ihandle *self){
	Ihandle *namer, *namer_label, *namer_text, *btn_accept, *btn_cancel, *tmp_vbox, *tmp_hbox1, *tmp_hbox2;
	namer_label = IupLabel("Enter new tag: ");
	namer_text = IupText(NULL);
	btn_accept = IupButton("Accept new tag", NULL);
	btn_cancel = IupButton("Cancel", NULL);
	tmp_hbox1 = IupHbox(namer_label, namer_text, NULL);
	tmp_hbox2 = IupHbox(btn_accept, btn_cancel, NULL);
	tmp_vbox = IupVbox(tmp_hbox1, tmp_hbox2, NULL);
	namer = IupDialog(tmp_vbox);
	IupSetAttribute(namer, "TITLE", "Add Tag");
	IupSetAttribute(namer, "SIZE", "QUARTERxQUARTER");
	IupSetAttribute(btn_accept, "PAR_DLG", (char *)namer);
	IupSetAttribute(btn_accept, "NEW_TAG", (char *)namer_text);
	IupSetAttribute(btn_cancel, "PAR_DLG", (char *)namer);
	IupSetCallback(btn_accept, "ACTION", (Icallback) btn_addToEntries);
	IupSetCallback(btn_cancel, "ACTION", (Icallback) btn_cancel);
	IupShowXY(namer, IUP_CENTER, IUP_CENTER);
	return IUP_DEFAULT;
}

int main (int argc, char **argv){
	char utsFilePath[MAX_SIZE]="";
	printf("Finding main file...\n");
	#ifdef __linux__
		printf("Using Linux\n");
		strcat(utsFilePath,"/home/");
	#elif _WIN32
		strcat(utsFilePath,"C:/Users/");
	#else
		IupMessage("Unsupported OS.", "This OS is unsupported, sorry");
		return EXIT_FAILURE;
	#endif
	if(strcmp(getUserName(), "ERR_NO_NAME")==0){
		printf("Huh?");
		IupMessage("ERROR", "Not a real user...");
		return EXIT_FAILURE;
	}
	strcat(utsFilePath, getUserName());
	strcat(utsFilePath, "/Documents/UTS Util");
	printf("%s\n", utsFilePath);
	printf("Checking if main folder exists\n");
	if(!isFolderCreated(utsFilePath)){
		#ifdef __linux__
			mkdir(utsFilePath, 0700);
		#elif _WIN32
			mkdir(utsFilePath);
		#endif
		char success_message[] = "Successfully created folder at ";
		strcat(success_message, utsFilePath);
		IupMessage("Success", success_message);	
	}
	printf("Found the file correctly\n");
 	strcat(utsFilePath, "/main_list.tfo");
	if(!isFileCreated(utsFilePath)){
		FILE *main_list = fopen(utsFilePath, "wb");
		char header[MAX_SIZE+2048] = "Main\n";
		fwrite(header, sizeof(header), 1, main_list);
		fclose(main_list);
		IupMessage("Success", "Successfully created main list file");
		printf("%s\n", utsFilePath);
		strcpy(curr_list, utsFilePath);
		writeTextFile();
	} else {
		strcpy(curr_list, utsFilePath);
		writeTextFile();
		printf("Opened list\n");
		if(loaded_list == NULL){
			printf("This is not right\n");
		}
	}
	printf("Getting lis contents...\n");
	printf("%s\n", curr_list);	
	printf("Starting GUI\n");
	IupOpen(&argc, &argv);
	heading = IupLabel("Welcome to GUTS, choose your options:");
	item_open = IupItem("Open", NULL);
	item_save = IupItem("Save", NULL);
	list_head = IupLabel("");
	tm.vbox = IupVbox(NULL);
	tm.list_container = IupScrollBox(tm.vbox);
	printf("Setting up scroll box...\n");
	returnListEntries();
	IupSetAttribute(item_open, "KEY", "O");
	IupSetAttribute(item_save, "KEY", "S");
	file_menu = IupMenu(item_open, item_save, NULL);
	sub1_menu = IupSubmenu("File", file_menu);
	menu = IupMenu(sub1_menu, NULL);
	IupSetHandle("main_menu", menu);
	button = IupButton("Add File To List", NULL);
	button_dir = IupButton("Add Directory To List", NULL);
	button_list_name = IupButton("Rename list", NULL);
	button_remove_entries = IupButton("Remove selected entries", NULL);
	button_add_tag = IupButton("Add tag to selected entries", NULL);
	printf("Setting up the vbox...\n");
	vbox=IupVbox(heading, list_head,  tm.list_container, button, button_dir, button_add_tag, button_remove_entries, button_list_name, NULL);
	dlg=IupDialog(vbox);
	IupSetAttribute(dlg, "MENU", "main_menu");
	IupSetAttribute(dlg, "TITLE", "GUTS");
	IupSetAttribute(dlg, "SIZE", "FULLxFULL");
	IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(vbox, "GAP", "10");
	IupSetAttribute(vbox, "MARGIN", "10x10");
	IupSetCallback(button_add_tag, "ACTION", (Icallback) btn_add_tag);
	IupSetCallback(item_open, "ACTION", (Icallback) btn_open_file);
	IupSetCallback(button, "ACTION", (Icallback) btn_addFileToList);
	IupSetCallback(button_dir, "ACTION", (Icallback) btn_addDirectoryToList);
	IupSetCallback(item_save, "ACTION", (Icallback) btn_save_file);
	IupSetCallback(button_list_name, "ACTION", (Icallback) btn_name_list);
	IupSetCallback(button_remove_entries, "ACTION", (Icallback)btn_remove_item);
	IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
	IupMainLoop();
	if(remove(curr_txtList)!=0){
		return 1;
	}
	IupClose();
	return EXIT_SUCCESS;
}
