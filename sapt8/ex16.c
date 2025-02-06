//folosind thread-uri, scrie un program in C care primeste ca argument in linia de 
//comanda un numar N de thread-uri si calea unui fisier. Programul trebuie sa imparta
//continutul fisierului in N parti si fiecare thread sa citeasca si sa verifice daca
//in partea lui apare litera 'e' sau 'E'. Apoi totalul din intreg fisierul va fi afisat 
//pe ecran

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/stat.h>  //pentru lstat
#include <fcntl.h>

#define BUFFER_SIZE 1024

typedef struct {
    char *segment;
    int length;
} ThreadData;

pthread_mutex_t mutex;
int total_count = 0;

void *handle_function(void *args) 
{
    ThreadData *data = (ThreadData *)args;
    int local_count = 0;
    int i;
    
    for (i = 0; i < data->length; i++) 
    {
        if (data->segment[i] == 'e' || data->segment[i] == 'E') 
        {
            local_count++;
        }
    }
    
    pthread_mutex_lock(&mutex);
    total_count += local_count;
    pthread_mutex_unlock(&mutex);
    
    free(data->segment);
    free(data);
    return NULL;
}

int main(int argc, char **argv) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Numar incorect de argumente! Folosire: %s <numar_threaduri> <cale_fisier>\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[1]);  //N reprezinta numarul de threaduri pe care trebuie sa il folosim
    if (N <= 0) 
    {
        fprintf(stderr, "Numarul de threaduri trebuie sa fie pozitiv!\n");
        exit(1);
    }
    const char *filePath = argv[2];

    int fd = open(filePath, O_RDONLY);
    if (fd == -1) 
    {
        perror("Eroare la deschiderea fisierului!");
        exit(1);
    }

     
    //obtinem dimensiunea fisierului
    struct stat st;
    if (fstat(fd, &st) != 0) 
    {
        perror("Eroare la fstat!");
        close(fd);
        exit(1);
    }
    size_t fileSize = st.st_size;

    char *buffer = malloc(fileSize + 1);
    if (!buffer) 
    {
        perror("Eroare la alocarea memoriei pentru buffer!");
        close(fd);
        exit(1);
    }
    
    if (read(fd, buffer, fileSize) != fileSize) 
    {
        perror("Eroare la citirea fisierului!");
        free(buffer);
        close(fd);
        exit(1);
    }
    buffer[fileSize] = '\0';
    close(fd);

    pthread_t threads[N];
    int segment_size = fileSize / N;
    int i;

    pthread_mutex_init(&mutex, NULL);
    for (i = 0; i < N; i++) 
    {
        ThreadData *data = malloc(sizeof(ThreadData));
        if (!data) 
        {
            perror("Eroare la alocarea dinamica!");
            exit(1);
        }

        //daca ma aflu la ultimul thread, atunci lui ii voi atribui restul de continut
        //ramas in cazul unei impartiri inegale de threaduri
        if (i == N - 1) 
        {
            data->length = fileSize - i * segment_size;
        } 
        else 
        {
            data->length = segment_size;
        }
        data->segment = malloc(data->length + 1);
        if (!data->segment) 
        {
            perror("Eroare la alocarea dinamica pentru segment!");
            exit(1);
        }
        memcpy(data->segment, buffer + i * segment_size, data->length);
        data->segment[data->length] = '\0';
        
        if (pthread_create(&threads[i], NULL, handle_function, data) != 0) 
        {
            perror("Eroare la pthread_create!");
            exit(1);
        }
    }
    free(buffer);

    for (i = 0; i < N; i++) 
    {
        pthread_join(threads[i], NULL);
    }

    printf("Numarul total de aparitii ale caracterelor 'e' si 'E' este: %d\n", total_count);
    pthread_mutex_destroy(&mutex);

    return 0;
}
