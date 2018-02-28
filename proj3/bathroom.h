#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <math.h>
#include <unistd.h>

typedef enum {male,female} gender;

#define VACANT -1
#define MALE 0
#define FEMALE 1

extern pthread_mutex_t lock;
extern pthread_cond_t change;
extern pthread_mutex_t print;

extern double meantime;
extern double arrival;
extern int loop;
extern int users;

extern int flag;

void Enter(gender g, int * wait);

void Leave();

typedef struct bathroom{
	int users;

	int quers;
	
	int ques;
	int uses;
	double vacanttime;
	double occtime;
	double avgqlen;
	double avgnumusers;

} bathroom;

extern bathroom abathroom;

void Initialize(bathroom * a);

void Finalize(bathroom * a);

double generateStandardRand(double mean);
