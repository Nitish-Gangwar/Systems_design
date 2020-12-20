#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#define MAX 100000
pthread_t t[MAX];
pthread_mutex_t lock;
/* 
 * This is the counter value which is to
 * be incremented by all the threads created 
 * by your program
*/
int counter = 0;
void *hello(void *arg)
{
	pthread_mutex_lock(&lock);
	counter++;
	pthread_mutex_unlock(&lock);
	return NULL;
}


int main(int argc, char const *argv[])
{
	/* Write you code to create n threads here*/
	/* Each thread must increment the counter once and exit*/

	int i=0;
	int number_of_threads=0;
	while((int)argv[1][i]!=0)
	{
		number_of_threads=number_of_threads*10+(argv[1][i]-'0');
		i++;
	}
	i=0;
	pthread_mutex_init(&lock,NULL);
	while(i<number_of_threads)
	{
		pthread_create(&t[i],NULL,hello,NULL);
		i++;
	}
	i=0;
	while(i<number_of_threads)
	{	
		pthread_join(t[i],NULL);
		i++;
	}
	/* Make sure you reap all threads created by your program
	 before printing the counter*/
	printf("%d\n", counter);
	sleep(10000);
	//sleep(1);
	return 0;
}
