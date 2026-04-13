// Exercice 13 - cp : copie de fichiers et repertoires avec permissions
// On utilise les appels systeme open/read/write et stat/chmod/mkdir

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 4096

// Q1 : Replique les permissions du fichier src sur le fichier tgt
void cp_mode(char *src, char *tgt)
{
    struct stat st;
    // On recupere les infos du fichier source
    if (stat(src, &st) == -1) {
        perror("stat");
        return;
    }
    // On applique les memes permissions sur la cible
    if (chmod(tgt, st.st_mode) == -1)
        perror("chmod");
}

// Q2 : Copie un fichier src vers tgt (ecrase si existe)
void cp_file(char *src, char *tgt)
{
    // Ouvrir le fichier source en lecture
    int fd_src = open(src, O_RDONLY);
    if (fd_src == -1) {
        perror("open src");
        return;
    }

    // Creer ou ecraser le fichier cible
    int fd_tgt = open(tgt, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_tgt == -1) {
        perror("open tgt");
        close(fd_src);
        return;
    }

    // Copier le contenu par blocs
    char buf[BUF_SIZE];
    ssize_t n;
    while ((n = read(fd_src, buf, BUF_SIZE)) > 0) {
        // Ecrire tout ce qu'on a lu
        ssize_t ecrit = 0;
        while (ecrit < n) {
            ssize_t w = write(fd_tgt, buf + ecrit, n - ecrit);
            if (w == -1) {
                perror("write");
                close(fd_src);
                close(fd_tgt);
                return;
            }
            ecrit += w;
        }
    }

    close(fd_src);
    close(fd_tgt);

    // Repliquer les permissions
    cp_mode(src, tgt);
}

// Q3 : Copie un repertoire recursivement
void cp_dir(char *src, char *tgt)
{
    struct stat st;

    // Verifier que la cible n'existe pas deja
    if (stat(tgt, &st) == 0) {
        fprintf(stderr, "Erreur : %s existe deja\n", tgt);
        exit(EXIT_FAILURE);
    }

    // Recuperer les permissions du repertoire source
    if (stat(src, &st) == -1) {
        perror("stat src");
        return;
    }

    // Creer le repertoire cible avec les memes permissions
    if (mkdir(tgt, st.st_mode) == -1) {
        perror("mkdir");
        return;
    }

    // Parcourir le contenu du repertoire source
    DIR *dir = opendir(src);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        // Ignorer . et ..
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        // Construire les chemins complets
        char src_path[1024];
        char tgt_path[1024];
        sprintf(src_path, "%s/%s", src, ent->d_name);
        sprintf(tgt_path, "%s/%s", tgt, ent->d_name);

        // Verifier si c'est un repertoire ou un fichier
        struct stat entry_st;
        if (stat(src_path, &entry_st) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(entry_st.st_mode)) {
            // Copie recursive du sous-repertoire
            cp_dir(src_path, tgt_path);
        } else {
            // Copie du fichier
            cp_file(src_path, tgt_path);
        }
    }

    closedir(dir);
}

// Q4 : Programme principal
int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s source destination\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct stat st;
    if (stat(argv[1], &st) == -1) {
        perror("stat");
        return EXIT_FAILURE;
    }

    // Selon le type : copie de fichier ou de repertoire
    if (S_ISDIR(st.st_mode))
        cp_dir(argv[1], argv[2]);
    else
        cp_file(argv[1], argv[2]);

    return EXIT_SUCCESS;
}
