/*
Citeste dintr-un fisier binar, dat de la tastatura folosind n(dat de la tastatura) threaduri folosind 
buffere cu marimea calculata cu lstat si pentru fiecare thread sa bagi intr-un array comun maximul local
si dupa sa printezi maximul global
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h> // pentru lstat
#include <limits.h>   // pentru INT_MIN

typedef struct {
    const char *filePath;
    int thread_id;
    size_t buffer_size;
    int local_max; // Maximul local pentru fiecare thread
} ThreadData;

pthread_mutex_t mutex; // Mutex pentru protejarea accesului la maximul global
int global_max = INT_MIN; // Maximul global

void *find_local_max(void *arg) 
{
    ThreadData *data = (ThreadData *)arg;
    FILE *file = fopen(data->filePath, "rb");
    if (!file) 
    {
        fprintf(stderr, "Eroare la deschiderea fisierului %s\n", data->filePath);
        free(data);
        return NULL;
    }

    // Alocăm bufferul
    void *buffer = malloc(data->buffer_size);
    if (!buffer) 
    {
        fprintf(stderr, "Eroare la alocarea memoriei pentru buffer.\n");
        fclose(file);
        free(data);
        return NULL;
    }

    data->local_max = INT_MIN; // Resetăm maximul local

    // Citim din fișier în bucle
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, data->buffer_size, file)) > 0) 
    {
        // Căutăm maximul în buffer
        ssize_t i;
        for (i = 0; i < bytesRead / sizeof(int); i++) 
        {
            int value = ((int *)buffer)[i];
            if (value > data->local_max) 
            {
                data->local_max = value;
            }
        }
    }

    fclose(file);
    free(buffer);

    // Actualizăm maximul global în siguranță
    pthread_mutex_lock(&mutex);
    if (data->local_max > global_max) 
    {
        global_max = data->local_max;
    }
    pthread_mutex_unlock(&mutex);

    free(data); // Eliberăm resursele
    return NULL;
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Utilizare: %s <cale_fisier> <numar_threaduri>\n", argv[0]);
        return 1;
    }

    const char *filePath = argv[1];
    int n = atoi(argv[2]);

    if (n <= 0) 
    {
        fprintf(stderr, "Numarul de thread-uri trebuie sa fie un numar pozitiv.\n");
        return 1;
    }

    // Obținem dimensiunea fișierului
    struct stat st;
    if (lstat(filePath, &st) != 0) 
    {
        perror("Eroare la obtinerea dimensiunii fisierului");
        return 1;
    }
    size_t file_size = st.st_size;

    // Calculăm dimensiunea bufferului pentru fiecare thread
    size_t buffer_size = file_size / n;
    if (buffer_size == 0) 
    {
        fprintf(stderr, "Dimensiunea bufferului este prea mica.\n");
        return 1;
    }

    pthread_mutex_init(&mutex, NULL); // Inițializăm mutexul

    pthread_t threads[n];

    // Creăm thread-urile
    int i;
    for (i = 0; i < n; i++) 
    {
        ThreadData *data = malloc(sizeof(ThreadData));
        if (!data) 
        {
            fprintf(stderr, "Eroare la alocarea memoriei pentru ThreadData.\n");
            return 1;
        }

        data->filePath = filePath;
        data->thread_id = i;
        data->buffer_size = buffer_size;

        if (pthread_create(&threads[i], NULL, find_local_max, data) != 0) 
        {
            fprintf(stderr, "Eroare la crearea thread-ului %d.\n", i);
            free(data);
            return 1;
        }
    }

    // Așteptăm ca toate thread-urile să termine
    for (i = 0; i < n; i++) 
    {
        pthread_join(threads[i], NULL);
    }

    // Afișăm maximul global
    printf("Maximul global este: %d\n", global_max);

    pthread_mutex_destroy(&mutex); // Distrugem mutexul
    return 0;
}
