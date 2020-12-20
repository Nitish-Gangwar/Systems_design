#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "barrier.h"

void barrier_init(struct barrier_t * barrier, int nthreads)
{
	pthread_mutex_init(&(barrier->lock), NULL);
  	pthread_cond_init(&(barrier->cond), NULL);
  	barrier->count = 0;
  	barrier->total_count_of_threads=nthreads;
	return;
}

void barrier_wait(struct barrier_t *barrier)
{
	pthread_mutex_lock(&(barrier->lock));
	//printf("barrier.count %d \n",barrier->count);
	(barrier->count)++;
	if((barrier->count) == (barrier->total_count_of_threads)) 
	{
	    barrier->count = 0;
	    pthread_cond_broadcast(&(barrier->cond));
	}
	else
	    while(pthread_cond_wait(&(barrier->cond), &(barrier->lock)) != 0);
	pthread_mutex_unlock(&(barrier->lock));
	return;
}
