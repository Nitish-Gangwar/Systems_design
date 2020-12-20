#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

/* 
 * This is the data array which is to
 * be incremented by all the threads created 
 * by your program as specified in the problem state
*/
pthread_mutex_t lock[10];
int data[10];
pthread_t t[10];

void *hello(void *arg)
{
	int *c=(int*)arg;
	int i;
	pthread_mutex_lock(&lock[c[0]]);
	for(i=0;i<1000;i++)
	{
		data[c[0]]++;
	}
	pthread_mutex_unlock(&lock[c[0]]);
	return NULL;
}



int main(int argc, char const *argv[])
{
	/* Write you code to create 10 threads here*/
	/* Increment the data array as specified in the problem statement*/
	int i=0;
	int number[10];
	for(i=0;i<10;i++)
	number[i]=i;
	i=0;
	while(i<10)
	{
		pthread_mutex_init(&lock[i],NULL);
		i++;
	}
	i=0;
	while(i<10)
	{
		pthread_create(&t[i],NULL,hello,(int*)&number[i]);
		i++;
	}
	i=0;
	int j;
	while(i<1000)
	{
		for(j=0;j<10;j++)
		{
			pthread_mutex_lock(&lock[j]);
			data[j]++;
			pthread_mutex_unlock(&lock[j]);
		}
		i++;
	}
	i=0;
	while(i<10)
	{
		pthread_join(t[i],NULL);
		i++;
	}
	/* Make sure you reap all threads created by your program
	 before printing the counter*/
	for (int i = 0; i < 10; ++i)
	{
		printf("%d\n", data[i]);
	}
	//sleep(5);
	sleep(10000);	
	return 0;
}
