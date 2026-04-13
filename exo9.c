// Exercice 9 - rpn : calculatrice en notation polonaise inverse
// On lit des nombres et operateurs sur la ligne de commande
// et on utilise une pile pour les traiter

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// La pile : tableau de taille fixe 128
long pile[128];
int sommet = 0; // position du prochain emplacement libre

// Empile une valeur, renvoie 0 si ok, -1 si pile pleine
int empiler(long val)
{
    if (sommet >= 128)
        return -1; // pile pleine
    pile[sommet] = val;
    sommet++;
    return 0;
}

// Depile une valeur, renvoie 0 si ok, -1 si pile vide
int depiler(long *val)
{
    if (sommet <= 0)
        return -1; // pile vide
    sommet--;
    *val = pile[sommet];
    return 0;
}

// Convertit une chaine en entier dans la base donnee
// Renvoie 0 si ok, -1 si erreur
int convertir(char *str, int base, long *resultat)
{
    char *fin;
    *resultat = strtol(str, &fin, base);

    // Si fin ne pointe pas sur '\0', la conversion a echoue
    if (*fin != '\0')
        return -1;
    return 0;
}

int main(int argc, char *argv[])
{
    // Recuperer la base depuis la variable d'environnement BASE
    int base = 10; // base par defaut
    char *base_env = getenv("BASE");

    if (base_env != NULL) {
        char *fin;
        long b = strtol(base_env, &fin, 10);
        if (*fin != '\0' || b < 2 || b > 16) {
            // Base invalide
            return 250;
        }
        base = (int)b;
    }

    // Pas d'arguments = rien a faire
    if (argc < 2) {
        return EXIT_SUCCESS;
    }

    // Traiter chaque argument
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        // Verifier si c'est un operateur (un seul caractere parmi +-*/)
        if (strlen(arg) == 1 && (arg[0] == '+' || arg[0] == '-' ||
                                  arg[0] == '*' || arg[0] == '/')) {
            // C'est un operateur
            long b, a;

            // Depiler b puis a
            if (depiler(&b) != 0)
                return 253; // pas assez d'arguments sur la pile

            if (depiler(&a) != 0)
                return 253;

            long resultat;
            switch (arg[0]) {
                case '+': resultat = a + b; break;
                case '-': resultat = a - b; break;
                case '*': resultat = a * b; break;
                case '/':
                    if (b == 0)
                        return 252; // division par zero
                    resultat = a / b;
                    break;
                default: return EXIT_FAILURE;
            }

            // Empiler le resultat
            if (empiler(resultat) != 0)
                return 254; // pile pleine
        }
        else {
            // C'est un nombre (on essaye de le convertir)
            long val;
            if (convertir(arg, base, &val) != 0)
                return 251; // entier incorrect

            if (empiler(val) != 0)
                return 254; // pile pleine
        }
    }

    // Depiler et afficher le resultat
    long resultat;
    if (depiler(&resultat) != 0) {
        // Pile vide, pas de resultat
        return EXIT_SUCCESS;
    }

    printf("%ld\n", resultat);

    // Verifier s'il reste des valeurs sur la pile
    if (sommet > 0)
        return 255;

    return EXIT_SUCCESS;
}
