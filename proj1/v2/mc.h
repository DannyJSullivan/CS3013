#ifndef MC_H_
#define MC_H_

#include <unistd.h>
#include <stdint.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>

#define MAX_ARG_LEN 200
#define MAX_ARGS 33
#define MAX_DIR_LEN 200
#define MAX_USERCMD 100
#define MAX_USERCMD_LEN 100
#define CMD_OFFSET 3 // There are 3 system commands, followed by MAX_USERMCD user-added commands
#define MAX_PCT 20 // Maximum number of processes to keep track of at once

struct pinfo_t {
	char* pname;
	pid_t pid;
	struct timeval* start_time;
	struct rusage pru;
};

int numOp(uint8_t op);
int alphOp(char* cin, char* cmdList[], uint8_t* cmdCt, uint8_t* bgPCt,
		struct pinfo_t** pinfoarr, _Bool flag);
int userOp(uint8_t op, char* cmdList[], uint8_t* cmdCt, uint8_t* bgPCt,
		struct pinfo_t** pinfoarr);
int parseCmd(char* cmd, char* pargv[]);
int parseCmdN(char* cmd);
int newProcess(char* cmd, pid_t pid, char* pargv[]);
int newBgProcess(char* full_cmd, char* cmdList[], uint8_t op,
		struct pinfo_t** pinfoarr, uint8_t* bgPCt);
int removeBgProcess(uint8_t* bgPCt, uint8_t parrnum, struct pinfo_t** pinfoarr);
void printStats(struct timeval start_time, struct timeval end_time,
		long page_fault, long page_rec);
void printUserCmd(char* cmdList[], uint8_t* cmdCt);

#endif /* MC_H_ */
