/*
Să se scrie un program C, care constă în 2 procese înrudite:
procesul copil: va invoca comanda "ls -l" și se va asigura că informația generată de această comandă este transmisă părintelui folosind un pipe
procesul părinte: va număra câți octeți a primit prin pipe, va afișa acest număr și după va prelua starea procesului copil.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipe_fd[2]; // Descriptori pentru pipe
    pid_t pid;
    
    // Creăm pipe-ul
    if (pipe(pipe_fd) == -1) {
        perror("Eroare la crearea pipe-ului");
        exit(1);
    }
    
    // Creăm procesul copil
    pid = fork();
    if (pid == -1) {
        perror("Eroare la fork");
        exit(1);
    }
    
    if (pid == 0) { // Cod pentru procesul copil
        close(pipe_fd[0]); // Închidem capătul de citire al pipe-ului în copil
        
        // Redirecționăm ieșirea standard a copilului către capătul de scriere al pipe-ului
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]); // Închidem capătul de scriere după redirecționare
        
        // Executăm comanda "ls -l"
        execlp("ls", "ls", "-l", (char *)NULL);
        
        // Dacă execlp eșuează
        perror("Eroare la execlp");
        exit(1);
    } 
    else 
    { // Cod pentru procesul părinte
        close(pipe_fd[1]); // Închidem capătul de scriere al pipe-ului în părinte
        
        // Citim datele primite prin pipe și calculăm numărul de octeți
        char buffer[1024];
        ssize_t bytes_read;
        int total_bytes = 0;
        
        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
            total_bytes += bytes_read;
        }
        
        // Închidem capătul de citire al pipe-ului
        close(pipe_fd[0]);
        
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
