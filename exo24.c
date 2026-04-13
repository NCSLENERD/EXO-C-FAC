// Exercice 24 - shm : memoire partagee sans synchronisation
// 10 enfants ajoutent des nombres premiers dans un tableau partage
// On observe les problemes d'acces concurrents
// NOTE : cet exo est concu pour Linux (CoW), les IPC System V
// marchent aussi sur macOS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

// Verifie si un nombre est premier
int est_premier(int n)
{
    if (n < 2) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

// Verifie si une valeur est deja dans le tableau
int deja_present(int *tab, int taille, int val)
{
    for (int i = 0; i < taille; i++) {
        if (tab[i] == val)
            return 1;
    }
    return 0;
}

int main(void)
{
    // Creer un segment de memoire partagee
    // Taille = 1 int (compteur) + 100 int (tableau)
    int taille = sizeof(int) + 100 * sizeof(int);
    int shmid = shmget(IPC_PRIVATE, taille, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return EXIT_FAILURE;
    }

    // Attacher le segment
    void *shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat");
        return EXIT_FAILURE;
    }

    // Le compteur est le premier int, le tableau commence apres
    int *compteur = (int *)shm_ptr;
    int *tableau = (int *)shm_ptr + 1;

    // Initialisation a 0
    *compteur = 0;
    for (int i = 0; i < 100; i++)
        tableau[i] = 0;

    // Creer 10 enfants
    for (int i = 0; i < 10; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Enfant : attacher au segment
            srand(getpid());

            // Generer 10 entiers aleatoires entre 2 et 100
            for (int j = 0; j < 10; j++) {
                int val = (rand() % 99) + 2;

                // Si c'est premier et pas deja dans le tableau
                if (est_premier(val) && !deja_present(tableau, *compteur, val)) {
                    // Ajouter a la fin du tableau
                    tableau[*compteur] = val;
                    (*compteur)++;
                }
            }

            // Trier le tableau en ordre croissant (tri a bulles simple)
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

            // Detacher et quitter
            shmdt(shm_ptr);
            exit(0);
        }
    }

    // Attendre tous les enfants
    for (int i = 0; i < 10; i++)
        wait(NULL);

    // Afficher le contenu du tableau
    printf("Compteur = %d\n", *compteur);
    printf("Tableau : ");
    for (int i = 0; i < *compteur; i++)
        printf("%d ", tableau[i]);
    printf("\n");

    // Verifier l'ordre croissant
    int ok = 1;
    for (int i = 1; i < *compteur; i++) {
        if (tableau[i] <= tableau[i - 1]) {
            ok = 0;
            break;
        }
    }
    printf("Tableau bien trie ? %s\n", ok ? "oui" : "NON (probleme de concurrence !)");

    // Verifier les doublons
    int doublons = 0;
    for (int i = 1; i < *compteur; i++) {
        if (tableau[i] == tableau[i - 1])
            doublons++;
    }
    if (doublons > 0)
        printf("Doublons detectes : %d (probleme de concurrence !)\n", doublons);

    // Detacher et supprimer le segment
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
