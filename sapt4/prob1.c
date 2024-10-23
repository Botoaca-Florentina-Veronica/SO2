/*
 Să se scrie un program C care primește ca și argument în linie de comandă calea către un director de pe disc.
Programul va parcurge directorul primit ca și argument, va procesa în paralel conținutul tuturor fișierelor obișnuite identificate și va calcula numărul total de litere mici din acestea. 
INDICIU: se va crea câte un proces copil pentru fiecare fișier obișnuit identificat și acestea vor apela exit având ca și parametru numărul de litere mici pentru fișierul curent.
Părintele va prelua informația aceasta folosind wait + WIFEXITED + WEXITSTATUS.
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define BUF_SIZE 1024

// Funcție pentru a număra literele mici dintr-un fișier
int count_lowercase_letters(const char *file_path) 
{
    int i;
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) 
    {
        perror("Eroare la deschiderea fișierului");
        exit(1);
    }

    char buffer[BUF_SIZE];
    int count = 0, bytes_read;

    // Citim conținutul fișierului în buffer și numărăm literele mici
    while ((bytes_read = read(fd, buffer, BUF_SIZE)) > 0) 
    {
        for (i = 0; i < bytes_read; i++) 
        {
            if (islower(buffer[i])) 
            {
                count++;
            }
        }
    }

    close(fd);

    return count;
}

// Funcție care procesează directorul și lansează procese copil
void process_directory(const char *dir_path) 
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL) 
    {
        perror("Eroare la deschiderea directorului");
        exit(1);
    }

    struct dirent *entry;
    struct stat file_stat;
    int total_lowercase = 0;

    // Parcurgem toate intrările/fisierele din director
    while ((entry = readdir(dir)) != NULL) 
    {
        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

        // Obținem informațiile despre fișier
        if (stat(file_path, &file_stat) == -1) 
        {
            perror("Eroare la functia stat");
            continue;
        }

        // Verificăm dacă este un fișier obișnuit
        if (S_ISREG(file_stat.st_mode)) 
        {
            pid_t pid = fork();
            if (pid == -1) 
            {
                perror("Eroare la fork");
                exit(1);
            }

            if (pid == 0) 
            {
                // Proces copil - numărăm literele mici
                int lowercase_count = count_lowercase_letters(file_path);
                exit(lowercase_count); // Procesul copil iese cu numărul de litere mici
            }
        }
    }

    // Așteptăm toate procesele copil și colectăm rezultatele
    int status;
    while (wait(&status) > 0) 
    {
        if (WIFEXITED(status)) 
        {
            int lowercase_count = WEXITSTATUS(status);
            total_lowercase += lowercase_count;
        }
    }

    printf("Numărul total de litere mici: %d\n", total_lowercase);

    closedir(dir);
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Utilizare: %s <cale_director>\n", argv[0]);
        exit(1);
    }

    process_directory(argv[1]);

    return 0;
}


// TEORIE

/*
Cum funcționează PID:
Crearea proceselor noi:

   Când un proces este creat folosind funcția fork(), sistemul de operare atribuie un nou PID procesului copil. 
Procesul copil are un PID diferit de cel al procesului părinte.
Procesul copil primește un PID pozitiv, în timp ce procesul părinte primește PID-ul procesului copil.
Funcția fork():

   Aceasta este o funcție fundamentală în Unix/Linux care creează un nou proces.
Procesul nou creat (copilul) este o copie aproape identică a procesului părinte.
Returnează:
--> 0 în procesul copil (indicând că acesta este copilul).
PID-ul procesului copil în procesul părinte (indicând succesul în crearea copilului).

--> -1 în caz de eroare (procesul copil nu a putut fi creat).
*/
