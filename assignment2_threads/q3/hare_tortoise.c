#include "cs_thread.h"

struct Repositioning {
	char player; 		// T for turtle and H for hare
	int time; 		// At what time god interrupt's
	int distance;		// How much does god move any of the player. 
							// distance can be negetive or positive.
							// using this distance if any of players position is less than zero then bring him to start line.
							// If more than finish_distance then make him win.
							// If time is after completion of game than you can ignore it as we will already have winner.
};
pthread_cond_t cond1,cond2,cond3,cond5,cond_last;
pthread_mutex_t lock;
pthread_t randomizer,report,turtle,hare;
char winner='N';
int time_counter=0;
int turtle_distance=0;
int hare_distance=0;
int reposition_current_count=0;
pthread_t turtle,hare,randomizer,report;
struct race {
	
	//	Don't change these variables.
	//	speeds are unit distance per unit time.
	int printing_delay;
	int tortoise_speed;					// speed of Turtle
	int hare_speed;						// speed of hare
	int hare_sleep_time; 				// how much time does hare sleep (in case he decides to sleep)
	int hare_turtle_distance_for_sleep; // minimum lead which hare has on turtle after which hare decides to move
										// Any lead greater than this distance and hare will ignore turtle and go to sleep
	int finish_distance;				// Distance between start and finish line
	struct Repositioning* reposition;	// pointer to array containing Randomizer's decision
	int repositioning_count;			// number of elements in array of repositioning structure
	
	//	Add your custom variables here.

	
	
};


void* Turtle(void *race);
void* Hare(void *race);
void* Randomizer(void *race);
void* Report(void *race);

char init(struct race *race)
{

	pthread_cond_init(&cond1,NULL);
	pthread_cond_init(&cond2,NULL);
	pthread_cond_init(&cond3,NULL);
	pthread_cond_init(&cond5,NULL);
	pthread_cond_init(&cond_last,NULL);
	pthread_mutex_init(&lock,NULL);
	reposition_current_count=0;
	winner='N';
	time_counter=0;
	turtle_distance=0;
	hare_distance=0;
	
	pthread_create(&randomizer,NULL,Randomizer,race);//firstly get randomizer into action
	
	pthread_create(&turtle,NULL,Turtle,race); //then arbitrarily making the four threads and synchronization is inside the functions
	
	pthread_create(&hare,NULL,Hare,race);
	
	pthread_create(&report,NULL,Report,race);   // all the four threads have been created
	
	
	pthread_mutex_lock(&lock);
	while(winner=='N')//synchronize all four threads here but how??
	{
		time_counter++;
		pthread_cond_signal(&cond5);//signalling the randomizer thread and blocking the rest three threads
		pthread_cond_wait(&cond_last,&lock);//blocking this while loop untill no one is winner
	}
	pthread_cond_broadcast(&cond1);
	sleep(1);
	pthread_cond_broadcast(&cond2);
	sleep(1);
	pthread_cond_broadcast(&cond3);
	sleep(1);
	pthread_cond_broadcast(&cond5);
	sleep(1);
	pthread_cond_broadcast(&cond_last);
	pthread_mutex_unlock(&lock);
	
	sleep(1);
	
	pthread_join(randomizer,NULL);
	pthread_join(report,NULL);	//here reaping the threads
	pthread_join(turtle,NULL);
	pthread_join(hare,NULL);
	
	
	pthread_cond_destroy(&cond1);
	pthread_cond_destroy(&cond2);
	pthread_cond_destroy(&cond3);
	pthread_cond_destroy(&cond5);
	pthread_cond_destroy(&cond_last);
	pthread_mutex_destroy(&lock);
	return winner; 	//returing the winner
}

void* Turtle(void *arg)
{
	struct race *racee=(struct race *)arg;
	pthread_mutex_lock(&lock);
	while(winner=='N')
	{
		pthread_cond_wait(&cond1,&lock);	// waiting on cond variable cond1 will get signal from randomizer
		if(winner!='N')
		{
			break;
		}
		if(turtle_distance >= racee->finish_distance) //if turtle has already crossed the finish line declaring him has a winner
		{
			winner='T';
			pthread_cond_signal(&cond_last);//this will signal the cond_last variable of init function
			pthread_mutex_unlock(&lock);
			pthread_exit(0);
		}
		turtle_distance=turtle_distance + racee->tortoise_speed;
		pthread_cond_signal(&cond2);
	}
	pthread_cond_signal(&cond_last);
	pthread_mutex_unlock(&lock);
  	pthread_exit(0);
}

