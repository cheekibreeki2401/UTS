#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iup.h>
#include <sys/types.h>
#include <unistd.h>
int main (int argc, char **argv){
	IupOpen(&argc, &argv);
	IupMessage("GUTS", "This is GUTS");
	IupClose();
	return EXIT_SUCCESS;
}
