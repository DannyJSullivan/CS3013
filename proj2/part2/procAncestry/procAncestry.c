#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


#define __NR_cs3013_syscall2 334

struct ancestry{
	pid_t ancestors[10];
	pid_t siblings[100];
	pid_t children[100];
};

//testCall2 simply calls the system call 2 that we intercept
long testCall2(unsigned short * a, struct ancestry * ances) {
	return (long) syscall(__NR_cs3013_syscall2,a,ances);
}


int main(int argc, char ** argv) {
	if(argc>1){//if the correct arguments are entered simply run and print the output
		unsigned short val = atoi(argv[1]);//create short pid target
		unsigned short *pid = &val;//assign to a pointer for persistent memory for kernel to read
		struct ancestry * response = (struct ancestry *)malloc(sizeof(struct ancestry));
		printf("The return value of syscall2 is: ");
		printf("%ld\n", testCall2(pid,response));
		printf("\n");
		printf("Process %d Information: \n",val);
		printf("Children: ");
		int i = 0;
		while(response->children[i]!=0){
			printf("%d ", response->children[i]);
			i++;
			if(i > 100){
				printf("\nError too many values for data struct!\n");
				return 1;
			}
		}
		printf("\n");
		printf("Siblings: ");
		i = 0;
		while(response->siblings[i]!=0){
			printf("%d ", response->siblings[i]);
			i++;
			if(i > 100){
				printf("\nError too many values for data struct!\n");
				return 1;
			}
		}
		printf("\n");
		printf("Ancestors: ");
		i = 0;
		while(response->ancestors[i]!=0){
			printf("%d ", response->ancestors[i]);
			i++;
			if(i > 10){
				printf("\nError too many values for data struct!\n");
				return 1;
			}
		}
		printf("\n");
		free(response);
		return 0;
	}
	else{//otherwise fail and prompt for more input
		printf("Not Enough input!\n");
		return 1;
	}
}
