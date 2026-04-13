// Exercice 5 - date : manipuler des structures et champs de bits
// On calcule la date a partir du nombre de secondes depuis l'epoch (1er jan 1970)
// SANS utiliser localtime, on fait tout a la main

#include <stdio.h>
#include <time.h>

// Enumeration pour les jours de la semaine
typedef enum {
    MON = 1,
    TUE = 2,
    WED = 3,
    THU = 4,
    FRI = 5,
    SAT = 6,
    SUN = 7
} dayname;

// Enumeration pour les mois
typedef enum {
    JAN = 1, FEB = 2, MAR = 3, APR = 4,
    MAY = 5, JUN = 6, JUL = 7, AUG = 8,
    SEP = 9, OCT = 10, NOV = 11, DEC = 12
} monthname;

// Structure pour stocker une date
typedef struct {
    unsigned int year;
    monthname month;
    unsigned int day;
    dayname weekday;
    unsigned int hour;
    unsigned int minute;
    unsigned int second;
} date;

// Q1 : Renvoie le nom du jour
char* dayname_str(dayname day)
{
    switch (day) {
        case MON: return "Monday";
        case TUE: return "Tuesday";
        case WED: return "Wednesday";
        case THU: return "Thursday";
        case FRI: return "Friday";
        case SAT: return "Saturday";
        case SUN: return "Sunday";
        default: return "Unknown";
    }
}

// Q2 : Renvoie le nom du mois
char* monthname_str(monthname month)
{
    switch (month) {
        case JAN: return "January";
        case FEB: return "February";
        case MAR: return "March";
        case APR: return "April";
        case MAY: return "May";
        case JUN: return "June";
        case JUL: return "July";
        case AUG: return "August";
        case SEP: return "September";
        case OCT: return "October";
        case NOV: return "November";
        case DEC: return "December";
        default: return "Unknown";
    }
}

// Q3 : Renvoie le jour de la semaine
// Le 1er janvier 1970 etait un jeudi (THU = 4)
dayname weekday(time_t when)
{
    // Nombre de jours depuis l'epoch
    long jours = when / 86400; // 86400 = 60*60*24 secondes par jour

    // Le 1er jan 1970 = jeudi = 4
    // On calcule le jour : (jours + 3) % 7 + 1
    // +3 car jeudi est le 4eme jour, et on veut lundi=1
    int jour = ((jours % 7) + 3) % 7 + 1;

    return (dayname)jour;
}

// Q4 : Verifie si une annee est bissextile
// Bissextile si divisible par 4 ET pas par 100, OU divisible par 400
int leapyear(unsigned int year)
{
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
        return 1;
    return 0;
}

// Fonction utilitaire : nombre de jours dans un mois
int days_in_month(monthname month, unsigned int year)
{
    switch (month) {
        case JAN: return 31;
        case FEB: return leapyear(year) ? 29 : 28;
        case MAR: return 31;
        case APR: return 30;
        case MAY: return 31;
        case JUN: return 30;
        case JUL: return 31;
        case AUG: return 31;
        case SEP: return 30;
        case OCT: return 31;
        case NOV: return 30;
        case DEC: return 31;
        default: return 30;
    }
}

// Q5 : Calcule la date a partir des secondes depuis l'epoch
date from_time(time_t when)
{
    date d;

    // On extrait les heures, minutes, secondes
    long total_seconds = (long)when;
    d.second = total_seconds % 60;
    total_seconds /= 60;
    d.minute = total_seconds % 60;
    total_seconds /= 60;
    d.hour = total_seconds % 24;

    // Nombre de jours depuis le 1er janvier 1970
    long jours = (long)when / 86400;

    // On cherche l'annee
    unsigned int year = 1970;
    while (1) {
        int days_this_year = leapyear(year) ? 366 : 365;
        if (jours < days_this_year)
            break;
        jours -= days_this_year;
        year++;
    }
    d.year = year;

    // On cherche le mois
    monthname month = JAN;
    while (1) {
        int days = days_in_month(month, year);
        if (jours < days)
            break;
        jours -= days;
        month++;
    }
    d.month = month;

    // Le jour (commence a 1)
    d.day = jours + 1;

    // Le jour de la semaine
    d.weekday = weekday(when);

    return d;
}

// Q6 : Fonction main qui affiche la date du jour
int main(void)
{
    // On recupere le temps actuel
    time_t now = time(NULL);

    // On calcule la date
    date d = from_time(now);

    // On affiche
    printf("%s %d %s %d, %02d:%02d:%02d\n",
           dayname_str(d.weekday),
           d.day,
           monthname_str(d.month),
           d.year,
           d.hour, d.minute, d.second);

    // Test de leapyear
    printf("2024 bissextile ? %s\n", leapyear(2024) ? "oui" : "non");
    printf("2023 bissextile ? %s\n", leapyear(2023) ? "oui" : "non");
    printf("2000 bissextile ? %s\n", leapyear(2000) ? "oui" : "non");
    printf("1900 bissextile ? %s\n", leapyear(1900) ? "oui" : "non");

    return 0;
}
