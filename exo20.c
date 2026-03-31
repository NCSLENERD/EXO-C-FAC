#include <stdio.h>
#include <openssl/md5.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <string.h>

int zeros(char *s, int n)
{
    int i = 0;
    while (i < n && s[i] == '0')
        i++;
    return (i == n);
}

void bruteforce(int first, int step, int zero, int pipe_fd)
{
    unsigned char hash[50];
    char hex[33];
    char buf[32];
    
    while(1)
    {
        int size = sprintf(buf, "%d", first);
        MD5((unsigned char*)buf, size, hash);

        for (int i = 0; i < 16; i++)
            sprintf(hex + i*2, "%02x", hash[i]);
            
        if(zeros(hex, zero))
        {
            // Écrire le nonce dans le pipe
            write(pipe_fd, buf, strlen(buf) + 1);
        }
        first = first + step;
    }
}

int main(void)
{
    pid_t pids[10];
    int pipes[10][2];
    struct pollfd fds[10];
    
    // Créer les pipes et les processus enfants
    for (int i = 0; i < 10; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
        
        pids[i] = fork();
        if (pids[i] == 0) {
            // Enfant : fermer le côté lecture et faire le bruteforce
            close(pipes[i][0]);
            bruteforce(i + 1, 10, 6, pipes[i][1]);
            close(pipes[i][1]);
            return 0;
        }
        
        // Parent : fermer le côté écriture
        close(pipes[i][1]);
        
        // Configurer pollfd pour ce pipe
        fds[i].fd = pipes[i][0];
        fds[i].events = POLLIN;
    }
    
    // Lire les 5 premiers nonces
    int count = 0;
    char buf[32];
    
    
    while (count < 5) {
        int ret = poll(fds, 10, -1);  // Attendre indéfiniment
        
        if (ret == -1) {
            perror("poll");
            break;
        }
        
        // Vérifier quel pipe a des données
        for (int i = 0; i < 10; i++) {
            if (fds[i].revents & POLLIN) {
                ssize_t n = read(fds[i].fd, buf, sizeof(buf));
                if (n > 0) {
                    printf("Nonce %d trouvé par processus %d : %s\n", 
                           count + 1, pids[i], buf);
                    count++;
                    
                    if (count >= 5)
                        break;
                }
            }
        }
    }
    
    for (int i = 0; i < 10; i++) {
        kill(pids[i], SIGKILL);
        close(pipes[i][0]);
    }

    while (wait(NULL) > 0)
        continue;
    return 0;
}