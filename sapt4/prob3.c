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


// TEORIE!!!
/*
Un pipe (sau "conductă") este un mecanism de comunicare între două procese într-un sistem de operare bazat pe Unix (precum Linux). Pipe-urile permit transferul de date într-un singur sens: un proces scrie în pipe și altul citește din pipe.
Cum funcționează un pipe în mod fundamental
Crearea unui pipe:

În C, un pipe este creat prin apelul funcției pipe(int pipe_fd[2]), care primește un tablou de două elemente pipe_fd.
pipe_fd[0] este descriptorul capătului de citire.
pipe_fd[1] este descriptorul capătului de scriere.
Odată creat, datele scrise în pipe_fd[1] pot fi citite din pipe_fd[0].
Comunicare unidirecțională:

Datele curg doar într-o singură direcție printr-un pipe standard, de la capătul de scriere către capătul de citire.
Dacă un proces dorește să trimită date înapoi, este necesar un alt pipe pentru a crea un flux bidirecțional de date.
Blocarea operațiilor:

Dacă procesul de citire încearcă să citească din pipe atunci când nu există date, acesta se blochează (așteaptă) până când sunt disponibile date.
În mod similar, dacă procesul de scriere încearcă să scrie atunci când pipe-ul este plin, acesta se blochează până când există spațiu liber.
Detectarea sfârșitului datelor:

Atunci când capătul de scriere este închis de toate procesele care îl folosesc, orice proces de citire primește un semnal EOF (End of File) atunci când nu mai sunt date de citit.
Acest lucru permite procesului care citește să știe că nu mai urmează alte date și că fluxul de comunicare s-a încheiat.
Funcționarea pipe-urilor cu procesele copil:

De obicei, pipe-urile sunt folosite între procese părinte și procese copil.
Când un proces creează un pipe și apoi folosește fork() pentru a crea un proces copil, ambele procese vor avea acces la aceleași capete ale pipe-ului.
Prin închiderea capetelor neutilizate (de exemplu, copilul închide capătul de citire și părintele capătul de scriere), se delimitează direcția de comunicare și se evită potențiale blocaje sau confuzii.
Exemplu simplu de flux de date printr-un pipe
Să presupunem că avem un proces părinte și un proces copil care folosesc un pipe pentru a transfera date de la copil la părinte:

Procesul părinte creează un pipe (pipe(pipe_fd)).
Procesul părinte folosește fork() pentru a crea un proces copil.

În copil:
Capătul de citire pipe_fd[0] este închis, iar copilul scrie date în pipe_fd[1].

În părinte:
Capătul de scriere pipe_fd[1] este închis, iar părintele citește date din pipe_fd[0].

Când copilul termină de scris datele, închide pipe_fd[1]. Părintele va citi datele și, la final, va primi EOF, indicând că nu mai sunt date.
Astfel, pipe-ul oferă un canal simplu de comunicare între procese, permițând transmiterea de date cu sincronizare automată a citirii și scrierii.
*/
