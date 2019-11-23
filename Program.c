#include <stdio.h>
#include <unistd.h>
#include <limits.h>
int main(){

char hostname[HOST_NAME_MAX];
char username[LOGIN_NAME_MAX];
char cwd[1024];
char a;
while(1)
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
	getlogin_r(username, LOGIN_NAME_MAX);
	gethostname(hostname, HOST_NAME_MAX);
       printf("\033[1;36m%s@%s:\033[1;35m~%s\033[0;33m>",username, hostname,cwd);
	scanf("%s",&a);
   }

   return 0;
}



