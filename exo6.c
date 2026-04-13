// Exercice 6 - colors : manipuler des structures et types unions
// Conversions RGB <-> HSL et gradient de couleurs
// ldd: -lm

#include <stdio.h>
#include <math.h>

// Mode de couleur : RGB ou HSL
typedef enum {RGB, HSL} colormode;

// Union pixel : peut etre en RGB ou en HSL
// Le champ mode est commun aux deux structures
typedef union {
    colormode mode;
    struct {
        colormode mode;
        unsigned char r, g, b;
    } rgb;
    struct {
        colormode mode;
        double h, s, l;
    } hsl;
} pixel;

// Q1 : Copie d'un pixel
pixel copy_pixel(pixel pix)
{
    pixel copie;
    // On regarde le mode pour copier les bons champs
    if (pix.mode == RGB) {
        copie.rgb.mode = RGB;
        copie.rgb.r = pix.rgb.r;
        copie.rgb.g = pix.rgb.g;
        copie.rgb.b = pix.rgb.b;
    } else {
        copie.hsl.mode = HSL;
        copie.hsl.h = pix.hsl.h;
        copie.hsl.s = pix.hsl.s;
        copie.hsl.l = pix.hsl.l;
    }
    return copie;
}

// Petite fonction pour trouver le max de 3 doubles
double max3(double a, double b, double c)
{
    double m = a;
    if (b > m) m = b;
    if (c > m) m = c;
    return m;
}

// Petite fonction pour trouver le min de 3 doubles
double min3(double a, double b, double c)
{
    double m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    return m;
}

// Q2 : Convertit un pixel en HSL
// Formules donnees dans le sujet (figure 3)
pixel to_hsl(pixel pix)
{
    // Si deja en HSL, on renvoie une copie
    if (pix.mode == HSL)
        return copy_pixel(pix);

    pixel res;
    res.hsl.mode = HSL;

    // Normaliser les composantes entre 0 et 1
    double r = pix.rgb.r / 255.0;
    double g = pix.rgb.g / 255.0;
    double b = pix.rgb.b / 255.0;

    double cmax = max3(r, g, b);
    double cmin = min3(r, g, b);
    double delta = cmax - cmin;

    // Calcul de L (luminosite)
    res.hsl.l = (cmax + cmin) / 2.0;

    // Calcul de S (saturation)
    if (delta == 0)
        res.hsl.s = 0;
    else
        res.hsl.s = delta / (1.0 - fabs(2.0 * res.hsl.l - 1.0));

    // Calcul de H (teinte)
    if (delta == 0) {
        res.hsl.h = 0;
    } else if (cmax == r) {
        res.hsl.h = 60.0 * fmod((g - b) / delta, 6.0);
    } else if (cmax == g) {
        res.hsl.h = 60.0 * (2.0 + (b - r) / delta);
    } else {
        res.hsl.h = 60.0 * (4.0 + (r - g) / delta);
    }

    // S'assurer que H est positif
    if (res.hsl.h < 0)
        res.hsl.h += 360.0;

    return res;
}

// Q3 : Convertit un pixel en RGB
// Formules donnees dans le sujet (figure 3)
pixel to_rgb(pixel pix)
{
    // Si deja en RGB, on renvoie une copie
    if (pix.mode == RGB)
        return copy_pixel(pix);

    pixel res;
    res.rgb.mode = RGB;

    double h = pix.hsl.h;
    double s = pix.hsl.s;
    double l = pix.hsl.l;

    double c = (1.0 - fabs(2.0 * l - 1.0)) * s;
    double x = c * (1.0 - fabs(fmod(h / 60.0, 2.0) - 1.0));
    double m = l - c / 2.0;

    double r, g, b;

    if (h < 60)       { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else              { r = c; g = 0; b = x; }

    // Convertir en 0-255 et arrondir
    res.rgb.r = (unsigned char)((r + m) * 255.0 + 0.5);
    res.rgb.g = (unsigned char)((g + m) * 255.0 + 0.5);
    res.rgb.b = (unsigned char)((b + m) * 255.0 + 0.5);

    return res;
}

// Q4 : Remplit un tableau avec un degrade de couleurs
// On interpole en HSL entre start et stop
void gradient(pixel start, pixel stop, pixel *tab, unsigned int len)
{
    // Convertir start et stop en HSL pour l'interpolation
    pixel hsl_start = to_hsl(start);
    pixel hsl_stop = to_hsl(stop);

    double h0 = hsl_start.hsl.h;
    double s0 = hsl_start.hsl.s;
    double l0 = hsl_start.hsl.l;
    double hk = hsl_stop.hsl.h;
    double sk = hsl_stop.hsl.s;
    double lk = hsl_stop.hsl.l;

    int k = len - 1; // car on veut k+1 = len couleurs

    for (unsigned int i = 0; i < len; i++) {
        // Interpolation lineaire des composantes HSL
        pixel p;
        p.hsl.mode = HSL;
        p.hsl.h = (h0 * (k - i) + hk * i) / k;
        p.hsl.s = (s0 * (k - i) + sk * i) / k;
        p.hsl.l = (l0 * (k - i) + lk * i) / k;

        // Convertir dans le mode de start
        if (start.mode == RGB)
            tab[i] = to_rgb(p);
        else
            tab[i] = p;
    }
}

int main(void)
{
    // Test : creer un pixel rouge en RGB
    pixel rouge;
    rouge.rgb.mode = RGB;
    rouge.rgb.r = 255;
    rouge.rgb.g = 0;
    rouge.rgb.b = 0;

    // Convertir en HSL
    pixel rouge_hsl = to_hsl(rouge);
    printf("Rouge en HSL : H=%.1f S=%.2f L=%.2f\n",
           rouge_hsl.hsl.h, rouge_hsl.hsl.s, rouge_hsl.hsl.l);

    // Reconvertir en RGB
    pixel rouge_rgb = to_rgb(rouge_hsl);
    printf("Rouge reconverti en RGB : R=%d G=%d B=%d\n",
           rouge_rgb.rgb.r, rouge_rgb.rgb.g, rouge_rgb.rgb.b);

    // Test du degrade
    pixel bleu;
    bleu.rgb.mode = RGB;
    bleu.rgb.r = 0;
    bleu.rgb.g = 0;
    bleu.rgb.b = 255;

    pixel degrade[8];
    gradient(rouge, bleu, degrade, 8);

    printf("\nDegrade rouge -> bleu (8 couleurs) :\n");
    for (int i = 0; i < 8; i++) {
        printf("  Couleur %d : R=%d G=%d B=%d\n",
               i, degrade[i].rgb.r, degrade[i].rgb.g, degrade[i].rgb.b);
    }

    return 0;
}
