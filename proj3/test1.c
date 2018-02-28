#include <stdlib.h>
#include <unistd.h>
int main(){
	char *const argv[]={"./bathroomsim", "1000","15", "5" , "3", NULL};
	execvp(argv[0], argv);
}
