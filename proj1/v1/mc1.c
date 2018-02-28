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
#include "mc.h"

int main() {
	struct pinfo** p = (struct pinfo**) malloc(sizeof(p));
	char** cmdList = (char**) calloc(MAX_USERCMD, sizeof(char*));
	uint8_t g_cmdCt = 0;
	uint8_t* cmdCt = &g_cmdCt;
	setbuf(stdout, NULL); // Changing stdout buffer required for correct printing order
	setbuf(stdin, NULL);

	printf("\n===== Mid Day Commander, v1 =====\n");
	while (1) {
		usleep(2e5);
		printf("\nG'day, Commander! What command would you like to run?\n");
		printf("\t0. whoami : Prints out the result of the whoami command\n");
		printf("\t1. last   : Prints out the result of the last command\n");
		printf(
				"\t2. ls     : Prints out the result of a listing on a user-specified path\n");
		printUserCmd(cmdList, cmdCt);
		printf("\ta. add command : Adds a new command to the menu\n");
		printf("\tc. change directory : Changes process working directory\n");
		printf("\te. exit : Leave Mid-Day Commander\n");
		printf("\tp. pwd : Prints working directory\n");
		printf("Option?: ");

		char* in = (char*) malloc(sizeof(char*));
		fgets(in, MAX_ARG_LEN, stdin);
		if (feof(stdin)) {
			*in = 'e';
			alphOp(in, cmdList, cmdCt);
		} else if (isdigit(*in)) {
			int in_num = atoi(in);
			if (in_num > 2) {
				userOp(in_num, cmdList, cmdCt);
			} else {
				numOp(in_num);
			}
		} else {
			alphOp(in, cmdList, cmdCt);
		}
		free(in);
	}
	for (int i; i < MAX_USERCMD; i++) {
		free(cmdList[i]);
	}
	return EXIT_SUCCESS;
}

int userOp(uint8_t op, char* cmdList[], uint8_t* cmdCt) {
	struct timeval start_time;
	struct timeval end_time;
	struct rusage ru;
	pid_t pid;
	char* full_cmd = (char*) malloc(sizeof(MAX_USERCMD_LEN * sizeof(char)));
	char** pargv = (char**) calloc(MAX_ARGS, sizeof(char*));
	long page_fault_start, page_fault_end;
	long page_rec_start, page_rec_end;
	strcpy(full_cmd, cmdList[op - CMD_OFFSET]);

	if (op > (*cmdCt + (MAX_USERCMD - 1))) {
		fprintf(stderr, "unexpected user input\n");
		return 1;
	} else {
		printf("\n-- Command: %s --\n", full_cmd);
		gettimeofday(&start_time, NULL);

		parseCmd(full_cmd, pargv);

		pid = fork();

		getrusage(RUSAGE_CHILDREN, &ru);
		page_fault_start = ru.ru_majflt;
		page_rec_start = ru.ru_minflt;

		char* cmd = pargv[0];
		newProcess(cmd, pid, pargv);

		getrusage(RUSAGE_CHILDREN, &ru);
		page_fault_end = ru.ru_majflt;
		page_rec_end = ru.ru_minflt;

		gettimeofday(&end_time, NULL);
		printStats(start_time, end_time, (page_fault_end - page_fault_start),
				(page_rec_end - page_rec_start));
	}
	return EXIT_SUCCESS;
}

int alphOp(char* cin, char* cmdList[], uint8_t* cmdCt) {
	char* cmd;
	pid_t pid;
	switch (tolower(*cin)) {
	case 'a':
		if (*cmdCt == MAX_USERCMD) {
			printf("ERROR: Max number of user added commands reached\n");
			break;
		}
		printf("\n-- Add a command --\n");
		printf("Command to add?: ");
		char* usercmd = (char*) malloc(MAX_USERCMD_LEN * sizeof(char));
		cmdList[*cmdCt] = (char*) malloc(MAX_USERCMD_LEN * sizeof(char));
		fgets(usercmd, MAX_USERCMD_LEN, stdin);
		parseCmdN(usercmd);

		printf("Okay, added with ID %u\n", (*cmdCt + CMD_OFFSET));
		strcpy(cmdList[(*cmdCt)++], usercmd);
		free(usercmd);
		break;
	case 'c':
		printf("\n-- Change Directory --\n");
		printf("New Directory?: ");
		char* cdir = (char*) malloc(MAX_DIR_LEN * sizeof(char));
		fgets(cdir, MAX_DIR_LEN, stdin);
		parseCmdN(cdir);
		chdir(cdir);
		free(cdir);
		break;
	case 'e':
		printf("\nLogging you out, Commander.\n");
		exit(EXIT_SUCCESS);
	case 'p':
		printf("\n-- Current Directory --\n");
		char cwd[MAX_DIR_LEN];
		if (getcwd(cwd, MAX_DIR_LEN) == NULL) {
			perror("getcwd() ERROR");
		} else {
			printf("Directory: %s\n", cwd);
		}
		break;
	default:
		fprintf(stderr, "unexpected user input\n");
		break;
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
		char* pargv1[] = { cmd, "-n 1", NULL }; // -n flag specifies num of output
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
		fprintf(stderr, "unexpected user input\n");
		break;
	}
	return EXIT_SUCCESS;
}

int parseCmd(char* cmd, char* pargv[]) {
	char* tokens = strtok(cmd, " \n");

	int i = 0;
	while (tokens != NULL) {
		pargv[i] = (char*) malloc(sizeof(char*));
		strcpy(pargv[i], tokens);

		tokens = strtok(NULL, " \n"); // Get new token
		i++;
	}
	pargv[i] = NULL; // NULL terminated
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

int newProcess(char* cmd, pid_t pid, char* pargv[]) {
	if (pid < 0) {
		fprintf(stderr, "creation of new process failed\n");
		exit(EXIT_FAILURE);
	} else if (!pid) {
		execvp(cmd, pargv); // Replace forked child
	} else {
		wait(NULL); // Wait for child process
	}
	return EXIT_SUCCESS;
}

int printStats(struct timeval start_time, struct timeval end_time,
		long page_fault, long page_rec) {
	printf("\n-- Statistics --\n");
	/*
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

int printUserCmd(char* cmdList[], uint8_t* cmdCt) {
	if (*cmdCt == 0) {
		return 1;
	} else {
		for (int i = 0; i < *cmdCt; i++) {
			printf("\t%d. %s : User added command\n", (i + 3), cmdList[i]);
		}
	}
	return EXIT_SUCCESS;
}
