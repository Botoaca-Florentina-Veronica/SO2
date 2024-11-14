/*
 Să se scrie un program C care primește ca și argument în linie de comandă calea către un director 
 de pe disc si un numar intreg.
Programul va parcurge directorul primit ca și argument, va procesa în paralel toate fișierele obișnuite 
identificate, le va selecta doar pe acelea care au extensia ''.bin'', iar apoi verifica daca numarul dintr-un fisier
este mai mic ca cel din terminal. Numerele din fisiere sunt in binar pe 4 bytes.
*/

#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<ctype.h>
#include<sys/stat.h>
#include<dirent.h>

//functie care verifica daca un fisier are extensia .bin
int areExtensiaBin(const char *file)
{
    const char *punct;
    punct = strrchr(file, '.');  //imi retin ultima aparitie a lui .
    if(punct != NULL)
    {
        if(strcmp(punct, ".bin") == 0)
        {
            return 1;  // fisierul are extensia .bin
        }
    }
    return 0; //fisierul nu are extensia .bin
}


// Funcție care verifică dacă există un număr mai mic decât valoarea dată în fișierul .bin
int verificaNumar(const char *file, int numar) 
{
    int fd = open(file, O_RDONLY);
    if (fd == -1) 
    {
        perror("Eroare la deschiderea fișierului");
        exit(1);
    }

    int numar_din_fisier;
    while (read(fd, &numar_din_fisier, sizeof(numar_din_fisier)) == sizeof(numar_din_fisier)) 
    {
        if (numar_din_fisier < numar) 
        {
            close(fd);
            return 1; // Găsit un număr mai mic decât valoarea dată
        }
    }

    close(fd);
    return 0; // Nu există numere mai mici decât valoarea dată
}

int main(int argc, char **argv) 
{
    if (argc != 3) 
    {
        printf("Numar incorect de argumente in linia de comanda!!!");
        exit(1);
    }

    int numar = atoi(argv[2]);

    // Deschidem directorul
    DIR *dir = opendir(argv[1]);
    if (dir == NULL) 
    {
        perror("Eroare la deschiderea directorului");
        exit(1);
    }

    struct dirent *entry;
    struct stat file_stat;

    // Parcurgem directorul
    while ((entry = readdir(dir)) != NULL) 
    {
        // Construim calea completă a fișierului
        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", argv[1], entry->d_name);

        // Verificăm dacă fișierul are extensia ".bin"
        if (areExtensiaBin(entry->d_name)) 
        {
            if (stat(file_path, &file_stat) == -1) 
            {
                perror("Eroare la apelul stat");
                continue;
            }

            // Verificăm dacă este un fișier obișnuit
            if (S_ISREG(file_stat.st_mode)) 
            {
                pid_t pid = fork();

                if (pid == -1) 
                {
                    perror("Eroare la fork");
                    closedir(dir);
                    exit(1);
                }

                if (pid == 0) 
                {
                    // Proces copil: verifică dacă există un număr mai mic decât cel specificat în fișierul binar
                    if (verificaNumar(file_path, numar)) 
                    {
                        printf("Fișierul %s conține un număr mai mic decât %d.\n", file_path, numar);
                    }
                    exit(0);
                }
            }
        }
    }

    // Închidem directorul
    closedir(dir);

    // Așteptăm terminarea proceselor copil
    int status;
    while (wait(&status) > 0);

    return 0;
}


/*
```
 **Citirea numerelor din fișier**:
   ```c
   while (read(fd, &numar_din_fisier, sizeof(numar_din_fisier)) == sizeof(numar_din_fisier))
   ```
   - Funcția `read` încearcă să citească din fișier câte 4 bytes (mărimea unui `int`).
   - Dacă funcția `read` reușește să citească exact `sizeof(int)` (adică 4 bytes), valoarea citită este plasată în variabila `numar_din_fisier`, și bucla continuă.
   - Dacă nu sunt 4 bytes disponibili (sau apare o eroare), bucla se oprește.

*/
