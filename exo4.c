
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

const char digits[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";

typedef struct {
    unsigned char base;
    size_t size;
    unsigned char *data;
} number;

void free_number(number nbr)
{
    if (nbr.data != NULL)
        free(nbr.data);
}

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

    for (size_t i = 0; i < n.size; i++) {

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

number to_base(number nbr, unsigned char base)
{
    if (!(base == 2 || base == 4 || base == 8 ||
          base == 16 || base == 32 || base == 64))
        base = 16;

    int bits_src = 0;
    int tmp = nbr.base;
    while (tmp > 1) { bits_src++; tmp /= 2; }

    int total_bits = nbr.size * bits_src;

    unsigned char *bits = calloc(total_bits, sizeof(unsigned char));

    int pos = 0;
    for (size_t i = 0; i < nbr.size; i++) {
        unsigned char val = nbr.data[i];
        for (int b = bits_src - 1; b >= 0; b--) {
            bits[pos] = (val >> b) & 1;
            pos++;
        }
    }
    int bits_dst = 0;
    tmp = base;
    while (tmp > 1) { bits_dst++; tmp /= 2; }
    int padding = 0;
    if (total_bits % bits_dst != 0)
        padding = bits_dst - (total_bits % bits_dst);

    int new_total = total_bits + padding;
    int nb_chiffres = new_total / bits_dst;

    number res;
    res.base = base;
    res.data = malloc(nb_chiffres * sizeof(unsigned char));
    res.size = nb_chiffres;
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
number add_number(number a, number b)
{
    number b2 = to_base(b, a.base);

    size_t max_size = a.size;
    if (b2.size > max_size) max_size = b2.size;
    max_size++;
    unsigned char *tmp = calloc(max_size, sizeof(unsigned char));
    int retenue = 0;
    for (size_t i = 0; i < max_size; i++) {
        int somme = retenue;
        if (i < a.size)
            somme += a.data[a.size - 1 - i];
        if (i < b2.size)
            somme += b2.data[b2.size - 1 - i];

        tmp[max_size - 1 - i] = somme % a.base;
        retenue = somme / a.base;
    }

    free_number(b2);
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
int cmp_number(number a, number b)
{
    number a2 = to_base(a, 2);
    number b2 = to_base(b, 2);
    if (a2.size > b2.size) { free_number(a2); free_number(b2); return 1; }
    if (a2.size < b2.size) { free_number(a2); free_number(b2); return -1; }
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
    number n1 = from_string("FF", 16);
    printf("from_string(\"FF\", 16) = %u\n", to_uint(n1));
    number n2 = to_base(n1, 2);
    char *str = to_string(n2);
    printf("FF en base 2 = %s\n", str);
    free(str);
    number n3 = to_base(n1, 8);
    str = to_string(n3);
    printf("FF en base 8 = %s\n", str);
    free(str);
    number a = to_number(100, 16);
    number b = to_number(200, 16);
    number somme = add_number(a, b);
    printf("100 + 200 = %u\n", to_uint(somme));
    printf("cmp(100, 200) = %d\n", cmp_number(a, b));
    printf("cmp(200, 100) = %d\n", cmp_number(b, a));
    printf("cmp(100, 100) = %d\n", cmp_number(a, a));
    free_number(n1);
    free_number(n2);
    free_number(n3);
    free_number(a);
    free_number(b);
    free_number(somme);

    return 0;
}
