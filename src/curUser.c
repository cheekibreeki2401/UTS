#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

const char *getUserName(){
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	if(pw){
		return pw->pw_name;
	} else {
		return "ERR_NO_NAME";
	}
}
