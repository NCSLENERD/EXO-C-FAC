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

typedef struct {
    uint first;
    uint last;
} task_t;


void *thread_sort(void *arg)
{
    task_t task;
	int val;
    
    while(1)
    {
		printf("thread : j'attends une tâche\n");
        mq_receive(mq, (char*)&task, sizeof(task_t), NULL);
		printf("tâche [%d, %d], taille=%d\n", task.first, task.last, task.last - task.first + 1);
        Tab sub = slice(tableau, task.first, task.last);
		printf("sub.len=%d\n", sub.len);
        uint p = partition(sub);
		printf("p=%d, gauche=[%d,%d] droit=[%d,%d]\n", 
      	 p,
       	task.first, task.first + p,
       	task.first + p + 1, task.last);
		printf("thread : partition p=%d\n", p);
		task_t gauche = {task.first, task.first + p};
		if(gauche.first != gauche.last)
			mq_send(mq, (char*)&gauche, sizeof(task_t), 0);
		else 
		{
			sem_wait(&sem);
			pthread_mutex_lock(&mutex);
			sem_getvalue(&sem, &val);
			if (val == 0)
    			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mutex);
		}
		task_t droit = {task.first + p + 1,task.last};
        if(droit.first != droit.last)
			mq_send(mq, (char*)&droit, sizeof(task_t), 0);
		else 
		{
    		sem_wait(&sem);
			pthread_mutex_lock(&mutex);
			sem_getvalue(&sem, &val);
			if (val == 0)
    			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mutex);
		}	
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
    attr.mq_maxmsg  = LEN;
    attr.mq_msgsize = sizeof(task_t);
    mq = mq_open("/qsort", O_CREAT | O_RDWR, 0644, &attr);
 
    task_t first_task = {0, LEN - 1};
    mq_send(mq, (char*)&first_task, sizeof(task_t), 0);
	for(int i = 0; i < 4 ; i++)
	{
		if(pthread_create(&tids[i], NULL, thread_sort, &tableau) != 0)
		{
			perror("thread create failed");
            exit(1);
        }
	}
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
	for (int i = 0; i < 4; i++)
   		pthread_cancel(tids[i]);
	for(int i = 0 ; i < 4 ; i++)
		pthread_join(tids[i], NULL);

	mq_close(mq);
	mq_unlink("/qsort");
	sem_destroy(&sem);
	free(tableau.val);

	for(int i = 0; i < tableau.len; i++)
	{
		printf("%d, ",tableau.val[i]);
	}
	printf("\n");
}