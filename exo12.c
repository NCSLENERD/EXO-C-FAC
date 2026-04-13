// Exercice 12 - tree : affichage d'une arborescence de repertoire
// On liste recursivement les fichiers avec des caracteres Unicode
// pour dessiner l'arbre

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// Q1 : Verifie si un chemin est un repertoire
int is_dir(char *path)
{
    struct stat st;
    if (stat(path, &st) == -1)
        return 0;
    return S_ISDIR(st.st_mode);
}

// Q2 : Structure pour stocker la liste des fichiers d'un repertoire
typedef struct {
    char **noms;  // tableau de noms de fichiers
    int count;    // nombre de fichiers
} dir_t;

// Q3 : Liste le contenu d'un repertoire (sans . et ..)
dir_t ls(char *path)
{
    dir_t result;
    result.noms = NULL;
    result.count = 0;

    DIR *dir = opendir(path);
    if (!dir)
        return result;

    struct dirent *ent;

    // Premier passage : compter les entrees
    while ((ent = readdir(dir)) != NULL) {
        // On ignore . et ..
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        result.count++;
    }

    // Allouer le tableau
    result.noms = malloc(result.count * sizeof(char *));

    // Deuxieme passage : copier les noms
    rewinddir(dir);
    int i = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        // strdup pour copier le nom (car d_name est libere par closedir)
        result.noms[i] = strdup(ent->d_name);
        i++;
    }

    closedir(dir);
    return result;
}

// Liberer une structure dir_t
void free_dir(dir_t d)
{
    for (int i = 0; i < d.count; i++)
        free(d.noms[i]);
    free(d.noms);
}

// Q4 : Affiche une entree de repertoire avec son prefixe
// last = 1 si c'est la derniere entree du repertoire
void print_name(char *path, char *prefix, int last)
{
    if (last)
        printf("%s\u2514\u2500 %s\n", prefix, path); // L--
    else
        printf("%s\u251c\u2500 %s\n", prefix, path); // |--
}

// Q5 : Affiche recursivement l'arborescence
void tree(char *path, char *prefix, int last)
{
    // Afficher le nom du repertoire/fichier
    print_name(path, prefix, last);

    // Si c'est un repertoire, on le parcourt recursivement
    if (!is_dir(path))
        return;

    // Construire le nouveau prefixe
    char *new_prefix = malloc(strlen(prefix) + 5);
    if (last)
        sprintf(new_prefix, "%s    ", prefix); // espaces car dernier
    else
        sprintf(new_prefix, "%s\u2502   ", prefix); // | car pas dernier

    // Sauvegarder le repertoire courant et entrer dans le sous-repertoire
    // (comme conseille dans le sujet, on utilise chdir)
    char *old_dir = getcwd(NULL, 0);
    chdir(path);

    // Lister le contenu
    dir_t contenu = ls(".");

    // Afficher chaque entree
    for (int i = 0; i < contenu.count; i++) {
        int est_dernier = (i == contenu.count - 1);
        tree(contenu.noms[i], new_prefix, est_dernier);
    }

    // Revenir au repertoire precedent
    chdir(old_dir);
    free(old_dir);
    free(new_prefix);
    free_dir(contenu);
}

// Q6 : Main qui affiche l'arborescence du repertoire courant
int main(void)
{
    printf(".\n");

    dir_t contenu = ls(".");

    for (int i = 0; i < contenu.count; i++) {
        int est_dernier = (i == contenu.count - 1);
        tree(contenu.noms[i], "", est_dernier);
    }

    free_dir(contenu);
    return 0;
}
