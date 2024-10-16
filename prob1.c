/* 
  Să se scrie un program care primește ca și argumente în linie de comandă calea către 2 fișiere:
<program> <fișier-intrare> <fișier-ieșire>
Programul va citi în întregime <fișier-intrare>, și va afișa la ieșirea standard octeții transformați după regula următoare:
dacă octetul are valoarea între 97 și 122, va fi afișat folosind printf, ca și literă mică
altfel se va afișa în hexadecimal
La final, programul va scrie în <fișier-ieșire> o linie de forma:
''Numărul total de litere mici afișate este ...''
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Utilizare: %s <fisier-intrare> <fisier-iesire>\n", argv[0]);
        return 1;
    }

    // Deschide fișierul de intrare
    int fd_in;
    fd_in = open(argv[1], O_RDONLY); //fd_in (file-descriptor) este un întreg non-negativ unic al fisierului de intrare, 
    // pe care îl voi folosi ulterior ca parametru în funcția de citire
    // el e necesar fiindcă reprezintă caracteristica unică a fișierului (de intrare în acest caz) pe care o va folosi
    // functia read ca să sție unde în memorie să găsească fișierul pentru a îl citi
    if (fd_in < 0) 
    {
        perror("Eroare la deschiderea fisierului de intrare");
        return 1;
    }

    // Deschide fișierul de ieșire
    int fd_out;
    fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0) 
    {
        perror("Eroare la deschiderea fisierului de iesire");
        close(fd_in);
        return 1;
    }

    char buffer[1024];
    int total_litere_mici = 0;
    ssize_t bytes_read;  //folosesc ssize_t fiindca asta returneaza functia read!!
    
    // Citește fișierul de intrare și procesează octeții
    while ((bytes_read = read(fd_in, buffer, sizeof(buffer))) > 0) 

    // CE DRACU FAC PARAMETRII DIN READ?, fii atentă că acuma îți zic:
    // primul e file descriptorul de mai sus de care vorbeam
    // al doilea parametru din read este un pointer către un buffer în care datele citite vor fi stocate!
    // al treilea reprezinta numărul maxim de octeți pe care read ar trebui să-i citească
    {
        for (ssize_t i = 0; i < bytes_read; i++) 
        {
            unsigned char octet = buffer[i];
            if (octet >= 97 && octet <= 122) 
            {
                // cât timp caracterul curent la care mă aflu, e literă mică, atunci o voi printa
                // și evident îmi măresc contorul pentru litere mici
                printf("%c", octet);
                total_litere_mici++;
            } 
            else 
            {
                //dacă nu e literă mică, o voi printa sub forma caracterului său ascii în format hexa
                printf("%02x", octet);
            }
        }
    }

    /*
    
    Funcția read în C este utilizată pentru a citi date dintr-un fișier sau o resursă de I/O, 
cum ar fi un socket sau un pipe. Aceasta returnează o valoare de tip ssize_t, care indică:

---> Numărul de octeți citiți: Dacă operațiunea de citire are succes, read va returna numărul de octeți 
efectiv citiți. Aceasta poate fi mai mică decât numărul cerut, mai ales la citirea din resurse care nu 
au suficiente date disponibile în acel moment (cum ar fi o conexiune de rețea sau un fișier parțial gol).
---> Zero (0): Un rezultat de 0 indică faptul că s-a ajuns la sfârșitul fișierului (EOF - End of File). 
Acest lucru este valabil mai ales pentru citirea fișierelor și indică faptul că nu mai sunt date de citit.
---> -1: O valoare de -1 indică o eroare. În acest caz, errno este setat pentru a specifica tipul erorii. 
Poți folosi funcția perror pentru a afișa un mesaj de eroare corespunzător, sau strerror(errno) pentru
a obține o descriere textuală a erorii.

*/

    if (bytes_read < 0) 
    {
        perror("Eroare la citirea fișierului de intrare");
        close(fd_in);
        close(fd_out);
        return 1;
    }

    // Scrie mesajul în fișierul de ieșire
    char mesaj[256];
    //Funcția snprintf este o funcție din biblioteca standard C care formatează un șir de caractere 
    //și îl scrie într-un buffer, având la bază un format specificat. Aceasta este similară cu funcția 
    //sprintf, dar cu un avantaj important: snprintf limitează numărul de caractere scrise în buffer, 
    //prevenind astfel depășirea acestuia (buffer overflow).
    int length = snprintf(mesaj, sizeof(mesaj), "Numărul total de litere mici afișate este %d\n", total_litere_mici);
    if (write(fd_out, mesaj, length) < 0) 
    {
        perror("Eroare la scrierea în fișierul de ieșire");
        close(fd_in);
        close(fd_out);
        return 1;
    }

    // Închidem fișierele
    close(fd_in);
    close(fd_out);

    return 0;
}
