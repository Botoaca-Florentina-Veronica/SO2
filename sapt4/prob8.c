/*
Să se scrie un program C care primește ca și argument în linie de comandă calea către un director de pe disc.
Programul va parcurge directorul primit ca și argument si va procesa în paralel conținutul tuturor fișierelor identificate.
Se vor utiliza 2 procese înrudite:
->procesul copil: va citi în întregime un fișier text primit ca și argument în linie de comandă și va
trimite doar numarul de linii procesului părinte folosind un pipe
->procesul părinte: va număra câți octeți a primit prin pipe, va afișa acest număr și după va prelua 
starea procesului copil.  --asta am inteles eu, nu arunca cu pietre in mine, stiu ca suna ca dracu--
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

// Funcție care numără liniile într-un fișier text
int numarLinii(const char *file) {
    int fd = open(file, O_RDONLY);
    if (fd == -1) 
    {
        perror("Eroare la deschiderea fișierului!!");
        exit(1);
    }

    int nrLinii = 0;
    char buffer[1024];
    int bytes_read;

    // Citim fișierul în blocuri și numărăm liniile
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) 
    {
        for (int i = 0; i < bytes_read; i++) 
        {
            if (buffer[i] == '\n') 
            {
                nrLinii++;
            }
        }
    }

    close(fd);
    return nrLinii;
}

int main(int argc, char **argv) 
{
    if (argc != 2) 
    {
        printf("Număr incorect de argumente în linia de comandă!!\n");
        exit(1);
    }

    // Deschidem directorul specificat
    DIR *dir = opendir(argv[1]);
    if (dir == NULL) 
    {
        perror("Eroare la deschiderea directorului!!");
        exit(1);
    }

    struct dirent *entry;
    struct stat file_stat;

    // Parcurgem fișierele din director
    while ((entry = readdir(dir)) != NULL) 
    {
        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", argv[1], entry->d_name);

        if (stat(file_path, &file_stat) == -1) 
        {
            perror("Eroare la stat!!");
            continue;
        }

        // Verificăm dacă este un fișier obișnuit
        if (S_ISREG(file_stat.st_mode)) 
        {
            int pipefd[2];
            if (pipe(pipefd) == -1) 
            {
                perror("Eroare la crearea pipe-ului");
                exit(1);
            }

            pid_t pid = fork();

            if (pid == -1) 
            {
                perror("Eroare la fork!!");
                exit(1);
            }

            if (pid == 0) 
            {
                // Proces copil: calculează numărul de linii și îl trimite procesului părinte
                close(pipefd[0]); // închidem capătul de citire

                int nrLinii = numarLinii(file_path);
                write(pipefd[1], &nrLinii, sizeof(nrLinii));

                close(pipefd[1]); // închidem capătul de scriere
                closedir(dir);    // închidem directorul deschis
                exit(0);
            } 
            else 
            {
                // Proces părinte: citește din pipe și așteaptă starea procesului copil
                close(pipefd[1]); // închidem capătul de scriere

                int nrLinii;
                read(pipefd[0], &nrLinii, sizeof(nrLinii));

                printf("Fișierul %s are %d linii.\n", file_path, nrLinii);
                close(pipefd[0]); // închidem capătul de citire

                // Așteptăm procesul copil și verificăm starea sa
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status)) 
                {
                    printf("Procesul copil s-a terminat cu codul de ieșire %d.\n", WEXITSTATUS(status));
                } 
                else 
                {
                    printf("Procesul copil nu s-a terminat corect.\n");
                }
            }
        }
    }

    closedir(dir); // Închidem directorul
    return 0;
}
