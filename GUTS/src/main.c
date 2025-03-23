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

char curr_list[MAX_SIZE]="";
char curr_txtList[MAX_SIZE]="";
FILE *loaded_list;
FILE *loaded_listt;
int curr_list_line_num;
char *listContents;
const int MAX_ENTRIES = 250;
Ihandle *dlg, *button,  *vbox, *heading, *list_contents, *list_scroll, *item_open, *file_menu, *sub1_menu, *menu;

char *returnListEntries(){
	if(loaded_list != NULL){
		fclose(loaded_list);	
	}
	loaded_list = fopen(curr_txtList, "r");
	char line[MAX_SIZE+2048];
	char *list = malloc((MAX_SIZE+2048)*MAX_ENTRIES);
	*list = '\0';
	printf("%s\n", list);
	while(fgets(line, sizeof(line), loaded_list)){
		strcat(list, line);
	}
	return list;
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

int btn_get_curr_user(Ihandle *self){
	IupMessage("Current User", getUserName());
	return IUP_CLOSE;
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
			listContents = returnListEntries();
			break;
		default:
			break;
	}
	IupSetAttribute(list_contents, "TITLE", listContents);
	IupMap(dlg);
	IupDestroy(filedlg);
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
	listContents = returnListEntries();
	printf("Starting GUI\n");
	IupOpen(&argc, &argv);
	heading = IupLabel("Welcome to GUTS, choose your options:");
	list_contents = IupLabel(listContents);
	list_scroll = IupScrollBox(list_contents);
	item_open = IupItem("Open", NULL);
	IupSetAttribute(item_open, "KEY", "O");
	file_menu = IupMenu(item_open, NULL);
	sub1_menu = IupSubmenu("File", file_menu);
	menu = IupMenu(sub1_menu, NULL);
	IupSetHandle("main_menu", menu);
	button = IupButton("Get current user name", NULL);
	vbox=IupVbox(heading, list_scroll, button, NULL);
	dlg=IupDialog(vbox);
	IupSetAttribute(dlg, "MENU", "main_menu");
	IupSetAttribute(dlg, "TITLE", "GUTS");
	IupSetAttribute(dlg, "SIZE", "FULLxFULL");
	IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(vbox, "GAP", "10");
	IupSetAttribute(vbox, "MARGIN", "10x10");
	IupSetCallback(item_open, "ACTION", (Icallback) btn_open_file);
	IupSetCallback(button, "ACTION", (Icallback) btn_get_curr_user);
	IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
	IupMainLoop();
	IupClose();
	writeBinaryFile();
	free(listContents);
	return EXIT_SUCCESS;
}
