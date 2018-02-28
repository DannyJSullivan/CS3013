#include "bathroom.h"


//Individual is a function pointer that takes an int as thread number as the only argument. It runs and
//mimics users waiting/using the bathroom and each thread calls individual once/
void * Individual(void * thread){
	gender g;//generate a random gender
	int randGend=rand()%2;
	g = randGend;
	char * gender;//assign a string for the gender to print later
	
	if(g==MALE){
		gender = "Male";
	}
	else if(g==FEMALE){	
		gender = "Female";
	}
	else{
		gender = "DEBUG";//used for testing
	}
	
	//create timer for lapsed time
	struct timeval before;
	struct timeval after;
	//generate loopcount using generateStandardRand with argv input
	int loopcount = floor(generateStandardRand(loop));
	if(loopcount==0){
		loopcount++;//minimum is 1
	}
	//start collecting data
	int queuemin = 9999999;
	int queuemax = 0;
	int avgqueue = 0;
	int queuetime = 0;
	int numwaits = 0;
	
	for(int i = 0; i < loopcount;i++){
		double arrivalt =  generateStandardRand(arrival);//generate random arrival time and use time
		double uset = generateStandardRand(meantime);
		usleep(arrivalt);//sleep for arrival time to occur
		gettimeofday(&before, NULL);//start timers
		int wait = -1;//create pointer for wait

		Enter(g,&wait);//attempt to enter bathroom
		gettimeofday(&after, NULL);//end timer

		usleep(uset);//sleep for the duration of bathroom use

		Leave();//leave and pass in use time to add to stats
	
		int lapsed = after.tv_usec - before.tv_usec;//calculate lapsed time
		if(lapsed > queuemax){
				queuemax = lapsed;
			}

		if(wait){//update stats
			pthread_mutex_lock(&print);
			pthread_mutex_unlock(&print);

			if(lapsed > queuemax){
				queuemax = lapsed;
			}
			if(lapsed < queuemin){
				queuemin = lapsed;
			}
			numwaits++;
			queuetime+=lapsed;
		}
		
	}
	//if waited at all
	if(numwaits>0){
		avgqueue = queuetime/numwaits;
	}//didnt wait set all to zero because they didnt occur
	else if (numwaits==0){
		queuemin = 0;
		avgqueue = 0;
		queuemax = 0;
	}

	pthread_mutex_lock(&print);//grab print lock
	//print thread termination data
	printf("Thread %d terminated! \n",*(int *)thread);
	printf("Gender: %s, # of loops: %d \n",gender,loopcount);
	printf("Queue times: Minimum: %d\xC2\xB5s , Average: %d\xC2\xB5s , Maximum: %d\xC2\xB5s  \n",queuemin,avgqueue,queuemax);
	printf("-----------------------------\n");
	
	pthread_mutex_unlock(&print);//exit print lock

	pthread_exit(NULL);//end thread

}

//main runs the whole thing
int main(int argc, char **argv){
	users=atoi(argv[1]);//assign user input to globals
	arrival = atof(argv[2]);
	meantime = atof(argv[3]);
	loop = atoi(argv[4]);
	//start
	printf("Starting simulation with %d user(s):\n", users);
	//intialize the bathroom
	Initialize(&abathroom);
	//create array for threads and for thread numbers
	pthread_t thread[users];
	int threadnum[users];
	for(int i =0; i < users; i++){//loop through and spawn threads passing in thread number to track
		threadnum[i] = i;
		pthread_create(&thread[i],NULL,Individual,&threadnum[i]);
	}
	for(int i =0; i<users;i++){//as threads die join them back
		pthread_join(thread[i],NULL);
	}
	printf("\n");
	Finalize(&abathroom);//finalize and end the program
}
