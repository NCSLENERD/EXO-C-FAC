// Exercice 10 - env : manipulation de la ligne de commande
// et des variables d'environnement
// Si l'argument est NOM=VALEUR : on set la variable
// Si l'argument est NOM= : on supprime la variable
// Sinon : on concatene tout le reste et on appelle system()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return EXIT_SUCCESS;

    for (int i = 1; i < argc; i++) {
        // Chercher le signe '=' dans l'argument
        char *egal = strchr(argv[i], '=');

        if (egal != NULL) {
            // C'est une variable d'environnement

            // Extraire le nom (avant le =)
            int nom_len = egal - argv[i];
            char *nom = malloc(nom_len + 1);
            strncpy(nom, argv[i], nom_len);
            nom[nom_len] = '\0';

            // La valeur est apres le =
            char *valeur = egal + 1;

            if (strlen(valeur) == 0) {
                // NOM= : on supprime la variable
                unsetenv(nom);
            } else {
                // NOM=VALEUR : on positionne la variable
                if (setenv(nom, valeur, 1) != 0) {
                    perror("setenv");
                    free(nom);
                    return EXIT_FAILURE;
                }
            }
            free(nom);
        }
        else {
            // Ce n'est pas une variable : c'est une commande
            // On concatene tout le reste des arguments

            // D'abord calculer la taille totale necessaire
            int taille = 0;
            for (int j = i; j < argc; j++) {
                taille += strlen(argv[j]);
                if (j < argc - 1)
                    taille++; // pour l'espace
            }

            // Construire la commande
            char *cmd = malloc(taille + 1);
            cmd[0] = '\0';

            for (int j = i; j < argc; j++) {
                strcat(cmd, argv[j]);
                if (j < argc - 1)
                    strcat(cmd, " ");
            }

            // Executer la commande
            int ret = system(cmd);
            free(cmd);
            return ret;
        }
    }

    // Si on arrive ici, tous les arguments etaient des variables
    return EXIT_SUCCESS;
}
