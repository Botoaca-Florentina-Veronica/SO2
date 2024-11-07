/*
   Să se scrie un program C care constă în 2 procese înrudite:
procesul copil: își va seta un comportament nou la recepția semnalului SIGUSR1: va afișa un mesaj și își va termina execuția.
procesul părinte: va trimite semnalul SIGUSR1 procesului copil și după va prelua starea acestuia.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// Handler-ul pentru semnalul SIGUSR1 în procesul copil
void handle_sigusr1(int sig) {
    printf("Procesul copil a primit semnalul SIGUSR1 și se va termina.\n");
    exit(0);  // Procesul copil se închide
}

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        // Eroare la crearea procesului copil
        perror("Eroare la fork");
        exit(1);
    } else if (pid == 0) {
        // Codul procesului copil
        // Setăm handler-ul pentru semnalul SIGUSR1
        struct sigaction sa;
        sa.sa_handler = handle_sigusr1;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        
        if (sigaction(SIGUSR1, &sa, NULL) == -1) {
            perror("Eroare la setarea handler-ului SIGUSR1");
            exit(1);
        }

        // Procesul copil așteaptă semnalul
        while (1) {
            pause();  // Așteaptă primirea unui semnal
        }
    } else {
        // Codul procesului părinte
        sleep(1);  // Pauză pentru a permite procesului copil să își seteze handler-ul

        // Trimitem semnalul SIGUSR1 procesului copil
        if (kill(pid, SIGUSR1) == -1) {
            perror("Eroare la trimiterea semnalului SIGUSR1");
            exit(1);
        }

        // Preluăm starea procesului copil după terminare
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("Eroare la waitpid");
            exit(1);
        }

        if (WIFEXITED(status)) {
            printf("Procesul copil s-a terminat cu codul de ieșire %d.\n", WEXITSTATUS(status));
        } else {
            printf("Procesul copil nu s-a terminat normal.\n");
        }
    }

    return 0;
}
