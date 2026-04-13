// Exercice 29 - qsort multithreads
// Quicksort parallele avec une file de taches partagee
// On utilise un mutex + condition au lieu de POSIX mq
// (mq_* n'est pas disponible sur macOS)
// ldd: -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

#define NB_THREADS 4
#define TAB_SIZE 100

// Le tableau a trier (partage entre tous les threads)
int tableau[TAB_SIZE];

// Structure pour une tache (bornes d'un sous-tableau)
typedef struct {
    int gauche;
    int droite;
} tache;

// File de taches (tableau circulaire simple)
#define MAX_TACHES 1024
tache file_taches[MAX_TACHES];
int file_debut = 0;
int file_fin = 0;
int file_count = 0;

// Mutex et condition pour la file de taches
pthread_mutex_t mutex_file = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_tache = PTHREAD_COND_INITIALIZER;

// Compteur d'elements "termines" (sous-tableaux de 1 element)
int elements_tries = 0;
pthread_mutex_t mutex_compteur = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_fini = PTHREAD_COND_INITIALIZER;

// Flag d'arret
int arret = 0;

// Ajouter une tache dans la file
void ajouter_tache(int g, int d)
{
    pthread_mutex_lock(&mutex_file);
    file_taches[file_fin].gauche = g;
    file_taches[file_fin].droite = d;
    file_fin = (file_fin + 1) % MAX_TACHES;
    file_count++;
    // Reveiller un thread qui attend une tache
    pthread_cond_signal(&cond_tache);
    pthread_mutex_unlock(&mutex_file);
}

// Prendre une tache dans la file (bloquant)
// Renvoie 1 si on a une tache, 0 si arret
int prendre_tache(tache *t)
{
    pthread_mutex_lock(&mutex_file);

    while (file_count == 0 && !arret) {
        // Attendre qu'une tache soit disponible
        pthread_cond_wait(&cond_tache, &mutex_file);
    }

    if (arret) {
        pthread_mutex_unlock(&mutex_file);
        return 0;
    }

    *t = file_taches[file_debut];
    file_debut = (file_debut + 1) % MAX_TACHES;
    file_count--;

    pthread_mutex_unlock(&mutex_file);
    return 1;
}

// Partitionne le sous-tableau et renvoie l'indice du pivot
int partition(int g, int d)
{
    int pivot = tableau[d]; // on prend le dernier element comme pivot
    int i = g - 1;

    for (int j = g; j < d; j++) {
        if (tableau[j] <= pivot) {
            i++;
            // Echanger tableau[i] et tableau[j]
            int tmp = tableau[i];
            tableau[i] = tableau[j];
            tableau[j] = tmp;
        }
    }

    // Mettre le pivot a sa place
    int tmp = tableau[i + 1];
    tableau[i + 1] = tableau[d];
    tableau[d] = tmp;

    return i + 1;
}

// Fonction executee par chaque thread
void *thread_tri(void *arg)
{
    tache t;

    while (prendre_tache(&t)) {
        int g = t.gauche;
        int d = t.droite;

        if (g < d) {
            // Partitionner
            int pivot = partition(g, d);

            // Sous-tableau gauche
            if (pivot - 1 > g) {
                ajouter_tache(g, pivot - 1);
            } else if (pivot - 1 == g) {
                // Sous-tableau de 1 element = trie
                pthread_mutex_lock(&mutex_compteur);
                elements_tries++;
                pthread_mutex_unlock(&mutex_compteur);
            }
            // Le pivot est aussi un element trie
            pthread_mutex_lock(&mutex_compteur);
            elements_tries++;
            if (elements_tries >= TAB_SIZE) {
                arret = 1;
                pthread_cond_broadcast(&cond_tache);
                pthread_cond_signal(&cond_fini);
            }
            pthread_mutex_unlock(&mutex_compteur);

            // Sous-tableau droit
            if (pivot + 1 < d) {
                ajouter_tache(pivot + 1, d);
            } else if (pivot + 1 == d) {
                pthread_mutex_lock(&mutex_compteur);
                elements_tries++;
                if (elements_tries >= TAB_SIZE) {
                    arret = 1;
                    pthread_cond_broadcast(&cond_tache);
                    pthread_cond_signal(&cond_fini);
                }
                pthread_mutex_unlock(&mutex_compteur);
            }
        } else {
            // Sous-tableau de 1 ou 0 elements
            pthread_mutex_lock(&mutex_compteur);
            elements_tries++;
            if (elements_tries >= TAB_SIZE) {
                arret = 1;
                pthread_cond_broadcast(&cond_tache);
                pthread_cond_signal(&cond_fini);
            }
            pthread_mutex_unlock(&mutex_compteur);
        }
    }

    return NULL;
}

int main(void)
{
    // Remplir le tableau avec des valeurs aleatoires
    srand(42);
    for (int i = 0; i < TAB_SIZE; i++)
        tableau[i] = rand() % 1000;

    // Afficher avant
    printf("Avant : ");
    for (int i = 0; i < TAB_SIZE; i++)
        printf("%d ", tableau[i]);
    printf("\n\n");

    // Ajouter la premiere tache (tout le tableau)
    ajouter_tache(0, TAB_SIZE - 1);

    // Creer les threads
    pthread_t threads[NB_THREADS];
    for (int i = 0; i < NB_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_tri, NULL);

    // Attendre que le tri soit termine
    pthread_mutex_lock(&mutex_compteur);
    while (!arret)
        pthread_cond_wait(&cond_fini, &mutex_compteur);
    pthread_mutex_unlock(&mutex_compteur);

    // Arreter les threads
    pthread_mutex_lock(&mutex_file);
    arret = 1;
    pthread_cond_broadcast(&cond_tache);
    pthread_mutex_unlock(&mutex_file);

    for (int i = 0; i < NB_THREADS; i++)
        pthread_join(threads[i], NULL);

    // Afficher apres
    printf("Apres : ");
    for (int i = 0; i < TAB_SIZE; i++)
        printf("%d ", tableau[i]);
    printf("\n");

    // Verifier que c'est trie
    int ok = 1;
    for (int i = 1; i < TAB_SIZE; i++) {
        if (tableau[i] < tableau[i - 1]) { ok = 0; break; }
    }
    printf("\nTableau trie ? %s\n", ok ? "oui" : "NON");

    return 0;
}
