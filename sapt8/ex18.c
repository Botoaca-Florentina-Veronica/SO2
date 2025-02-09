/* 
  Scrie o problema in C in care ai construiesti 2 threaduri, ambele sa genereze numere mai
mari decat 30000 si sa le sorteze, si intr-o variabila comuna pe care o blochezi cu mutex
sa se incrementeze numarul de interschimbari comun de la cele 2 threaduri. Seteaza apoi
fiecare thread pe un cpu diferit. 
*/



#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

#define SIZE 1000  // Dimensiunea vectorului
#define MIN_VALUE 30000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int swap_count = 0;  // Variabilă globală pentru numărul total de interschimbări

// Funcție pentru generarea unui vector cu numere > 30000
void generate_random_numbers(int *arr, int size) 
{
    for (int i = 0; i < size; i++) 
    {
        arr[i] = MIN_VALUE + rand() % 10000;
    }
}

// Funcție Bubble Sort modificată pentru a returna numărul de interschimbări
void BubbleSort(int *v, int n, int *swap_counter)
{
    int s, i, aux;
    *swap_counter = 0; // Inițializăm contorul

    do {
        s = 0;  
        for (i = 1; i < n; i++) 
        {      
            if (v[i - 1] > v[i]) 
            {             
                aux = v[i - 1];                 
                v[i - 1] = v[i];
                v[i] = aux;
                s = 1;  
                (*swap_counter)++;  // Incrementăm contorul de interschimbări
            }
        }
    } while (s);
}

// Funcția executată de fiecare thread
void *sort_thread(void *arg) 
{
    int *arr = (int *)malloc(SIZE * sizeof(int));
    if (!arr) 
    {
        perror("Eroare alocare memorie");
        pthread_exit(NULL);
    }

    generate_random_numbers(arr, SIZE);  // Generare numere

    int local_swaps;
    BubbleSort(arr, SIZE, &local_swaps);  // Sortare și contorizare interschimbări

    // Actualizare sincronizată a contorului global
    pthread_mutex_lock(&mutex);
    swap_count += local_swaps;
    pthread_mutex_unlock(&mutex);

    free(arr);
    pthread_exit(NULL);
}

// Setează thread-ul pe un CPU specific
void set_thread_affinity(pthread_t thread, int cpu_id) 
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}

int main(void) 
{
    pthread_t thread1, thread2;
    srand(time(NULL));

    // Crearea thread-urilor
    pthread_create(&thread1, NULL, sort_thread, NULL);
    pthread_create(&thread2, NULL, sort_thread, NULL);

    // Setarea fiecărui thread pe un CPU diferit
    set_thread_affinity(thread1, 0);
    set_thread_affinity(thread2, 1);

    // Așteptarea finalizării thread-urilor
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Numărul total de interschimbări: %d\n", swap_count);

    pthread_mutex_destroy(&mutex);
    return 0;
}
