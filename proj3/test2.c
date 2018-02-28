#include <stdlib.h>
#include <unistd.h>
int main(){
	char *const argv[]={"./bathroomsim", "500","30", "10" , "5", NULL};
	execvp(argv[0], argv);
}
