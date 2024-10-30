/*
Să se scrie un program C, care constă în 2 procese înrudite:
procesul copil: va citi în întregime un fișier text primit ca și argument în linie de comandă și va trimite doar vocalele procesului părinte folosind un pipe
procesul părinte: va număra câți octeți a primit prin pipe, va afișa acest număr și după va prelua starea procesului copil.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

int esteVocala(char c) 
{
    c = tolower(c);
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        fprintf(stderr, "Utilizare: %s <nume_fisier>\n", argv[0]);
        exit(1);
    }

    int pipe_fd[2]; // Descriptori pentru pipe
    pid_t pid;

    // Creăm pipe-ul
    if (pipe(pipe_fd) == -1) 
    {
        perror("Eroare la crearea pipe-ului");
        exit(1);
    }

    // Creăm procesul copil
    pid = fork();
    if (pid == -1) 
    {
        perror("Eroare la fork");
        exit(1);
    }

    if (pid == 0) 
    { // Cod pentru procesul copil
        close(pipe_fd[0]); // Închidem capătul de citire al pipe-ului în copil

        FILE *fisier = fopen(argv[1], "r");
        if (fisier == NULL) 
        {
            perror("Eroare la deschiderea fișierului");
            exit(1);
        }

        char c;
        while ((c = fgetc(fisier)) != EOF) 
        {
            if (esteVocala(c)) 
            {
                write(pipe_fd[1], &c, 1); // Trimitem vocala prin pipe
            }
        }

        fclose(fisier);
        close(pipe_fd[1]); // Închidem capătul de scriere după ce am terminat
        exit(0); // Terminăm procesul copil
    } 
    else 
    { // Cod pentru procesul părinte
        close(pipe_fd[1]); // Închidem capătul de scriere al pipe-ului în părinte

        char buffer[1024];
        ssize_t bytes_read;
        int total_bytes = 0;

        // Citim datele primite prin pipe și calculăm numărul de octeți
        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) 
        {
            total_bytes += bytes_read;
        }

        close(pipe_fd[0]); // Închidem capătul de citire al pipe-ului

        // Afișăm numărul total de octeți primiți
        printf("Numărul de octeți primiți prin pipe: %d\n", total_bytes);

        // Așteptăm terminarea procesului copil și preluăm starea
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) 
        {
            printf("Procesul copil s-a terminat cu codul de ieșire: %d\n", WEXITSTATUS(status));
        }
    }

    return 0;
}
