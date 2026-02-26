#define _XOPEN_SOURCE 700

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_type(mode_t m) {
    if (S_ISREG(m)) printf("-");
    else if (S_ISDIR(m)) printf("d");
    else if (S_ISLNK(m)) printf("l");
    else if (S_ISSOCK(m)) printf("s");
    else if (S_ISFIFO(m)) printf("p");
    else if (S_ISBLK(m)) printf("b");
    else if (S_ISCHR(m)) printf("c");
    else printf("?");
}

void print_perms(mode_t m) {
    char perms[10] = "---------";

 
    if (m & S_IRUSR) perms[0] = 'r';
    if (m & S_IWUSR) perms[1] = 'w';
    if (m & S_IXUSR) perms[2] = 'x';

    if (m & S_IRGRP) perms[3] = 'r';
    if (m & S_IWGRP) perms[4] = 'w';
    if (m & S_IXGRP) perms[5] = 'x';
    
    if (m & S_IROTH) perms[6] = 'r';
    if (m & S_IWOTH) perms[7] = 'w';
    if (m & S_IXOTH) perms[8] = 'x';

    if (m & S_ISUID) perms[2] = (perms[2] == 'x') ? 's' : 'S';
    if (m & S_ISGID) perms[5] = (perms[5] == 'x') ? 's' : 'S';
    if (m & S_ISVTX) perms[8] = (perms[8] == 'x') ? 't' : 'T';

    printf("%s", perms);
}

int main(void) {
    DIR *dir = opendir(".");
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *ent;
    struct stat st;
    char linktarget[PATH_MAX];

    while ((ent = readdir(dir)) != NULL) {

        if (lstat(ent->d_name, &st) == -1) {
            perror("lstat");
            continue;
        }

        
        print_type(st.st_mode);
        print_perms(st.st_mode);
        printf(" %lu", (unsigned long)st.st_nlink);
        printf(" %u %u", st.st_uid, st.st_gid);
        printf(" %lld", (long long)st.st_size);
        printf(" %s", ent->d_name);

        if (S_ISLNK(st.st_mode)) {
            ssize_t len = readlink(ent->d_name, linktarget, sizeof(linktarget) - 1);
            if (len != -1) {
                linktarget[len] = '\0';
                printf(" -> %s", linktarget);
            }
        }

        printf("\n");
    }

    closedir(dir);
    return 0;
}
