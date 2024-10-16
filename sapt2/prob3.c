/*
  Să se scrie un program care primește ca și argumente în linie de comandă calea către 2 fișiere:
<program> <fișier-intrare> <fișier-ieșire>
Fișierul de intrare va conține un număr necunoscut de numere întregi pe 4 octeți.
Programul va citi fișierul de intrare în întregime și va scrie în fișierul de ieșire, în format text, 
numărul minim, numărul maxim și media numerelor citite din fișierul de intrare binar.
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>

int main(int argc, char *argv[]) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Utilizare: %s <fisier_intrare> <fisier_iesire>\n", argv[0]);
        return 1;
    }

    // Deschiderea fișierului de intrare
    int fd_input = open(argv[1], O_RDONLY);
    if (fd_input < 0) 
    {
        perror("Eroare la deschiderea fișierului de intrare");
        return 1;
    }

    // Deschiderea fișierului de ieșire
    int fd_output = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd_output < 0) 
    {
        perror("Eroare la deschiderea fișierului de ieșire");
        close(fd_input);
        return 1;
    }

    int number;
    int count = 0;
    int min = INT_MAX;
    int max = INT_MIN;
    long sum = 0;

    // Citirea fișierului de intrare

/*
    read(fd_input, &number, sizeof(number)):
 -->read(): Aceasta este o funcție care citește date dintr-un fișier deschis.
 -->fd_input: Este descriptorul de fișier (un întreg) care a fost obținut prin deschiderea fișierului. 
 Acesta indică fișierul din care dorim să citim.
 -->&number: Aceasta este adresa de memorie unde vor fi stocate datele citite. number este o variabilă 
 de tip int, iar &number ne oferă adresa acestei variabile.
 -->sizeof(number): Aceasta specifică câte octeți dorim să citim din fișier. sizeof(number) va returna 4 
 (sau 32 de biți), deoarece number este un int, care ocupă 4 octeți în memorie pe majoritatea platformelor

Compararea cu sizeof(number):
== sizeof(number): Această parte verifică dacă numărul de octeți citiți din fișier este egal cu dimensiunea lui number
 Adică, ne asigurăm că am citit exact 4 octeți (un întreg).
read() returnează numărul de octeți citiți efectiv. Dacă read() reușește să citească 4 octeți,aceasta va returna 4. 
 Dacă nu reușește să citească 4 octeți (de exemplu, dacă a ajuns la finalul fișierului sau a avut 
 loc o eroare), read() va returna un număr mai mic decât 4 sau o valoare negativă (indicând o eroare).

*/
    while (read(fd_input, &number, sizeof(number)) == sizeof(number)) 
    {
        // Actualizăm statistica
        if (number < min) 
        {
            min = number;
        }
        if (number > max) 
        {
            max = number;
        }
        sum = sum + number;
        count++;
    }

/*
    Ciclul while: Va continua să execute blocul de cod din interiorul său atâta timp cât read() reușește să citească exact 4 octeți (un int)
    Scopul: Aceasta este o modalitate de a citi un fișier binar până la sfârșit, asigurându-ne că fiecare număr citit este complet
*/

    // Verificăm dacă am citit cel puțin un număr
    if (count == 0) 
    {
        fprintf(stderr, "Fișierul de intrare nu conține numere.\n");
        close(fd_input);
        close(fd_output);
        return 1;
    }

    double average = (double)sum / count;

    // Scrierea rezultatelor în fișierul de ieșire
    char buffer[256];
    int n = snprintf(buffer, sizeof(buffer), "Min: %d\nMax: %d\nMedia: %.2f\n", min, max, average);
    if (n < 0 || n >= sizeof(buffer)) 
    {
        perror("Eroare la formatarea rezultatului");
        close(fd_input);
        close(fd_output);
        return 1;
    }

    write(fd_output, buffer, n);

    // Închiderea fișierelor
    close(fd_input);
    close(fd_output);

    return 0;
}
