/*
   Să se scrie un program C care primește ca și argument în linie de comandă calea către un director 
de pe disc.
Programul va parcurge directorul primit ca și argument, va procesa în paralel toate fișierele obișnuite
 identificate, le va selecta doar pe acelea care au extensia ''.c'' și va invoca gcc pentru a le compila.
INDICIU: se va crea câte un proces copil pentru fiecare fișier obișnuit identificat cu extensia cerută și 
se va folosi execlp/execvp pentru a invoca gcc.
*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

// Funcție care verifică dacă un fișier are extensia ".c"
int areExtensiaC(const char *nume_fisier) 
{
    // Caută ultima apariție a caracterului '.' în numele fișierului
    const char *punct = strrchr(nume_fisier, '.');
    
    // Verificăm dacă am găsit un punct și dacă acesta este la finalul ".c"
    if (punct != NULL) 
    {
        // Compara dacă textul după ultimul punct este ".c"
        if (strcmp(punct, ".c") == 0) 
        {
            return 1; // Fișierul are extensia ".c"
        }
    }
    
    return 0; // Fișierul nu are extensia ".c"
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Utilizare: %s <cale_director>\n", argv[0]);
        exit(1);
    }

    DIR *dir = opendir(argv[1]);
    if (dir == NULL) 
    {
        perror("Eroare la deschiderea directorului");
        exit(1);
    }

    struct dirent *entry;
    struct stat file_stat;

    while ((entry = readdir(dir)) != NULL) 
    {
        // Verificăm dacă intrarea are extensia ".c", iar apoi daca este un fișier obișnuit
        if (areExtensiaC(entry->d_name)) 
        {
            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", argv[1], entry->d_name);
            
            if(lstat(file_path, &file_stat) == -1)
            {
                perror("Eroare la functia stat!!");
                continue;
            }

            if(S_ISREG(file_stat.st_mode))
            {
                pid_t pid = fork();
                if (pid == -1) 
                {
                    perror("Eroare la crearea procesului copil");
                    closedir(dir);
                    exit(1);
                } 
                
                if (pid == 0) 
                {
                    // Suntem în procesul copil
                    // Numele executabilului va fi numele fișierului fără extensia .c

                    char output_name[1024];
                    snprintf(output_name, sizeof(output_name), "%s/%.*s", argv[1], (int)(strrchr(entry->d_name, '.') - entry->d_name), entry->d_name);

                    // Invocăm gcc pentru a compila fișierul
                    execlp("gcc", "gcc", file_path, "-o", output_name, NULL);
                    
                    // Dacă execlp eșuează
                    perror("Eroare la execlp");
                    exit(0);
                }
                else
                {
                    //proces parinte

                    // Așteptăm toate procesele copil și verificăm starea fiecăruia
                    int status;
                    while (wait(&status) > 0) 
                    {
                        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) 
                        {
                            printf("Un fișier a fost compilat cu succes.\n");
                        } 
                        else 
                        {
                            printf("Eroare la compilarea unui fișier.\n");
                        }
                    }
                }
            }
        }
    }

    // Închidem directorul
    closedir(dir);
    printf("Compilarea s-a încheiat pentru toate fișierele.\n");
    return 0;
}
