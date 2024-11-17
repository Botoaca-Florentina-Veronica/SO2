#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char **argv) 
{
    if (argc != 2) 
    {
        printf("Numar incorect de argumente! Folosire: %s <cale_director>\n", argv[0]);
        exit(1);
    }

    // Deschidem directorul specificat
    DIR *dir = opendir(argv[1]);
    if (dir == NULL) 
    {
        perror("Eroare la deschiderea directorului");
        exit(1);
    }

    struct dirent *entry;
    int pipefd[2];

    // Parcurgem directorul
    while ((entry = readdir(dir)) != NULL) 
    {
        // Construim calea completă a fișierului
        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", argv[1], entry->d_name);

        // Ignorăm directoarele (., .. și alte directoare)
        struct stat file_stat;
        if (lstat(file_path, &file_stat) == -1) 
        {
            perror("Eroare la apelul lstat");
            continue;
        }

        // Verificăm dacă este un fișier obișnuit sau un link simbolic
        if (S_ISREG(file_stat.st_mode) || S_ISLNK(file_stat.st_mode)) 
        {
            // Creăm un pipe pentru a comunica cu procesul copil
            if (pipe(pipefd) == -1) 
            {
                perror("Eroare la crearea pipe-ului");
                exit(1);
            }

            pid_t pid = fork();

            if (pid == -1) 
            {
                perror("Eroare la fork");
                closedir(dir);
                exit(1);
            }

            if (pid == 0) 
            {
                // Proces copil
                close(pipefd[0]);  // Închidem capătul de citire al pipe-ului în copil

                // Redirecționăm ieșirea standard către capătul de scriere al pipe-ului
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);

                // Apelăm `stat` folosind `execlp` pentru a obține dimensiunea și permisiunile fișierului
                execlp("stat", "stat", "-c", "%s %A", file_path, NULL);

                // Dacă execuția lui execlp eșuează
                perror("Eroare la execuția comenzii stat");
                exit(1);
            } 
            else 
            {
                // Proces părinte
                close(pipefd[1]);  // Închidem capătul de scriere al pipe-ului în părinte

                // Citim din pipe datele trimise de procesul copil
                char buffer[128];
                ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
                if (n > 0) 
                {
                    buffer[n] = '\0';  // Terminăm șirul
                    printf("Fișierul: %s\nDetalii: %s\n", file_path, buffer);
                } 
                else 
                {
                    perror("Eroare la citirea din pipe");
                }
                close(pipefd[0]);

                // Așteptăm procesul copil să se termine
                wait(NULL);
            }
        }
    }

    // Închidem directorul
    closedir(dir);

    return 0;
}
