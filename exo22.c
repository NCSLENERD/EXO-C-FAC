#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define MAX_TEXT 256

struct message {
    long mtype;
    char mtext[MAX_TEXT];
};

void erreur(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [snd|rcv|stat|rm]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    key_t key = ftok(".", 'A');
    if (key == -1)
        erreur("ftok");

    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1)
        erreur("msgget");

    // snd
    if (strcmp(argv[1], "snd") == 0) {
        struct message msg;
        msg.mtype = 1;

        printf("Entrez le message (ligne vide pour terminer) :\n");

        char buffer[MAX_TEXT];
        msg.mtext[0] = '\0';

        while (fgets(buffer, sizeof(buffer), stdin)) {
            if (strcmp(buffer, "\n") == 0)
                break;

            if (strlen(msg.mtext) + strlen(buffer) >= MAX_TEXT) {
                fprintf(stderr, "Message trop long\n");
                exit(EXIT_FAILURE);
            }

            strcat(msg.mtext, buffer);
        }

        if (msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0) == -1)
            erreur("msgsnd");

        printf("Message envoyé\n");
    }

    // rcv
    else if (strcmp(argv[1], "rcv") == 0) {
        struct message msg;

        if (msgrcv(msgid, &msg, MAX_TEXT, 0, 0) == -1)
            erreur("msgrcv");

        printf("Message reçu :\n%s\n", msg.mtext);
    }

    // ✅ stat corrigé
    else if (strcmp(argv[1], "stat") == 0) {
        struct message msg;
        int total_size = 0;
        int count = 0;

        while (1) {
            int ret = msgrcv(msgid, &msg, MAX_TEXT, 0, IPC_NOWAIT);
            if (ret == -1) {
                if (errno == ENOMSG)
                    break;
                else
                    erreur("msgrcv stat");
            }

            total_size += ret;
            count++;

            if (msgsnd(msgid, &msg, ret, 0) == -1)
                erreur("msgsnd stat");
        }

        printf("Nombre de messages : %d\n", count);
        printf("Taille totale : %d octets\n", total_size);
    }

    // rm
    else if (strcmp(argv[1], "rm") == 0) {
        if (msgctl(msgid, IPC_RMID, NULL) == -1)
            erreur("msgctl rm");

        printf("File supprimée\n");
    }

    else {
        fprintf(stderr, "Commande inconnue\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
