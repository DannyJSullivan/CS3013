#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#define __NR_cs3013_syscall1 333


long testCall1(void) {
	return (long) syscall(__NR_cs3013_syscall1);
}

int main(void){
	printf("The system call 1 value was: %ld\n",testCall1());
}
