#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARG_LEN 200
#define MAX_DIR_LEN 200

int numOp(uint8_t op);
int printStats(struct timeval start_time, struct timeval end_time,
		long page_fault, long page_rec);
int parseCmdN(char* cmd);
int newProcess(char* cmd, pid_t pid, char* pargv[]);

int main() {
	printf("\n===== Mid Day Commander, v0 =====\n");
	while (1) {
		usleep(2e5);
		printf("\nG'day, Commander! What command would you like to run?\n");
		printf("\t0. whoami : Prints out the result of the whoami command\n");
		printf("\t1. last   : Prints out the result of the last command\n");
		printf(
				"\t2. ls     : Prints out the result of a listing on a user-specified path\n");
		printf("Option?: ");

		char* in = (char*) malloc(sizeof(char));
		fgets(in, MAX_ARG_LEN, stdin);

		if (feof(stdin)) {
			printf("\n");
			exit(EXIT_SUCCESS);
		} else if (isdigit(*in)) {
			int in_num = atoi(in);
			numOp(in_num);
		} else {
			fprintf(stderr, "unexpected user input");
		}
		free(in);
	}
	return EXIT_SUCCESS;
}

int numOp(uint8_t op) {
	struct timeval start_time;
	struct timeval end_time;
	struct rusage ru;
	pid_t pid;
	char* cmd;
	long page_fault_start, page_fault_end;
	long page_rec_start, page_rec_end;

	switch (op) {
	case 0:
		printf("\n-- Who Am I? --\n");
		gettimeofday(&start_time, NULL);

		pid = fork();

		getrusage(RUSAGE_CHILDREN, &ru);
		page_fault_start = ru.ru_majflt;
		page_rec_start = ru.ru_minflt;

		cmd = "whoami";
		char* pargv0[] = { cmd, NULL };
		newProcess(cmd, pid, pargv0);

		getrusage(RUSAGE_CHILDREN, &ru);
		page_fault_end = ru.ru_majflt;
		page_rec_end = ru.ru_minflt;

		gettimeofday(&end_time, NULL);
		printStats(start_time, end_time, (page_fault_end - page_fault_start),
				(page_rec_end - page_rec_start));
		break;
	case 1:
		printf("\n-- Last Logins --\n");
		gettimeofday(&start_time, NULL);

		pid = fork();

		getrusage(RUSAGE_CHILDREN, &ru);
		page_fault_start = ru.ru_majflt;
		page_rec_start = ru.ru_minflt;

		cmd = "last";
		char* pargv1[] = { cmd, "-n 1", NULL }; // -n flag specifies num of lines of output
		newProcess(cmd, pid, pargv1);

		getrusage(RUSAGE_CHILDREN, &ru);
		page_fault_end = ru.ru_majflt;
		page_rec_end = ru.ru_minflt;

		gettimeofday(&end_time, NULL);
		printStats(start_time, end_time, (page_fault_end - page_fault_start),
				(page_rec_end - page_rec_start));
		break;
	case 2:
		printf("\n-- Directory Listing --\n");

		printf("Arguments?: ");
		char* arg = (char*) malloc(MAX_ARG_LEN * sizeof(char));
		fgets(arg, MAX_ARG_LEN, stdin);
		if (arg[0] != '\n') {
			parseCmdN(arg);
		} else {
			arg = "-al";
		}

		printf("Path?: ");
		char* dirpth = (char*) malloc(MAX_DIR_LEN * sizeof(char));
		fgets(dirpth, MAX_DIR_LEN, stdin);
		parseCmdN(dirpth);
		printf("\n");

		cmd = "ls";
		char* pargv2[] = { cmd, arg, dirpth, NULL };
		gettimeofday(&start_time, NULL);

		pid = fork();

		getrusage(RUSAGE_CHILDREN, &ru);
		page_fault_start = ru.ru_majflt;
		page_rec_start = ru.ru_minflt;
		newProcess(cmd, pid, pargv2);

		getrusage(RUSAGE_CHILDREN, &ru);
		page_fault_end = ru.ru_majflt;
		page_rec_end = ru.ru_minflt;

		gettimeofday(&end_time, NULL);
		printStats(start_time, end_time, (page_fault_end - page_fault_start),
				(page_rec_end - page_rec_start));
		break;
	default:
		fprintf(stderr, "ERROR: Unexpected user input\n");
		break;
	}
	return EXIT_SUCCESS;
}

int newProcess(char* cmd, pid_t pid, char* pargv[]) {
	if (pid < 0) {
		fprintf(stderr, "creation of new process failed");
		exit(1);
	} else if (!pid) {
		execvp(cmd, pargv); // Replace the forked child with the command
	} else {
		wait(NULL); // Wait for child process
	}
	return EXIT_SUCCESS;
}

int parseCmdN(char* cmd) {
	for (int i = 0; i < strlen(cmd); i++) {
		if (cmd[i] == '\n') {
			cmd[i] = '\0';
		}
	}
	return EXIT_SUCCESS;
}

int printStats(struct timeval start_time, struct timeval end_time,
		long page_fault, long page_rec) {
	printf("\n-- Statistics ---\n");
	/*
	 * Notes:
	 * Cast tv_usec attribute of timeval type to float (orig. of time_t)
	 * Attribute tv_usec is in microsecs (e-6) and tv_sec is in secs
	 * Attribute tv_usec is *remainder* of time past tv_sec
	 */
	float tm = (float) (end_time.tv_sec - start_time.tv_sec) * 1000;
	float rtm = (float) (end_time.tv_usec - start_time.tv_usec) / 1000;
	printf("Elapsed time: %.4f milliseconds\n", tm + rtm);
	printf("Page Faults: %ld\n", page_fault);
	printf("Page Faults (reclaimed): %ld\n", page_rec);
	return EXIT_SUCCESS;
}
