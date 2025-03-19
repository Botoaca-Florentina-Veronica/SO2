//Scrie un program in C, in care primesti ca argument in linia de comanda calea catre un
//director cu fisiere, care mai poata si acesta sa contina alte directoare cu alte fisiere
//(deci e necesare o parcurgere recursiva). Programul verifica daca exista un regular
// file si daca are dimensiunea mai are ca 10 KB sa fac legatura simbolica si sa o denumesc 
// cu numele fisierului initial plus extensia “mylink”. Apoi se va transmite printr-un 
// proces copil, catre un proces parinte, cate astfel de fisiere au fost redenumite, iar
// procesul parinte va afisa acest rezultat pe ecran

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#define SIZE_THRESHOLD 10 * 1024  // 10 KB

int process_directory(const char *dirpath, int *count) 
{
    DIR *dir;
    struct dirent *entry;
    struct stat st;

    dir = opendir(dirpath);

    if (dir == NULL) 
    {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);

        if (lstat(path, &st) < 0) 
        {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) 
        {
            // Skip "." and ".." directories
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            {
                continue;
            }
            // Recursively process subdirectories
            process_directory(path, count);
        } 
        else if (S_ISREG(st.st_mode)) 
        {
            if (st.st_size > SIZE_THRESHOLD) 
            {
                char linkpath[1024];
                snprintf(linkpath, sizeof(linkpath), "%s/%s.mylink", dirpath, entry->d_name);

                if (symlink(path, linkpath) == 0) 
                {
                    (*count)++;
                } 
                else 
                {
                    perror("symlink");
                }
            }
        }
    }

    closedir(dir);
    return 0;
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Usage: %s <directory_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int count = 0;
    pid_t pid = fork();

    if (pid < 0) 
    {
        perror("fork");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) 
    {
        // Child process
        process_directory(argv[1], &count);
        exit(count);
    } 
    else 
    {
        // Parent process
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) 
        {
            printf("Number of symbolic links created: %d\n", WEXITSTATUS(status));
        } 
        else 
        {
            printf("Child process did not exit normally\n");
        }
    }

    return 0;
}
