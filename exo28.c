// Exercice 28 - sort-threads : sleepsort avec des threads POSIX
// Chaque thread dort proportionnellement a sa valeur puis ecrit
// directement dans le tableau resultat
// ldd: -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define TAB_SIZE 10

// Tableau partage pour le resultat
unsigned int resultat[TAB_SIZE];
int position = 0; // prochaine position a ecrire

// Mutex pour proteger l'acces au tableau
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Fonction executee par chaque thread
void *tri_thread(void *arg)
{
    unsigned int valeur = *(unsigned int *)arg;

    // Dormir proportionnellement a la valeur
    // On utilise usleep pour aller plus vite (millisecondes)
    usleep(valeur * 100000); // 100ms par unite

    // Section critique : ecrire dans le tableau
    pthread_mutex_lock(&mutex);
    resultat[position] = valeur;
    position++;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(void)
{
    unsigned int tab[TAB_SIZE] = {5, 3, 8, 1, 9, 2, 7, 4, 10, 6};
    pthread_t threads[TAB_SIZE];

    // Afficher le tableau avant
    printf("Avant : ");
    for (int i = 0; i < TAB_SIZE; i++)
        printf("%d ", tab[i]);
    printf("\n");

    // Creer un thread pour chaque valeur
    for (int i = 0; i < TAB_SIZE; i++) {
        pthread_create(&threads[i], NULL, tri_thread, &tab[i]);
    }

    // Attendre tous les threads
    for (int i = 0; i < TAB_SIZE; i++) {
        pthread_join(threads[i], NULL);
    }

    // Afficher le resultat
    printf("Apres : ");
    for (int i = 0; i < TAB_SIZE; i++)
        printf("%d ", resultat[i]);
    printf("\n");

    return 0;
}
