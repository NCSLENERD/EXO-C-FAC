#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define NB_THREADS 10
#define MAX_NONCES 5
#define NB_ZEROS   6

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int first;
    int step;
    int zero;
} thread_args_t;

char nonces[MAX_NONCES][32];
volatile int count = 0;

int zeros(char *s, int n)
{
    int i = 0;
    while (i < n && s[i] == '0')
        i++;
    return (i == n);
}

void *bruteforce(void *arg)
{
	thread_args_t *a = (thread_args_t *)arg;
	int first = a->first;
    int zero  = a->zero;
	int step = a->step;

    unsigned char hash[50];
    char hex[33];
    char buf[32];
    
    while(count < 5)
    {
        int size = sprintf(buf, "%d", first);
        MD5((unsigned char*)buf, size, hash);

        for (int i = 0; i < 16; i++)
            sprintf(hex + i*2, "%02x", hash[i]);
            
        if(zeros(hex, a->zero))
        {
            pthread_mutex_lock(&mutex);
        	strcpy(nonces[count], buf);
        	count++; 
			printf("trouvé ! count = %d\n", count);
			pthread_mutex_unlock(&mutex);
        }
        first += step;
    }
	return NULL;
}
int main(void)
{
    pthread_t tids[10];
	thread_args_t args[10];
    
    for (int i = 0; i < 10; i++) {
		args[i].first = i + 1;
    	args[i].zero = 6;
		args[i].step = 10;
        if (pthread_create(&tids[i], NULL, bruteforce, &args[i]) != 0) {
            perror("thread create failed");
            exit(1);
        }
    }
    for (int i = 0; i < 10; i++)
		pthread_join(tids[i], NULL);
	for (int i = 0; i < 5; i++)
    	printf("Nonce %d : %s\n", i + 1, nonces[i]);
    return 0;
}	