// Exercice 17 - sig : envoyer et recevoir des signaux
// 10 enfants tirent un nombre au hasard et le parent doit le deviner
// en envoyant des SIGTERM. Chaque enfant meurt apres num signaux SIGTERM.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// Variable globale pour compter les SIGTERM recus
int compteur = 0;
int num_cible = 0;

// Handler de signal pour les enfants
void handler_enfant(int sig)
{
    compteur++;
    if (compteur >= num_cible) {
        // On a recu assez de SIGTERM, on termine
        exit(0);
    }
    // Sinon on continue de dormir
}

int main(void)
{
    pid_t pids[10];

    // Creer 10 enfants
    for (int i = 0; i < 10; i++) {
        pids[i] = fork();

        if (pids[i] == 0) {
            // Enfant
            srand(getpid()); // initialiser le generateur aleatoire
            num_cible = (rand() % 10) + 1; // nombre entre 1 et 10
            compteur = 0;

            printf("Enfant %d : num = %d\n", getpid(), num_cible);

            // Installer le handler pour SIGTERM
            signal(SIGTERM, handler_enfant);

            // Dormir indefiniment (le handler nous reveillera)
            while (1)
                pause(); // attend un signal

            exit(0);
        }
    }

    // Laisser le temps aux enfants de s'initialiser
    sleep(1);

    // Le parent doit deviner le num de chaque enfant
    // Strategie : envoyer des SIGTERM un par un et attendre
    for (int i = 0; i < 10; i++) {
        int count = 0;

        while (1) {
            count++;
            kill(pids[i], SIGTERM);
            usleep(50000); // attendre 50ms pour laisser le temps au signal

            // Verifier si l'enfant est mort
            int status;
            pid_t ret = waitpid(pids[i], &status, WNOHANG);

            if (ret > 0) {
                // L'enfant est termine
                printf("Parent : enfant %d avait num = %d\n", pids[i], count);
                break;
            }
        }
    }

    printf("Tous les enfants sont termines\n");
    return 0;
}
