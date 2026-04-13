// Exercice 26 - shm-sem-posix : memoire partagee avec semaphores POSIX
// Meme chose que l'exo 25 mais avec des semaphores POSIX nommes
// au lieu des semaphores IPC System V

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

#define SEM_NAME "/exo26_sem"

// Verifie si un nombre est premier
int est_premier(int n)
{
    if (n < 2) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

// Verifie si une valeur est deja dans le tableau
int deja_present(int *tab, int taille, int val)
{
    for (int i = 0; i < taille; i++) {
        if (tab[i] == val) return 1;
    }
    return 0;
}

int main(void)
{
    // Creer le segment de memoire partagee
    int taille = sizeof(int) + 100 * sizeof(int);
    int shmid = shmget(IPC_PRIVATE, taille, IPC_CREAT | 0666);
    if (shmid == -1) { perror("shmget"); return EXIT_FAILURE; }

    // Creer le semaphore POSIX nomme (initialise a 1)
    sem_unlink(SEM_NAME); // supprimer au cas ou il existe deja
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) { perror("sem_open"); return EXIT_FAILURE; }

    // Attacher le segment
    void *shm_ptr = shmat(shmid, NULL, 0);
    int *compteur = (int *)shm_ptr;
    int *tableau = (int *)shm_ptr + 1;

    // Initialiser a 0
    *compteur = 0;
    for (int i = 0; i < 100; i++)
        tableau[i] = 0;

    // Creer 10 enfants
    for (int i = 0; i < 10; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            srand(getpid());

            for (int j = 0; j < 10; j++) {
                int val = (rand() % 99) + 2;

                if (est_premier(val)) {
                    // Prendre le semaphore POSIX (P / wait)
                    sem_wait(sem);

                    if (!deja_present(tableau, *compteur, val)) {
                        tableau[*compteur] = val;
                        (*compteur)++;
                    }

                    // Trier le tableau (tri a bulles)
                    int n = *compteur;
                    for (int a = 0; a < n - 1; a++) {
                        for (int b = 0; b < n - 1 - a; b++) {
                            if (tableau[b] > tableau[b + 1]) {
                                int tmp = tableau[b];
                                tableau[b] = tableau[b + 1];
                                tableau[b + 1] = tmp;
                            }
                        }
                    }

                    // Liberer le semaphore (V / post)
                    sem_post(sem);
                }
            }

            shmdt(shm_ptr);
            exit(0);
        }
    }

    // Attendre tous les enfants
    for (int i = 0; i < 10; i++)
        wait(NULL);

    // Afficher le resultat
    printf("Compteur = %d\n", *compteur);
    printf("Tableau : ");
    for (int i = 0; i < *compteur; i++)
        printf("%d ", tableau[i]);
    printf("\n");

    // Verifier l'ordre
    int ok = 1;
    for (int i = 1; i < *compteur; i++) {
        if (tableau[i] <= tableau[i - 1]) { ok = 0; break; }
    }
    printf("Tableau bien trie ? %s\n", ok ? "oui" : "NON");

    // Nettoyer
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);
    sem_close(sem);
    sem_unlink(SEM_NAME);

    return 0;
}
