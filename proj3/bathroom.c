#include "bathroom.h"

//import all extern variables from header
pthread_mutex_t lock;
pthread_cond_t change;
pthread_mutex_t print;

double meantime;
double arrival;
int loop;
int users;
int * barray;
int * qarray;

bathroom abathroom;
int flag;

//create timers for important stats
struct timeval starttime;
struct timeval endtime;
struct timeval vacstart;
struct timeval vacend;


//intialize simply initalizes all the values of a bathroom along with starting timers and setting up locks
void Initialize(bathroom * a){
	srand(time(NULL));
	srand48(time(NULL));
	a->users = 0;
	a->uses = 0;
	a->ques=0;
	a->quers=0;
	a->vacanttime = 0;
	a->occtime = 0;
	a->avgqlen = 0;
	a->avgnumusers = 0;
	pthread_mutex_init(&lock,NULL);
	pthread_cond_init(&change,NULL);
	pthread_mutex_init(&print,NULL);
	gettimeofday(&starttime,NULL);
	flag = VACANT;
	gettimeofday(&vacstart, NULL);
	barray = malloc(sizeof(int) * users);
	qarray = malloc(sizeof(int) * users);
	for(int i =0; i < users; i++){
		barray[i] = -1;
		qarray[i] = -1;
	}

}

//finalize takes a bathroom and finishes the stats calculations and prints them
void Finalize(bathroom * a){
	a->avgqlen = a->ques/users;
	a->avgnumusers = a->uses/users;
	gettimeofday(&endtime,NULL);
	double runtime = (endtime.tv_usec-starttime.tv_usec) + 1000000* (endtime.tv_sec - starttime.tv_sec);//take the difference in microseconds and then multiply it by the difference in microseconds from the seconds field
	runtime /= 1000000;//convert to seconds
	a->vacanttime /=1000000;
	a->occtime = runtime - a->vacanttime;
	int bper = 0;//calculate total frequencies of bathroom and queue
	int qper = 0;
	for(int i =0; i < users; i++){//loop through and add to the running total
		if(barray[i]!=-1){
			bper+=barray[i];
		}
		if(qarray[i]!=-1){
			qper+=qarray[i];
		}
	}
	for(int i =0; i < users; i++){
		if(barray[i]!=-1){
			a->avgnumusers+= ((barray[i] * 1.00)/bper) * i;//calculate Expected value using discrete probability distribution
		}
		if(qarray[i]!=-1){
			a->avgqlen+= ((qarray[i] * 1.00)/qper) * i;
		}
	}
	printf("The simulation ran with ./bathroomsim %d %.1f %.1f %d \n",users,arrival,meantime,loop);
	printf("The simulation has finished with runtime of: %f seconds \n", runtime);
	printf("The number of uses was: %d \n", a->uses);
	printf("The amount of time the bathroom was vacant was: %f seconds \n",a->vacanttime);
	printf("The amount of time the bathroom was occupied was: %f seconds \n",a->occtime);
	printf("The average amount of people in the queue was: %.3f \n",a->avgqlen);
	printf("The average number of users in the bathroom at a time was: %.3f \n",a->avgnumusers);
	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&print);
	pthread_cond_destroy(&change);
	free(barray);
	free(qarray);


}

//Enter takes a gender and a wait pointer and will change wait 1 if the user had to wait to enter or 0 if not
void Enter(gender g, int * wait){
	*wait = 1;
	pthread_mutex_lock(&lock);//always grab lock
	if(flag==VACANT){//vacant case, user simply enters
		gettimeofday(&vacend, NULL);//end vacant time
		abathroom.vacanttime+=vacend.tv_usec - vacstart.tv_usec;
		flag = g;//set the flag
		abathroom.users++;//increment data
		abathroom.uses++;
		barray[abathroom.users]++;
		*wait = 0;//we didnt wait
	}
	else if(flag==g){//case of same gender, mostly same as above
		abathroom.uses++;
		abathroom.users++;
		barray[abathroom.users]++;
		*wait = 0;
	}
	if(*wait){//if were waiting meaning the bathroom is in use
		abathroom.quers++;//increment queue data
		abathroom.ques++;
		qarray[abathroom.quers]++;
		pthread_cond_wait(&change,&lock);//wait on flag to change using change variable
		flag = g;//change flag now that the bathroom is open
		abathroom.users++;//increment bathroom data and remove users from the queue
		barray[users]++;
		abathroom.uses++;
		abathroom.quers--;
	}
	pthread_mutex_unlock(&lock);//drop the lock
}

//leave takes the usetime that the user was using the bathroom for and has them leave the bathroom
void Leave(){
 	pthread_mutex_lock(&lock);//grab lock
 	abathroom.users--;//decrement users
 	if(abathroom.users<=0){//if bathroom is empty now signal to the queue
 		flag = VACANT;//change flag
		gettimeofday(&vacstart,NULL);//could be vacant if no one is in queue
 		pthread_cond_broadcast(&change);//signal
 	}
 	pthread_mutex_unlock(&lock);//unlock
}

//generateStandardRand takes a mean value and generates a random variable belonging to a normal distribution
//with mean= mean passed in and standard deviation of mean/2
//uses the Box-Muller transformation
double generateStandardRand(double mean){
	double a = drand48();//generate 2 random numbers
	double b = drand48();
	double random = sqrt(-2 * log(a)) * cos(2 * M_PI * b);
	double stddev = mean/2;

	double standrand = (stddev * random) + mean;

	if (standrand <=0){//minimum value is 1
		standrand = 1;
	}
	else if(standrand < 1){//if the value is unrealistically small especially for bathroom use time add .5
		standrand+=.5;
	}
	return standrand;
}

