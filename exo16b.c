// Exercice 16 Q2 - sleepsort avec fichiers
// Version qui gere les entiers > 127 en ecrivant dans un fichier
// au lieu d'utiliser exit()

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    unsigned int tab[10] = {500, 300, 800, 100, 900, 200, 700, 400, 1000, 600};

    // Pour accelerer, on divise les valeurs par un facteur
    // Q4 : on utilise usleep au lieu de sleep pour aller plus vite
    unsigned int mini = tab[0];
    for (int i = 1; i < 10; i++)
        if (tab[i] < mini) mini = tab[i];

    printf("Avant : ");
    for (int i = 0; i < 10; i++)
        printf("%d ", tab[i]);
    printf("\n");

    // Creer un enfant pour chaque valeur
    for (int i = 0; i < 10; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Q4 : usleep au lieu de sleep pour accelerer
            // On dort proportionnellement a la valeur (en microsecondes)
            usleep(tab[i] * 1000); // 1ms par unite

            // Q2 : ecrire le resultat dans un fichier
            char filename[64];
            sprintf(filename, "sleepsort.%d", getpid());
            FILE *f = fopen(filename, "w");
            fprintf(f, "%d\n", tab[i]);
            fclose(f);

            exit(0);
        }
    }

    // Le parent attend les enfants dans l'ordre
    for (int i = 0; i < 10; i++) {
        int status;
        pid_t pid = wait(&status);

        // Lire la valeur depuis le fichier
        char filename[64];
        sprintf(filename, "sleepsort.%d", pid);
        FILE *f = fopen(filename, "r");
        if (f) {
            fscanf(f, "%d", &tab[i]);
            fclose(f);
            remove(filename); // nettoyer le fichier
        }
    }

    printf("Apres : ");
    for (int i = 0; i < 10; i++)
        printf("%d ", tab[i]);
    printf("\n");

    return 0;
}
