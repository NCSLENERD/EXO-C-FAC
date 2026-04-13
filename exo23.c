// Exercice 23 - msg-pipe : snd et rcv avec un pipe nomme
// On reproduit les commandes snd et rcv de l'exercice 22
// mais avec un pipe nomme (FIFO) au lieu d'une file de messages

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#define PIPE_PATH "/tmp/exo23_pipe"
#define MAX_MSG 256
// On utilise un caractere special pour separer les messages dans le pipe
#define SEPARATEUR '\0'

// Programme serveur : cree le pipe et le garde ouvert
// Lance avec : ./exo23 server
// Le tuer avec Ctrl+C pour nettoyer

int pipe_fd = -1;

void cleanup(int sig)
{
    // Nettoyer le pipe nomme quand on recoit SIGINT
    if (pipe_fd >= 0)
        close(pipe_fd);
    unlink(PIPE_PATH);
    printf("\nPipe supprime, au revoir\n");
    exit(0);
}

void server(void)
{
    // Creer le pipe nomme s'il n'existe pas
    mkfifo(PIPE_PATH, 0666);

    // Installer le handler pour nettoyer en cas de Ctrl+C
    signal(SIGINT, cleanup);

    printf("Serveur demarre, pipe = %s\n", PIPE_PATH);
    printf("Ctrl+C pour arreter\n");

    // Ouvrir le pipe en lecture et ecriture pour eviter le blocage
    pipe_fd = open(PIPE_PATH, O_RDWR);
    if (pipe_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Garder le pipe ouvert indefiniment
    while (1)
        pause();
}

// Envoyer un message
void snd(void)
{
    // Ouvrir le pipe en ecriture (non bloquant)
    int fd = open(PIPE_PATH, O_WRONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("open pipe (le serveur est-il lance ?)");
        exit(EXIT_FAILURE);
    }

    printf("Entrez le message (ligne vide pour terminer) :\n");

    char msg[MAX_MSG];
    msg[0] = '\0';
    char buffer[MAX_MSG];

    // Lire le message ligne par ligne
    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (strcmp(buffer, "\n") == 0)
            break;

        if (strlen(msg) + strlen(buffer) >= MAX_MSG) {
            fprintf(stderr, "Message trop long\n");
            close(fd);
            exit(EXIT_FAILURE);
        }
        strcat(msg, buffer);
    }

    // Ecrire le message dans le pipe, suivi du separateur
    int len = strlen(msg);
    write(fd, msg, len);
    write(fd, &(char){SEPARATEUR}, 1); // separateur de message
    close(fd);

    printf("Message envoye\n");
}

// Recevoir un message
void rcv(void)
{
    // Ouvrir le pipe en lecture
    int fd = open(PIPE_PATH, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("open pipe (le serveur est-il lance ?)");
        exit(EXIT_FAILURE);
    }

    char msg[MAX_MSG];
    int pos = 0;
    char c;

    // Lire caractere par caractere jusqu'au separateur
    while (pos < MAX_MSG - 1) {
        ssize_t n = read(fd, &c, 1);
        if (n <= 0) break;  // rien a lire ou erreur

        if (c == SEPARATEUR) break; // fin du message
        msg[pos] = c;
        pos++;
    }
    msg[pos] = '\0';

    close(fd);

    if (pos == 0) {
        printf("Pas de message\n");
    } else {
        printf("Message recu :\n%s\n", msg);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [server|snd|rcv]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "server") == 0)
        server();
    else if (strcmp(argv[1], "snd") == 0)
        snd();
    else if (strcmp(argv[1], "rcv") == 0)
        rcv();
    else {
        fprintf(stderr, "Commande inconnue : %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
