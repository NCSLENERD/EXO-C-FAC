#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#include "quicksort.c"

#define LEN 8
#define NB_THREADS 4

Tab tableau;
sem_t sem;
mqd_t mq;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond   = PTHREAD_COND_INITIALIZER;
int done = 0;

typedef struct {
    uint first;
    uint last;
} task_t;

static void handle_subrange(task_t t)
{
    if (t.first > t.last)
        return;
    if (t.first == t.last) {
        sem_wait(&sem);
        pthread_mutex_lock(&mutex);
        int val;
        sem_getvalue(&sem, &val);
        if (val == 0) {
            done = 1;
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
        return;
    }
    mq_send(mq, (char*)&t, sizeof(task_t), 0);
}

void *thread_sort(void *arg)
{
    task_t task;

    while(1)
    {
        mq_receive(mq, (char*)&task, sizeof(task_t), NULL);
        Tab sub = slice(tableau, task.first, task.last);
        uint p = partition(sub);

        task_t gauche = {task.first, task.first + p};
        task_t droit  = {task.first + p + 1, task.last};
        handle_subrange(gauche);
        handle_subrange(droit);
    }
    return NULL;
}

int main(void)
{
	pthread_t tids[4];
    tableau.len = LEN;
    tableau.val = malloc(LEN * sizeof(int));
    for (int i = 0; i < LEN; i++)
        tableau.val[i] = rand() % 100;

	for(int i = 0; i < tableau.len; i++)
	{
		printf("%d, ",tableau.val[i]);
	}
	printf("\n");

    sem_init(&sem, 0, LEN);

    struct mq_attr attr;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof(task_t);
    mq_unlink("/qsort");
    mq = mq_open("/qsort", O_CREAT | O_RDWR, 0644, &attr);
    if (mq == (mqd_t)-1) { perror("mq_open"); exit(1); }

    task_t first_task = {0, LEN - 1};
    mq_send(mq, (char*)&first_task, sizeof(task_t), 0);
	for(int i = 0; i < NB_THREADS ; i++)
	{
		if(pthread_create(&tids[i], NULL, thread_sort, &tableau) != 0)
		{
			perror("thread create failed");
            exit(1);
        }
	}
	pthread_mutex_lock(&mutex);
	while (!done)
		pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
	for (int i = 0; i < NB_THREADS; i++)
   		pthread_cancel(tids[i]);
	for(int i = 0 ; i < NB_THREADS ; i++)
		pthread_join(tids[i], NULL);

	mq_close(mq);
	mq_unlink("/qsort");
	sem_destroy(&sem);

	for(int i = 0; i < tableau.len; i++)
	{
		printf("%d, ",tableau.val[i]);
	}
	printf("\n");

	free(tableau.val);
}