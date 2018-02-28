#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(){
	int pid=getpid();
	char str[15];
	sprintf(str, "%d", pid);
	char *const argv[]={"./procAncestry", str, NULL};
	execvp(argv[0], argv);
}