void* Hare(void *arg)		//here program is not able to read using structure
{
	struct race *racee=(struct race *)arg;
	pthread_mutex_lock(&lock);
	while(winner=='N')
	{
		pthread_cond_wait(&cond2,&lock);//will get signal from turtle thread
		if(winner!='N')
		{
			break;
		}
		if(hare_distance >= racee->finish_distance)
		{
			winner='H';
			pthread_cond_signal(&cond_last);//this will signal the cond_last variable of init function 
			pthread_mutex_unlock(&lock);
			pthread_exit(0);
		}
		if(hare_distance-turtle_distance < racee->hare_turtle_distance_for_sleep) //hare will panic and will move
		{
			hare_distance=hare_distance+racee->hare_speed;
		}
		int sleep_time=0;
		while(sleep_time!=racee->hare_sleep_time)
		{
			sleep_time++;
			pthread_cond_signal(&cond3);
			pthread_cond_wait(&cond2,&lock);
			if(winner!='N')
			break;
		}
		pthread_cond_signal(&cond3);
		
	}
	pthread_cond_signal(&cond_last);
	pthread_mutex_unlock(&lock);
	pthread_exit(0);
}


void* Randomizer(void *arg)
{
	struct race *racee=(struct race *)arg;
	pthread_mutex_lock(&lock);
	while(winner=='N')
	{			//first time execution because of signal obtained from init function
		pthread_cond_wait(&cond5,&lock); // on second time waiting for reporter to signal on conditional variable cond5
		if(winner!='N')
		{
			break;
		}
		if((reposition_current_count < racee->repositioning_count)&&(racee->reposition[reposition_current_count].time==time_counter))
		{
			
			if(racee->reposition[reposition_current_count].player=='H')
			{
				hare_distance=hare_distance+racee->reposition[reposition_current_count].distance;
				reposition_current_count++;
				if(hare_distance<=0)
				{
					hare_distance=0;
				}
				else if(hare_distance >= racee->finish_distance)
				{
					winner='H';
					pthread_cond_signal(&cond_last);
					pthread_mutex_unlock(&lock);
					pthread_exit(0);
				}
			}
			else
			{
				turtle_distance=turtle_distance+racee->reposition[reposition_current_count].distance;
				reposition_current_count++;
				if(turtle_distance<=0)
				{
					turtle_distance=0;
				}
				else if(turtle_distance >= racee->finish_distance)
				{
					winner='T';
					pthread_cond_signal(&cond_last);
					pthread_mutex_unlock(&lock);
					pthread_exit(0);
				}
			}
		}
		pthread_cond_signal(&cond1);//this signals turtle thread and this thread  will keep on going if no winner and will wait on cond5
	}
	pthread_cond_signal(&cond_last);
	pthread_mutex_unlock(&lock);
	pthread_exit(0);
}

void* Report(void *arg)
{
	struct race *racee=(struct race *)arg;
	pthread_mutex_lock(&lock);
	while(winner=='N')
	{
		pthread_cond_wait(&cond3,&lock);// will get signal from hare thread
		if(winner!='N')
		{
			//printf("breaking from reporter \n");
			break;
		}
		if(turtle_distance>=racee->finish_distance)
		{
			winner='T';
			//printf("---Hey winner decided inside reporter %c---\n",winner);
			pthread_cond_signal(&cond_last);
			pthread_mutex_unlock(&lock);
			pthread_exit(0);
		}
		else if(hare_distance>=racee->finish_distance)
		{
			winner='H';
			//printf("---Hey winner decided inside reporter %c---\n",winner);
			pthread_cond_signal(&cond_last);
			pthread_mutex_unlock(&lock);
			pthread_exit(0);
		}
		if(time_counter%racee->printing_delay==0)
		{
			printf("Reporter reporting here----hare position=%d---turtle position=%d\n",hare_distance,turtle_distance);
		}
		pthread_cond_signal(&cond_last);
	}
	pthread_cond_signal(&cond_last);
	pthread_mutex_unlock(&lock);
	pthread_exit(0);
}

