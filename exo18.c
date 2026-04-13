// Exercice 18 - randsig : envoyer et recevoir des signaux
// 10 enfants choisissent un comportement aleatoire parmi 4
// Le parent doit deviner le comportement de chaque enfant

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// Handlers pour les enfants

// Comportement 2 : ignore SIGUSR1 indefiniment
void handler_ignore(int sig)
{
    // On ne fait rien, on ignore le signal
}

// Comportement 3 : termine avec code 0 sur SIGUSR1
void handler_exit0(int sig)
{
    exit(0);
}

// Comportement 4 : termine avec code 1 sur SIGUSR1
void handler_exit1(int sig)
{
    exit(1);
}

int main(void)
{
    pid_t pids[10];

    for (int i = 0; i < 10; i++) {
        pids[i] = fork();

        if (pids[i] == 0) {
            // Enfant
            srand(getpid());
            int choix = (rand() % 4) + 1;

            printf("Enfant %d : comportement %d\n", getpid(), choix);

            switch (choix) {
                case 1:
                    // Dort indefiniment (pas de handler pour SIGUSR1)
                    // Le signal par defaut de SIGUSR1 est de tuer le processus
                    while (1) pause();
                    break;
                case 2:
                    // Ignore SIGUSR1 indefiniment
                    signal(SIGUSR1, handler_ignore);
                    while (1) pause();
                    break;
                case 3:
                    // Attend SIGUSR1 et termine avec code 0
                    signal(SIGUSR1, handler_exit0);
                    while (1) pause();
                    break;
                case 4:
                    // Attend SIGUSR1 et termine avec code 1
                    signal(SIGUSR1, handler_exit1);
                    while (1) pause();
                    break;
            }
            exit(0);
        }
    }

    // Laisser le temps aux enfants de s'initialiser
    sleep(1);

    // Le parent doit deviner le comportement de chaque enfant
    for (int i = 0; i < 10; i++) {
        // Envoyer SIGUSR1
        kill(pids[i], SIGUSR1);
        usleep(100000); // attendre 100ms

        // Verifier si l'enfant est mort
        int status;
        pid_t ret = waitpid(pids[i], &status, WNOHANG);

        if (ret > 0) {
            // L'enfant est termine
            if (WIFSIGNALED(status)) {
                // Tue par le signal = comportement 1 (dort)
                printf("Parent : enfant %d -> comportement 1 (dort, tue par signal)\n",
                       pids[i]);
            } else if (WIFEXITED(status)) {
                int code = WEXITSTATUS(status);
                if (code == 0)
                    printf("Parent : enfant %d -> comportement 3 (SIGUSR1 -> exit 0)\n",
                           pids[i]);
                else
                    printf("Parent : enfant %d -> comportement 4 (SIGUSR1 -> exit 1)\n",
                           pids[i]);
            }
        } else {
            // L'enfant est encore vivant = il a ignore le signal
            printf("Parent : enfant %d -> comportement 2 (ignore SIGUSR1)\n", pids[i]);
            // Le tuer proprement
            kill(pids[i], SIGKILL);
            waitpid(pids[i], NULL, 0);
        }
    }

    printf("Tous les enfants sont termines\n");
    return 0;
}
