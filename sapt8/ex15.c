/* Scrie un program in C, in care acesta sa primeasca calea unui fisier binar ca argument, un numar N.
Programul in main, va citi si va imparti continutul fisierului in N threaduri, iar fiecare thread va calcula
suma caracterelor din el. Apoi aceste sume vor fi adaugate la o variabila comuna si printata pe terminal.
Foloseste mutex pentru aceasta suma totala.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

#define BUFFER_SIZE 1024

// Structura pentru a transmite argumente thread-urilor
typedef struct {
    unsigned char *buffer;
    size_t size;
} ThreadArgs;

pthread_mutex_t mutex;
uint64_t total_sum = 0;

void *compute_sum(void *args) 
{
    ThreadArgs *data = (ThreadArgs *)args;
    uint64_t local_sum = 0;
    
    for (size_t i = 0; i < data->size; i++) 
    {
        local_sum = local_sum + data->buffer[i];
    }
    
    pthread_mutex_lock(&mutex);
    total_sum = total_sum + local_sum;
    pthread_mutex_unlock(&mutex);
    
    free(data->buffer);
    free(data);
    
    return NULL;
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Utilizare: %s <cale_fisier> <numar_threaduri>\n", argv[0]);
        return 1;
    }
    
    char *filename = argv[1];
    int num_threads = atoi(argv[2]);
    
    if (num_threads <= 0) 
    {
        fprintf(stderr, "Numarul de thread-uri trebuie sa fie pozitiv!\n");
        return 1;
    }
    
    FILE *file = fopen(filename, "rb");
    if (!file) 
    {
        perror("Eroare la deschiderea fisierului");
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);
    
    size_t chunk_size = file_size / num_threads;
    size_t remaining_bytes = file_size % num_threads;
    
    pthread_t threads[num_threads];
    pthread_mutex_init(&mutex, NULL);
    
    int i;
    for (i = 0; i < num_threads; i++) 
    {
        size_t current_chunk_size = chunk_size;  
        if (i == num_threads - 1) 
        {  
            current_chunk_size += remaining_bytes;  
        }

        unsigned char *buffer = malloc(current_chunk_size);
        if (!buffer) 
        {
            perror("Eroare la alocarea bufferului");
            fclose(file);
            return 1;
        }
        
        fread(buffer, 1, current_chunk_size, file);
        
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        if (!args) 
        {
            perror("Eroare la alocarea argumentelor threadului");
            free(buffer);
            fclose(file);
            return 1;
        }
        
        args->buffer = buffer;
        args->size = current_chunk_size;
        
        if (pthread_create(&threads[i], NULL, compute_sum, args) != 0) 
        {
            perror("Eroare la crearea threadului");
            free(buffer);
            free(args);
            fclose(file);
            return 1;
        }
    }
    
    fclose(file);
    
    for (i = 0; i < num_threads; i++) 
    {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&mutex);
    printf("Suma totala a caracterelor din fisier: %lu\n", total_sum);
    
    return 0;
}
