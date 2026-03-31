#include <stdio.h>
#define OPENSSL_SUPPRESS_DEPRECATED
#include <openssl/md5.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int zeros(char *s, int n)
{
    int i = 0;
    while (i < n && s[i] == '0')
        i++;
    return (i == n);
}

void bruteforce(int first, int step, int zero)
{
    unsigned char hash[MD5_DIGEST_LENGTH];
    char hex[33];
    char buf[32];
    char filename[50];
    
    while(1)
    {
        int size = sprintf(buf, "%d", first);
        MD5((unsigned char*)buf, size, hash);
        
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
            sprintf(hex + i*2, "%02x", hash[i]);
        hex[32] = '\0';
        
        if(zeros(hex, zero))
        {
            sprintf(filename, "found.%d", getpid());
            FILE *f = fopen(filename, "w");
            if (!f) {
                perror("Erreur ouverture fichier");
                return;
            }
            fprintf(f, "%s\n", buf);
            fclose(f);
            exit(0);
        }
        first = first + step;
    }
}

int main(void)
{
    pid_t pids[10];
    
    for (int i = 0; i < 10; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            bruteforce(i + 1, 10, 6);
            return 0;
        }
    }
    
    int status;
    pid_t winner;
    
    while ((winner = wait(&status)) > 0) {
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            break;
    }
    
    // Tuer tous les autres enfants
    for (int i = 0; i < 10; i++) {
        kill(pids[i], SIGKILL);
    }
    
    while (wait(NULL) > 0)
        continue;
    
    // Lire le résultat
    char filename[50];
    sprintf(filename, "found.%d", winner);
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Erreur ouverture fichier");
        return 1;
    }
    
    char buf[32];
    fscanf(f, "%s", buf);
    fclose(f);
    
    printf("Nonce trouvé : %s (par processus %d)\n", buf, winner);
    
    // Nettoyer SAUF le fichier du gagnant
    for (int i = 0; i < 10; i++) {
        if (pids[i] != winner) {  // ← MODIFICATION ICI
            char tmp[50];
            sprintf(tmp, "found.%d", pids[i]);
            remove(tmp);
        }
    }
    
    printf("Parent : terminé\n");
    
    return 0;
}