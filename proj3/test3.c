#include <stdlib.h>
#include <unistd.h>
int main(){
	char *const argv[]={"./bathroomsim", "10000","10", "3" , "3", NULL};
	execvp(argv[0], argv);
}
