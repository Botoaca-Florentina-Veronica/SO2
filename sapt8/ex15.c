/*
  Împarte un sir de caractere în segmente egale, fiecare fir calculează suma 
segmentului său, iar programul principal calculează suma totală.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

typedef struct {
    char *segment;
    int length;
} ThreadData;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int total_count = 0;

void *handle_function(void *args) 
{
    ThreadData *data = (ThreadData *)args;
    int local_count = data->length;
    
    pthread_mutex_lock(&mutex);
    total_count = total_count + local_count;
    pthread_mutex_unlock(&mutex);
    
    free(data->segment);
    free(data);
    return NULL;
}

int main(int argc, char **argv) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Numar incorect de argumente! Folosire: %s <numar_threaduri>\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[1]); // Convertim argumentul în număr de threaduri
    if (N <= 0) 
    {
        fprintf(stderr, "Numarul de threaduri trebuie sa fie pozitiv!\n");
        exit(1);
    }

    //ne retinem in buffer sirul de caractere introdus de cititor
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) 
    {
        perror("Eroare la citire!");
        exit(1);
    }
    buffer[bytes_read] = '\0'; // adaugam terminatorul de sir

    pthread_t threads[N];
    int segment_size = bytes_read / N;
    int i;

    for (i = 0; i < N; i++) 
    {
        ThreadData *data = malloc(sizeof(ThreadData));
        if (!data) 
        {
            perror("Eroare la alocarea dinamica!");
            exit(1);
        }
        
        if (i == N - 1) 
        {
            data->length = bytes_read - i * segment_size;
        } 
        else 
        {
            data->length = segment_size;
        }
        data->segment = strndup(buffer + i * segment_size, data->length);
        
        if (pthread_create(&threads[i], NULL, handle_function, data) != 0) 
        {
            perror("Eroare la pthread_create!");
            exit(1);
        }
    }

    //asteptam sa se finalizeze toate thread-urile
    for (i = 0; i < N; i++) 
    {
        pthread_join(threads[i], NULL);
    }

    printf("Numarul total de caractere este: %d\n", total_count-1);
    pthread_mutex_destroy(&mutex);

    return 0;
}
