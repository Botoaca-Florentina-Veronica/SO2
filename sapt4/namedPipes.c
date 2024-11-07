/*
   Named pipes, cunoscute și ca FIFO (First In, First Out), sunt un tip special de fișier utilizat pentru a comunica între 
procese în sisteme de operare UNIX și Linux. Ele permit schimbul de date între două procese care rulează în același sistem 
și sunt un mecanism de comunicare între procese (IPC - Inter-Process Communication).

   Caracteristici principale ale named pipes
->Persistență în sistemul de fișiere: Named pipes sunt create ca fișiere speciale în sistemul de fișiere și sunt 
identificate printr-un nume (de unde și denumirea "named"). Spre deosebire de pipe-urile anonime (cele create fără nume, 
doar cu pipe()), named pipes există ca fișiere și pot fi accesate de mai multe procese după numele lor.
->Unidirecționale: Deși datele într-o named pipe sunt transferate în mod FIFO, named pipes sunt, în general, unidirecționale. Un proces scrie date în pipe, iar altul le citește. Totuși, se pot crea două named pipes pentru a permite comunicația bidirecțională.
->Comunicație sincronizată: Named pipes ajută la sincronizarea între procese. Dacă un proces încearcă să citească dintr-o 
named pipe goală, el va aștepta până când alt proces scrie în aceasta (și invers, un proces care scrie va aștepta dacă nu 
există un cititor).

Crearea și utilizarea unei named pipe
În Linux, o named pipe poate fi creată în linia de comandă sau dintr-un program C folosind funcția mkfifo.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int main() 
{
    const char *fifo_path = "/tmp/my_named_pipe";

    // Crearea named pipe (FIFO)
    if (mkfifo(fifo_path, 0666) == -1) {
        perror("Eroare la crearea FIFO");
        exit(1);
    }

    // Scriere într-o named pipe (într-un proces separat sau după o bifurcare)
    int fd_write = open(fifo_path, O_WRONLY);
    if (fd_write == -1) 
    {
        perror("Eroare la deschiderea pentru scriere");
        exit(1);
    }
  
    const char *message = "Salut din named pipe!";
    write(fd_write, message, sizeof(message));
    close(fd_write);

    // Citire din named pipe (în alt proces)
    int fd_read = open(fifo_path, O_RDONLY);
    if (fd_read == -1) 
    {
        perror("Eroare la deschiderea pentru citire");
        exit(1);
    }
  
    char buffer[100];
    read(fd_read, buffer, sizeof(buffer));
    printf("Mesaj citit: %s\n", buffer);
    close(fd_read);

    // Eliminarea named pipe după utilizare
    unlink(fifo_path);

    return 0;
}

/*
Explicație a codului
   Crearea pipe-ului: mkfifo(fifo_path, 0666); creează un fișier special, my_named_pipe, în /tmp cu permisiuni 
de citire și scriere.
Scrierea într-un named pipe: Procesul deschide pipe-ul cu open(fifo_path, O_WRONLY); și scrie un mesaj.
Citirea din named pipe: Un alt proces sau același proces, deschide pipe-ul cu open(fifo_path, O_RDONLY); și citește datele.
Eliminarea pipe-ului: unlink(fifo_path); șterge fișierul special după utilizare.
Avantaje și utilizări
Separarea proceselor: Named pipes permit două procese independente să comunice.
Comunicare simplă: Sunt mai simple de utilizat decât alte mecanisme de IPC precum sockets sau shared memory.
Folosit în scripturi și aplicații: Named pipes sunt frecvent utilizate în scripturi de shell și în aplicațiile care
necesită o comunicație între procese.
În concluzie, named pipes sunt un mecanism simplu și eficient pentru IPC între procese, permițând trimiterea și primirea
de date printr-un canal FIFO.







*/
