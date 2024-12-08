/*
   Sa se scrie un program care primeste ca si argument un fisier text si un numar N reprezentat un numar de thread-uri. 
Programul va imparti fisierul in N partitii egale si pentru fiecare partitie va lansa un thread care va realiza histograma 
caracterelor din fisier. Thread-urile vor completa rezultatul intr-un tablou comun de histrograma. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NUM_CHARS 256  // Numărul total de caractere ASCII

// Histogramă globală
int global_histogram[NUM_CHARS] = {0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Structură pentru parametrii fiecărui thread
typedef struct {
    FILE *file;
    int start;
    int end;
} ThreadArg;

// Funcția rulată de fiecare thread pentru a calcula histograma
void* create_histogram(void *arg) 
{
    ThreadArg *threadArg = (ThreadArg*)arg;
    FILE *file = threadArg->file;
    fseek(file, threadArg->start, SEEK_SET);

    int local_histogram[NUM_CHARS] = {0};
    char c;
    int position = threadArg->start;

    while (position < threadArg->end && fread(&c, 1, 1, file) == 1) 
    {
        local_histogram[(unsigned char)c]++;
        position++;
    }

    pthread_mutex_lock(&mutex);
    int i;
    for (i = 0; i < NUM_CHARS; i++) 
    {
        global_histogram[i] += local_histogram[i];
    }
    pthread_mutex_unlock(&mutex);

    free(threadArg);
    return NULL;
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Utilizare: %s <fisier_text> <numar_threaduri>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *filename = argv[1];
    int num_threads = atoi(argv[2]);
    if (num_threads <= 0) 
    {
        fprintf(stderr, "Numărul de thread-uri trebuie să fie pozitiv.\n");
        return EXIT_FAILURE;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) 
    {
        perror("Eroare deschidere fișier");
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);

    int partition_size = file_size / num_threads;
    pthread_t threads[num_threads];

    int i;
    for (i = 0; i < num_threads; i++) 
    {
        ThreadArg *arg = malloc(sizeof(ThreadArg));
        arg->file = file;
        arg->start = i * partition_size;
        if (i == num_threads - 1) 
        {
            arg->end = file_size;
        } 
        else 
        {
            arg->end = (i + 1) * partition_size;
        }

        pthread_create(&threads[i], NULL, create_histogram, arg);
    }

    for (i = 0; i < num_threads; i++) 
    {
        pthread_join(threads[i], NULL);
    }

    fclose(file);
    pthread_mutex_destroy(&mutex);

    printf("Histograma finală:\n");
    for (i = 0; i < NUM_CHARS; i++) 
    {
        if (global_histogram[i] > 0)
        {
            printf("%c: %d\n", (i >= 32 && i <= 126) ? i : '.', global_histogram[i]);
        }
    }

    return EXIT_SUCCESS;
}
