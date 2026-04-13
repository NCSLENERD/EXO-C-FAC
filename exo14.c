// Exercice 14 - fusion : tri de fichiers par fusion
// On decoupe un fichier d'entiers en morceaux, on trie chaque morceau,
// puis on fusionne les morceaux deux a deux

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNKCOUNT 1024
#define CHUNKSIZE (CHUNKCOUNT * sizeof(int))

// Q1 : Cree un fichier avec des entiers aleatoires depuis /dev/urandom
void make_data(char *path, unsigned int size)
{
    FILE *src = fopen("/dev/urandom", "rb");
    if (!src) { perror("fopen urandom"); return; }

    FILE *dst = fopen(path, "wb");
    if (!dst) { perror("fopen dst"); fclose(src); return; }

    // On lit size entiers depuis urandom et on les ecrit dans le fichier
    int val;
    for (unsigned int i = 0; i < size; i++) {
        fread(&val, sizeof(int), 1, src);
        fwrite(&val, sizeof(int), 1, dst);
    }

    fclose(src);
    fclose(dst);
}

// Q2 : Verifie si les entiers dans le fichier sont tries
int is_sorted(char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) { perror("fopen"); return 0; }

    int prev, curr;

    // Lire le premier entier
    if (fread(&prev, sizeof(int), 1, f) != 1) {
        fclose(f);
        return 1; // fichier vide = trie
    }

    // Comparer chaque entier avec le precedent
    while (fread(&curr, sizeof(int), 1, f) == 1) {
        if (curr < prev) {
            fclose(f);
            return 0; // pas trie
        }
        prev = curr;
    }

    fclose(f);
    return 1;
}

// Q3 : Fabrique un nom de fichier "base.NNNNNN"
char* chunk_name(char *base, unsigned int count, char *prev)
{
    // Calculer la taille necessaire
    int len = strlen(base) + 1 + 6 + 1; // base + . + 6 chiffres + \0

    char *nom;
    if (prev == NULL)
        nom = malloc(len);
    else
        nom = prev; // reutiliser la memoire

    sprintf(nom, "%s.%06d", base, count);
    return nom;
}

// Fonction de comparaison pour qsort
int compare_int(const void *a, const void *b)
{
    int va = *(const int *)a;
    int vb = *(const int *)b;
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

// Q4 : Decoupe un fichier en morceaux tries
int split(char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) { perror("fopen"); return 0; }

    int buf[CHUNKCOUNT];
    int nb_morceaux = 0;
    char *nom = NULL;
    size_t lus;

    // Lire le fichier par blocs de CHUNKCOUNT entiers
    while ((lus = fread(buf, sizeof(int), CHUNKCOUNT, f)) > 0) {
        // Trier le bloc en memoire
        qsort(buf, lus, sizeof(int), compare_int);

        // Fabriquer le nom du fichier morceau
        nom = chunk_name(path, nb_morceaux, nom);

        // Ecrire le bloc trie dans un fichier
        FILE *out = fopen(nom, "wb");
        if (!out) { perror("fopen chunk"); break; }
        fwrite(buf, sizeof(int), lus, out);
        fclose(out);

        nb_morceaux++;
    }

    free(nom);
    fclose(f);
    return nb_morceaux;
}

// Q5 : Fusionne deux fichiers tries en un seul
void fuse(char *src1, char *src2, char *tgt)
{
    FILE *f1 = fopen(src1, "rb");
    FILE *f2 = fopen(src2, "rb");
    FILE *out = fopen(tgt, "wb");

    if (!f1 || !f2 || !out) {
        perror("fopen fuse");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        if (out) fclose(out);
        return;
    }

    int v1, v2;
    int a_v1 = fread(&v1, sizeof(int), 1, f1); // 1 si on a lu une valeur
    int a_v2 = fread(&v2, sizeof(int), 1, f2);

    // Tant qu'on a des valeurs dans les deux fichiers
    while (a_v1 && a_v2) {
        if (v1 <= v2) {
            fwrite(&v1, sizeof(int), 1, out);
            a_v1 = fread(&v1, sizeof(int), 1, f1);
        } else {
            fwrite(&v2, sizeof(int), 1, out);
            a_v2 = fread(&v2, sizeof(int), 1, f2);
        }
    }

    // Recopier le reste du fichier qui n'est pas fini
    while (a_v1) {
        fwrite(&v1, sizeof(int), 1, out);
        a_v1 = fread(&v1, sizeof(int), 1, f1);
    }
    while (a_v2) {
        fwrite(&v2, sizeof(int), 1, out);
        a_v2 = fread(&v2, sizeof(int), 1, f2);
    }

    fclose(f1);
    fclose(f2);
    fclose(out);
}

// Q6 : Tri par fusion d'un fichier
void sort_file(char *path)
{
    // Etape 1 : decouper en morceaux tries
    int nb = split(path);

    if (nb <= 0) return;

    // Etape 2 : fusionner les morceaux deux a deux
    // On fusionne jusqu'a n'avoir plus qu'un seul fichier
    char *nom1 = NULL;
    char *nom2 = NULL;
    char *nom_tmp = NULL;

    int step = 0; // pour generer des noms temporaires

    while (nb > 1) {
        int new_nb = 0;

        for (int i = 0; i < nb; i += 2) {
            nom1 = chunk_name(path, i, nom1);

            if (i + 1 < nb) {
                // On a deux fichiers a fusionner
                nom2 = chunk_name(path, i + 1, nom2);

                // Nom du fichier resultat
                char tmp_name[256];
                sprintf(tmp_name, "%s.tmp.%06d", path, new_nb);

                fuse(nom1, nom2, tmp_name);

                // Supprimer les fichiers sources
                remove(nom1);
                remove(nom2);

                // Renommer le resultat
                char new_name[256];
                sprintf(new_name, "%s.%06d", path, new_nb);
                rename(tmp_name, new_name);
            } else {
                // Nombre impair : on renomme juste le dernier
                char new_name[256];
                sprintf(new_name, "%s.tmp2.%06d", path, new_nb);
                rename(nom1, new_name);

                // Re-renommer proprement
                char final_name[256];
                sprintf(final_name, "%s.%06d", path, new_nb);
                rename(new_name, final_name);
            }
            new_nb++;
        }

        nb = new_nb;
    }

    free(nom1);
    free(nom2);

    // Renommer le dernier fichier en path.sorted
    char sorted_name[256];
    sprintf(sorted_name, "%s.sorted", path);
    char last_chunk[256];
    sprintf(last_chunk, "%s.%06d", path, 0);
    rename(last_chunk, sorted_name);
}

int main(void)
{
    char *fichier = "test_data";
    int nb_entiers = CHUNKCOUNT * 10; // 10 blocs pour tester

    printf("Creation du fichier de test avec %d entiers...\n", nb_entiers);
    make_data(fichier, nb_entiers);

    printf("Fichier trie avant tri ? %s\n", is_sorted(fichier) ? "oui" : "non");

    printf("Tri en cours...\n");
    sort_file(fichier);

    char sorted_name[256];
    sprintf(sorted_name, "%s.sorted", fichier);
    printf("Fichier trie apres tri ? %s\n", is_sorted(sorted_name) ? "oui" : "non");

    // Nettoyage
    remove(fichier);
    remove(sorted_name);

    printf("Termine !\n");
    return 0;
}
