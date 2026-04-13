// Exercice 27 - md5-threads : refaire l'exo 20 avec des threads POSIX
// et un tableau partage pour stocker les nonces trouves
// ldd: -lcrypto -lpthread

#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Tableau partage pour stocker les 5 premiers nonces
int nonces[5];
int nb_trouves = 0;

// Mutex pour proteger l'acces au tableau
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Flag pour dire aux threads d'arreter
// volatile = force le compilateur a relire la variable en memoire
// a chaque fois, sinon il pourrait la garder dans un registre
// et un thread ne verrait jamais que fini a change
volatile int fini = 0;

int zeros(char *s, int n)
{
    int i = 0;
    while (i < n && s[i] == '0')
        i++;
    return (i == n);
}

// Structure pour passer les parametres au thread (comme les params de bruteforce dans exo20)
typedef struct {
    int first;
    int step;
    int zero;
} thread_args;

// Version thread de bruteforce (adaptee de exo20)
void *bruteforce(void *arg)
{
    thread_args *args = (thread_args *)arg;
    unsigned char hash[50];
    char hex[33];
    char buf[32];

    int current = args->first;

    while (!fini)
    {
        int size = sprintf(buf, "%d", current);
        MD5((unsigned char*)buf, size, hash);

        for (int i = 0; i < 16; i++)
            sprintf(hex + i*2, "%02x", hash[i]);

        if(zeros(hex, args->zero))
        {
            // Au lieu d'ecrire dans un fichier comme exo20,
            // on ecrit dans le tableau partage (protege par mutex)
            pthread_mutex_lock(&mutex);
            if (nb_trouves < 5) {
                nonces[nb_trouves] = current;
                printf("Nonce %d trouve par thread %d : %s\n",
                       nb_trouves + 1, args->first, buf);
                nb_trouves++;

                if (nb_trouves >= 5)
                    fini = 1;
            }
            pthread_mutex_unlock(&mutex);
        }
        current = current + args->step;
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[10];
    thread_args args[10];

    // Creer les 10 threads (comme les 10 enfants dans exo20)
    for (int i = 0; i < 10; i++) {
        args[i].first = i + 1;
        args[i].step = 10;
        args[i].zero = 6;

        pthread_create(&threads[i], NULL, bruteforce, &args[i]);
    }

    // Attendre tous les threads (comme wait dans exo20)
    for (int i = 0; i < 10; i++)
        pthread_join(threads[i], NULL);

    // Afficher les 5 nonces trouves
    printf("\nNonces trouves :\n");
    for (int i = 0; i < nb_trouves; i++)
        printf("  %d\n", nonces[i]);

    return 0;
}
