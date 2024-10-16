/*
 Să se scrie un program care primește ca și argument în linie de comandă calea către o intrare de pe disc.
Programul va afișa pentru intrarea respectivă următoarele informații:
Tipul intrării: director / fișier obișnuit / legătură simbolică
Permisiunile pentru owner/user, sub forma: rwx, - dacă vreuna lipsește
Dimensiunea în octeți 
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

void print_permissions(struct stat fileStat) 
{
    // Permisiuni pentru owner
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Utilizare: %s <cale_fisier>\n", argv[0]);
        return 1;
    }

    struct stat fileStat;
    
    // Folosim lstat pentru a obține informații și despre legături simbolice
    if (lstat(argv[1], &fileStat) < 0) 
    {
        perror("Eroare la lstat");
        return 1;
    }

    // Determinăm tipul de fișier
    if (S_ISREG(fileStat.st_mode)) 
    {
        printf("Tipul intrării: Fișier obișnuit\n");
    } 
    else if (S_ISDIR(fileStat.st_mode)) 
    {
        printf("Tipul intrării: Director\n");
    } 
    else if (S_ISLNK(fileStat.st_mode)) 
    {
        printf("Tipul intrării: Legătură simbolică\n");
    } 
    else 
    {
        printf("Tipul intrării: Alt tip\n");
    }

    // Afișăm permisiunile pentru owner/user
    printf("Permisiunile pentru owner/user: ");
    print_permissions(fileStat);
    printf("\n");

    // Afișăm dimensiunea în octeți
    printf("Dimensiunea în octeți: %ld\n", fileStat.st_size);

    return 0;
}
