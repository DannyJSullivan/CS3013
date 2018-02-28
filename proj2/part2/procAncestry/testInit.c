#include <stdio.h>
#include <unistd.h>

int main(){
	char *const argv[]={"./procAncestry", "1", NULL};
	execvp(argv[0], argv);
}
