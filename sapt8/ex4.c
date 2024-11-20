/*
 Sa se scrie un program care primeste ca si argumente doua numere ce reprezinta captele unui interval
A si B si un alt treilea numar N ce va reprezeinta un numar de thread-uri pe care programul le va crea. 
Programul va imparti intervalul [A, B] in N intervale egale. Apoi programul va crea N threaduri. 
Fiecare thread va procesa cate un subinterval din intervalul [A,B] si va identifica numere prime din 
subinerval. In momentul in care un thread va identifica cate un numar prim il va printa la iesirea standard.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

// Structura pentru a stoca datele pentru fiecare thread
typedef struct {
    int start;
    int end;
} ThreadData;

// Funcție pentru a verifica dacă un număr este prim
int is_prime(int num) 
{
    if (num < 2) return 0;
    for (int i = 2; i <= sqrt(num); i++) 
    {
        if (num % i == 0) return 0;
    }
    return 1;
}

// Funcția executată de fiecare thread
void *find_primes(void *arg) 
{
    ThreadData *data = (ThreadData *)arg;
    int start = data->start;
    int end = data->end;

    int i;
    for (i = start; i <= end; i++) 
    {
        if (is_prime(i)) 
        {
            printf("Thread [%d - %d]: %d\n", start, end, i);
        }
    }

    free(data); // Eliberăm memoria alocată pentru datele thread-ului
    return NULL;
}

int main(int argc, char *argv[]) 
{
    if (argc != 4) 
    {
        fprintf(stderr, "Usage: %s <A> <B> <N>\n", argv[0]);
        return 1;
    }

    // Citim argumentele din linia de comandă
    int A = atoi(argv[1]);
    int B = atoi(argv[2]);
    int N = atoi(argv[3]);

    if (A > B || N <= 0) 
    {
        fprintf(stderr, "Invalid input: Ensure A <= B and N > 0.\n");
        return 1;
    }

    pthread_t threads[N];
    int interval_length = (B - A + 1) / N;
    int remainder = (B - A + 1) % N;

    // Creăm și lansăm thread-urile
    int current_start = A;
    int i;
    for (i = 0; i < N; i++) 
    {
        ThreadData *data = malloc(sizeof(ThreadData));
        if (data == NULL) 
        {
            perror("Failed to allocate memory");
            return 1;
        }

        // Împărțim intervalul
        data->start = current_start;
        data->end = current_start + interval_length - 1;

        // Adăugăm restul la ultimul thread
        if (i == N - 1) 
        {
            data->end += remainder;
        }

        current_start = data->end + 1;

        // Creăm thread-ul
        if (pthread_create(&threads[i], NULL, find_primes, data) != 0) 
        {
            perror("Failed to create thread");
            free(data);
            return 1;
        }
    }

    // Așteptăm finalizarea tuturor thread-urilor
    for (i = 0; i < N; i++) 
    {
        pthread_join(threads[i], NULL);
    }
    return 0;
}


/*

gcc -Wall -o vera vera.c -pthread -lm
vera@DESKTOP-0GQBRQR:~$ ./vera 10 50 4
Thread [10 - 19]: 11
Thread [10 - 19]: 13
Thread [20 - 29]: 23
Thread [20 - 29]: 29
Thread [10 - 19]: 17
Thread [10 - 19]: 19
Thread [30 - 39]: 31
Thread [30 - 39]: 37
Thread [40 - 50]: 41
Thread [40 - 50]: 43
Thread [40 - 50]: 47

*/
