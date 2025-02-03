#include <stdio.h>
#ifdef __linux__
	#include <pwd.h>
#elif _WIN32
	#include <Windows.h>
#endif
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

const char *getUserName(){
	#ifdef __linux__
		uid_t uid = geteuid();
		struct passwd *pw = getpwuid(uid);
		if(pw){
			return pw->pw_name;
		} else {
			return "ERR_NO_NAME";
		}
	#elif _WIN32
		return getenv("USERNAME");
	#endif
}
