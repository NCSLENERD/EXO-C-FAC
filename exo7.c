// Exercice 7 - colors-bis : generer une image XPM avec un degrade
// On utilise les fonctions de l'exercice 6
// ldd: -lm

#include <stdio.h>
#include <math.h>

// --- Copie des types et fonctions de l'exo 6 ---

typedef enum {RGB, HSL} colormode;

typedef union {
    colormode mode;
    struct { colormode mode; unsigned char r, g, b; } rgb;
    struct { colormode mode; double h, s, l; } hsl;
} pixel;

pixel copy_pixel(pixel pix) {
    pixel c;
    if (pix.mode == RGB) { c.rgb.mode = RGB; c.rgb.r = pix.rgb.r; c.rgb.g = pix.rgb.g; c.rgb.b = pix.rgb.b; }
    else { c.hsl.mode = HSL; c.hsl.h = pix.hsl.h; c.hsl.s = pix.hsl.s; c.hsl.l = pix.hsl.l; }
    return c;
}

double max3(double a, double b, double c) { double m = a; if (b > m) m = b; if (c > m) m = c; return m; }
double min3(double a, double b, double c) { double m = a; if (b < m) m = b; if (c < m) m = c; return m; }

pixel to_hsl(pixel pix) {
    if (pix.mode == HSL) return copy_pixel(pix);
    pixel res; res.hsl.mode = HSL;
    double r = pix.rgb.r / 255.0, g = pix.rgb.g / 255.0, b = pix.rgb.b / 255.0;
    double cmax = max3(r,g,b), cmin = min3(r,g,b), delta = cmax - cmin;
    res.hsl.l = (cmax + cmin) / 2.0;
    if (delta == 0) res.hsl.s = 0;
    else res.hsl.s = delta / (1.0 - fabs(2.0 * res.hsl.l - 1.0));
    if (delta == 0) res.hsl.h = 0;
    else if (cmax == r) res.hsl.h = 60.0 * fmod((g - b) / delta, 6.0);
    else if (cmax == g) res.hsl.h = 60.0 * (2.0 + (b - r) / delta);
    else res.hsl.h = 60.0 * (4.0 + (r - g) / delta);
    if (res.hsl.h < 0) res.hsl.h += 360.0;
    return res;
}

pixel to_rgb(pixel pix) {
    if (pix.mode == RGB) return copy_pixel(pix);
    pixel res; res.rgb.mode = RGB;
    double h = pix.hsl.h, s = pix.hsl.s, l = pix.hsl.l;
    double c = (1.0 - fabs(2.0 * l - 1.0)) * s;
    double x = c * (1.0 - fabs(fmod(h / 60.0, 2.0) - 1.0));
    double m = l - c / 2.0;
    double r, g, b;
    if (h < 60)       { r=c; g=x; b=0; }
    else if (h < 120) { r=x; g=c; b=0; }
    else if (h < 180) { r=0; g=c; b=x; }
    else if (h < 240) { r=0; g=x; b=c; }
    else if (h < 300) { r=x; g=0; b=c; }
    else              { r=c; g=0; b=x; }
    res.rgb.r = (unsigned char)((r+m)*255.0+0.5);
    res.rgb.g = (unsigned char)((g+m)*255.0+0.5);
    res.rgb.b = (unsigned char)((b+m)*255.0+0.5);
    return res;
}

void gradient(pixel start, pixel stop, pixel *tab, unsigned int len) {
    pixel hsl_start = to_hsl(start), hsl_stop = to_hsl(stop);
    double h0 = hsl_start.hsl.h, s0 = hsl_start.hsl.s, l0 = hsl_start.hsl.l;
    double hk = hsl_stop.hsl.h, sk = hsl_stop.hsl.s, lk = hsl_stop.hsl.l;
    int k = len - 1;
    for (unsigned int i = 0; i < len; i++) {
        pixel p; p.hsl.mode = HSL;
        p.hsl.h = (h0 * (k - i) + hk * i) / k;
        p.hsl.s = (s0 * (k - i) + sk * i) / k;
        p.hsl.l = (l0 * (k - i) + lk * i) / k;
        if (start.mode == RGB) tab[i] = to_rgb(p);
        else tab[i] = p;
    }
}

// --- Exercice 7 proprement dit ---

// Les caracteres utilises pour les 64 couleurs dans le XPM
// On utilise 1 caractere par pixel, donc on a besoin de 64 caracteres distincts
// On prend les lettres, chiffres et quelques symboles
const char xpm_chars[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int main(void)
{
    // Couleur de depart : rouge
    pixel start;
    start.rgb.mode = RGB;
    start.rgb.r = 255;
    start.rgb.g = 0;
    start.rgb.b = 0;

    // Couleur d'arrivee : bleu
    pixel stop;
    stop.rgb.mode = RGB;
    stop.rgb.r = 0;
    stop.rgb.g = 0;
    stop.rgb.b = 255;

    // Calculer le degrade de 64 couleurs
    pixel degrade[64];
    gradient(start, stop, degrade, 64);

    // Afficher le fichier XPM
    // Image de 64x64 pixels, 64 couleurs, 1 char par pixel
    printf("/* XPM */\n");
    printf("static char *image[] = {\n");
    printf("/* columns rows colors chars-per-pixel */\n");
    printf("\"64 64 64 1\",\n");

    // Definir les 64 couleurs
    for (int i = 0; i < 64; i++) {
        // S'assurer que c'est en RGB pour l'affichage
        pixel p = degrade[i];
        if (p.mode == HSL) p = to_rgb(p);
        printf("\"%c c #%02X%02X%02X\",\n",
               xpm_chars[i], p.rgb.r, p.rgb.g, p.rgb.b);
    }

    // Afficher les 64 lignes de pixels (degrade horizontal)
    // Chaque ligne a 64 pixels, tous de la meme couleur (= 1 bande)
    printf("/* pixels */\n");
    for (int row = 0; row < 64; row++) {
        printf("\"");
        for (int col = 0; col < 64; col++) {
            // Degrade horizontal : la couleur depend de la colonne
            printf("%c", xpm_chars[col]);
        }
        if (row < 63)
            printf("\",\n");
        else
            printf("\"\n");
    }

    printf("};\n");

    return 0;
}
