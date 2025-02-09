/*
 Se dau doua fisiere in linie de comanda si trebuia sa creez doua thread uri 
fiecare sa proceseze cate un fisier...si sa numere cati octeti de 0 sunt in 
ambele(era variabila comuna...trebuia mutex)
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024  // Dimensiunea bufferului de citire

pthread_mutex_t mutex;
int total_count = 0;  // Variabilă globală pentru numărarea octeților de 0

// Funcția executată de fiecare thread
void *handle_function(void *arg) 
{
    char *file_path = (char *)arg;
    
    int fd = open(file_path, O_RDONLY);  // Deschidem fișierul pentru citire
    if (fd == -1) 
    {
        perror("Eroare la deschiderea fișierului");
        pthread_exit(NULL);
    }

    unsigned char buffer[BUFFER_SIZE];
    int local_count = 0;
    ssize_t bytes_read;

    // Citim fișierul și numărăm octeții de 0
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) 
    {
        for (ssize_t i = 0; i < bytes_read; i++) 
        {
            if (buffer[i] == 0x00) 
            {
                local_count++;
            }
        }
    }

    close(fd);  // Închidem fișierul

    // Actualizăm contorul global într-o secțiune critică
    pthread_mutex_lock(&mutex);
    total_count += local_count;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main(int argc, char **argv) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Utilizare: %s fisier1 fisier2\n", argv[0]);
        exit(1);
    }

    pthread_t thread1, thread2;
    pthread_mutex_init(&mutex, NULL);

    // Creăm thread-urile și le trimitem calea fișierelor
    pthread_create(&thread1, NULL, handle_function, (void *)argv[1]);
    pthread_create(&thread2, NULL, handle_function, (void *)argv[2]);

    // Așteptăm finalizarea thread-urilor
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Numărul total de octeți de 0 în ambele fișiere: %d\n", total_count);

    pthread_mutex_destroy(&mutex);
    return 0;
}
