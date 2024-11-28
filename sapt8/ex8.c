/*
    Să se scrie un program care primește ca și argument un fisier text si un numar N reprezentand un numar de threaduri. 
 Programul va citi din fisier cate un buffer de dimensiunea CHUNK (configurabila printr-un define) si il va da spre procesare 
 cate unui thread. Daca nu mai sunt thread-uri disponibile din cele N programul va astepta pana cand exista vreun thread 
 disponibil pentru a prelucra urmatori buffer disponibil. Toate thread-urile vor completa rezultatul intr-un tablou comun de 
 histograma. Este necesar ca programul sa tina evidenta starii de executie si de join a fiecarui thread. Numarul N nu va fi 
 ales ca si la problema 7 astfel incat fisierul sa fie impartit in partitii egale. Se va considera ca exista un pool de N 
 thread-uri care se va ocupa de procesare. Daca nu exista thread-uri disponibile din pool (toate sunt ocupate cu procesarea 
 unui buffer) atunci programul va astepta eliberarea unui thread. Se poate utiliza o functia care sa obtina care thread este 
 liber din pool-ul de thread-uri. Se va tine evidenta starii de join si se va face join pe fiecare thread care isi termina 
 executia
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define NUM_CHARS 256  // Numărul de caractere ASCII
#define CHUNK 1024     // Dimensiunea bufferului

// Histogramă globală
int global_histogram[NUM_CHARS] = {0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Structură pentru argumentele fiecărui thread
typedef struct {
    char buffer[CHUNK];
    size_t length;
} ThreadArg;

// Structura pentru gestiunea pool-ului de thread-uri
typedef struct {
    pthread_t thread;
    int available;
} ThreadPool;

// Funcția rulată de fiecare thread pentru a calcula histograma
void* process_chunk(void* arg) 
{
    ThreadArg* threadArg = (ThreadArg*)arg;
    int local_histogram[NUM_CHARS] = {0};

    for (size_t i = 0; i < threadArg->length; i++) 
    {
        unsigned char c = (unsigned char)threadArg->buffer[i];
        local_histogram[c]++;
    }

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < NUM_CHARS; i++) 
    {
        global_histogram[i] += local_histogram[i];
    }
    pthread_mutex_unlock(&mutex);

    free(threadArg);
    pthread_exit(NULL);
}

// Găsește un thread disponibil în pool
int find_available_thread(ThreadPool* pool, int num_threads) 
{
    for (int i = 0; i < num_threads; i++) 
    {
        if (pool[i].available) 
        {
            return i;
        }
    }
    return -1;
}

int main(int argc, char* argv[]) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Utilizare: %s <fisier_text> <numar_threaduri>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* filename = argv[1];
    int num_threads = atoi(argv[2]);
    if (num_threads <= 0) 
    {
        fprintf(stderr, "Numărul de thread-uri trebuie să fie pozitiv.\n");
        return EXIT_FAILURE;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) 
    {
        perror("Eroare deschidere fișier");
        return EXIT_FAILURE;
    }

    ThreadPool* pool = malloc(num_threads * sizeof(ThreadPool));
    for (int i = 0; i < num_threads; i++) 
    {
        pool[i].available = 1;
    }

    while (1) {
        ThreadArg* arg = malloc(sizeof(ThreadArg));
        arg->length = fread(arg->buffer, 1, CHUNK, file);

        if (arg->length == 0) 
        {
            free(arg);
            break;
        }

        int thread_index = -1;
        while ((thread_index = find_available_thread(pool, num_threads)) == -1) 
        {
            usleep(1000);  // Așteaptă puțin înainte de a verifica din nou
        }

        pool[thread_index].available = 0;
        pthread_create(&pool[thread_index].thread, NULL, process_chunk, arg);
    }

    for (int i = 0; i < num_threads; i++) 
    {
        if (!pool[i].available) 
        {
            pthread_join(pool[i].thread, NULL);
            pool[i].available = 1;
        }
    }

    fclose(file);
    pthread_mutex_destroy(&mutex);
    free(pool);

    printf("Histograma finală:\n");
    for (int i = 0; i < NUM_CHARS; i++) 
    {
        if (global_histogram[i] > 0) 
        {
            printf("%c: %d\n", (i >= 32 && i <= 126) ? i : '.', global_histogram[i]);
        }
    }

    return EXIT_SUCCESS;
}
