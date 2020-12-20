#include <pthread.h>

struct barrier_t
{
	pthread_mutex_t lock;
        pthread_cond_t cond;
        int count;
        int total_count_of_threads;
};

void barrier_init(struct barrier_t *b, int i);
void barrier_wait(struct barrier_t *b);
