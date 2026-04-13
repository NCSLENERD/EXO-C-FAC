// Exercice 4 - basey : suite de l'exercice 3
// Conversion entre bases, addition et comparaison de grands nombres

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Les chiffres pour toutes les bases
const char digits[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";

// Le type number (meme que exo3)
typedef struct {
    unsigned char base;
    size_t size;
    unsigned char *data;
} number;

// Libere la memoire d'un number
void free_number(number nbr)
{
    if (nbr.data != NULL)
        free(nbr.data);
}

// Convertit un entier C en number (repris de exo3)
number to_number(unsigned int nbr, unsigned char base)
{
    unsigned int stock = nbr;
    size_t i = 0;
    number n;
    n.data = NULL;
    n.size = 0;

    if (!(base == 2 || base == 4 || base == 8 ||
          base == 16 || base == 32 || base == 64))
        base = 16;

    n.base = base;

    if (nbr == 0) {
        n.data = malloc(1);
        n.data[0] = 0;
        n.size = 1;
        return n;
    }

    while (stock > 0) {
        stock /= base;
        n.size++;
    }

    n.data = malloc(n.size * sizeof(unsigned char));
    while (nbr > 0 && i < n.size) {
        n.data[(n.size - i) - 1] = nbr % base;
        nbr /= base;
        i++;
    }
    return n;
}

// Convertit un number en entier C (repris de exo3)
unsigned int to_uint(number nbr)
{
    unsigned int res = 0;
    unsigned int i = 0;
    size_t taille = nbr.size;

    while (taille > 0) {
        res += nbr.data[i] * (unsigned int)(pow(nbr.base, taille - 1));
        taille--;
        i++;
    }
    return res;
}

// Convertit un number en chaine de caracteres (repris de exo3)
char* to_string(number nbr)
{
    char *res = malloc((nbr.size * sizeof(char)) + 1);
    if (nbr.size == 0) {
        res[0] = '\0';
        return res;
    }
    for (size_t i = 0; i < nbr.size; i++)
        res[i] = digits[nbr.data[i]];
    res[nbr.size] = '\0';
    return res;
}

// Q1 : Convertit une chaine en number dans la base indiquee
// Ex: from_string("FF", 16) donne 255
number from_string(char *str, unsigned char base)
{
    number n;
    n.data = NULL;
    n.size = 0;

    if (!(base == 2 || base == 4 || base == 8 ||
          base == 16 || base == 32 || base == 64))
        base = 16;

    n.base = base;
    n.size = strlen(str);

    if (n.size == 0) {
        n.data = malloc(1);
        n.data[0] = 0;
        n.size = 1;
        return n;
    }

    n.data = malloc(n.size * sizeof(unsigned char));

    // Pour chaque caractere, on cherche sa valeur dans digits
    for (size_t i = 0; i < n.size; i++) {
        // On cherche le caractere dans la table digits
        int val = -1;
        for (int j = 0; j < 64; j++) {
            if (digits[j] == str[i]) {
                val = j;
                break;
            }
        }
        n.data[i] = val;
    }
    return n;
}

// Q2 : Convertit un number d'une base vers une autre
// Comme les bases sont des puissances de 2, on passe par le binaire
// chiffre par chiffre
number to_base(number nbr, unsigned char base)
{
    if (!(base == 2 || base == 4 || base == 8 ||
          base == 16 || base == 32 || base == 64))
        base = 16;

    // D'abord on convertit en binaire
    // Nombre de bits par chiffre dans la base source
    int bits_src = 0;
    int tmp = nbr.base;
    while (tmp > 1) { bits_src++; tmp /= 2; }

    // Nombre total de bits
    int total_bits = nbr.size * bits_src;

    // Tableau de bits
    unsigned char *bits = calloc(total_bits, sizeof(unsigned char));

    // Remplir les bits depuis le number source
    int pos = 0;
    for (size_t i = 0; i < nbr.size; i++) {
        unsigned char val = nbr.data[i];
        // On ecrit les bits du chiffre (du plus significatif au moins)
        for (int b = bits_src - 1; b >= 0; b--) {
            bits[pos] = (val >> b) & 1;
            pos++;
        }
    }

    // Nombre de bits par chiffre dans la base cible
    int bits_dst = 0;
    tmp = base;
    while (tmp > 1) { bits_dst++; tmp /= 2; }

    // On pad a gauche si le total de bits n'est pas multiple de bits_dst
    int padding = 0;
    if (total_bits % bits_dst != 0)
        padding = bits_dst - (total_bits % bits_dst);

    int new_total = total_bits + padding;
    int nb_chiffres = new_total / bits_dst;

    number res;
    res.base = base;
    res.data = malloc(nb_chiffres * sizeof(unsigned char));
    res.size = nb_chiffres;

    // Lire les bits par groupes de bits_dst
    for (int i = 0; i < nb_chiffres; i++) {
        unsigned char val = 0;
        for (int b = 0; b < bits_dst; b++) {
            int bit_pos = i * bits_dst + b - padding;
            int bit = 0;
            if (bit_pos >= 0 && bit_pos < total_bits)
                bit = bits[bit_pos];
            val = (val << 1) | bit;
        }
        res.data[i] = val;
    }

    free(bits);

    // Enlever les zeros non significatifs
    int start = 0;
    while (start < (int)res.size - 1 && res.data[start] == 0)
        start++;

    if (start > 0) {
        int new_size = res.size - start;
        unsigned char *new_data = malloc(new_size * sizeof(unsigned char));
        for (int i = 0; i < new_size; i++)
            new_data[i] = res.data[start + i];
        free(res.data);
        res.data = new_data;
        res.size = new_size;
    }

    return res;
}

// Q3 : Additionne deux nombres, resultat dans la base de a
// On fait l'addition chiffre par chiffre en passant par la base de a
number add_number(number a, number b)
{
    // Convertir b dans la base de a
    number b2 = to_base(b, a.base);

    // On determine la taille max du resultat
    size_t max_size = a.size;
    if (b2.size > max_size) max_size = b2.size;
    max_size++; // pour la retenue eventuelle

    // Tableau temporaire pour le resultat (on calcule de droite a gauche)
    unsigned char *tmp = calloc(max_size, sizeof(unsigned char));
    int retenue = 0;

    // Addition chiffre par chiffre depuis la droite
    for (size_t i = 0; i < max_size; i++) {
        int somme = retenue;

        // Ajouter le chiffre de a (si il existe)
        if (i < a.size)
            somme += a.data[a.size - 1 - i];

        // Ajouter le chiffre de b2 (si il existe)
        if (i < b2.size)
            somme += b2.data[b2.size - 1 - i];

        tmp[max_size - 1 - i] = somme % a.base;
        retenue = somme / a.base;
    }

    free_number(b2);

    // Enlever les zeros non significatifs
    int start = 0;
    while (start < (int)max_size - 1 && tmp[start] == 0)
        start++;

    number res;
    res.base = a.base;
    res.size = max_size - start;
    res.data = malloc(res.size * sizeof(unsigned char));

    for (size_t i = 0; i < res.size; i++)
        res.data[i] = tmp[start + i];

    free(tmp);
    return res;
}

// Q4 : Compare deux nombres
// Renvoie -1 si a < b, 0 si a == b, +1 si a > b
int cmp_number(number a, number b)
{
    // On convertit dans la meme base (base 2 par exemple)
    number a2 = to_base(a, 2);
    number b2 = to_base(b, 2);

    // Comparer les tailles d'abord
    if (a2.size > b2.size) { free_number(a2); free_number(b2); return 1; }
    if (a2.size < b2.size) { free_number(a2); free_number(b2); return -1; }

    // Meme taille : comparer chiffre par chiffre
    for (size_t i = 0; i < a2.size; i++) {
        if (a2.data[i] > b2.data[i]) { free_number(a2); free_number(b2); return 1; }
        if (a2.data[i] < b2.data[i]) { free_number(a2); free_number(b2); return -1; }
    }

    free_number(a2);
    free_number(b2);
    return 0;
}

int main(void)
{
    // Test from_string
    number n1 = from_string("FF", 16);
    printf("from_string(\"FF\", 16) = %u\n", to_uint(n1));

    // Test to_base : convertir FF (base 16) en base 2
    number n2 = to_base(n1, 2);
    char *str = to_string(n2);
    printf("FF en base 2 = %s\n", str);
    free(str);

    // Test to_base : convertir en base 8
    number n3 = to_base(n1, 8);
    str = to_string(n3);
    printf("FF en base 8 = %s\n", str);
    free(str);

    // Test add_number
    number a = to_number(100, 16);
    number b = to_number(200, 16);
    number somme = add_number(a, b);
    printf("100 + 200 = %u\n", to_uint(somme));

    // Test cmp_number
    printf("cmp(100, 200) = %d\n", cmp_number(a, b));
    printf("cmp(200, 100) = %d\n", cmp_number(b, a));
    printf("cmp(100, 100) = %d\n", cmp_number(a, a));

    // Nettoyage
    free_number(n1);
    free_number(n2);
    free_number(n3);
    free_number(a);
    free_number(b);
    free_number(somme);

    return 0;
}
