// Exercice 16 - sleepsort : tri par sleep
// Chaque enfant dort tab[i] secondes puis termine avec tab[i] comme code
// Le parent attend les enfants et recupere les valeurs triees

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    unsigned int tab[10] = {5, 3, 8, 1, 9, 2, 7, 4, 10, 6};

    // Afficher le tableau avant le tri
    printf("Avant : ");
    for (int i = 0; i < 10; i++)
        printf("%d ", tab[i]);
    printf("\n");

    // Q1 : Version simple avec exit() (valeurs <= 127)
    // Pour chaque valeur, on cree un processus enfant
    for (int i = 0; i < 10; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Enfant : dormir tab[i] secondes puis terminer
            sleep(tab[i]);
            exit(tab[i]); // code de retour = la valeur
        }
    }

    // Le parent attend ses 10 enfants
    // Les enfants se reveillent dans l'ordre croissant de tab[i]
    for (int i = 0; i < 10; i++) {
        int status;
        wait(&status);
        if (WIFEXITED(status))
            tab[i] = WEXITSTATUS(status);
    }

    // Afficher le tableau apres le tri
    printf("Apres : ");
    for (int i = 0; i < 10; i++)
        printf("%d ", tab[i]);
    printf("\n");

    return 0;
}
