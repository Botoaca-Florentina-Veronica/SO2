/*
  Acelasi enunt ca si la problema 4 cu modificarea ca in momentul in care un thread va identifica un numar
prim din subinterval aceasta il va adauga intr-un tablou comun tuturor thread-urile. Cand toate thread-urile
au terminat calculul programul principal va afisa la sfarsit tabloul cu toate numerele prime gasite.
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

// Tabloul comun și variabile globale
int *primes = NULL;   // Tablou dinamic pentru numerele prime
int prime_count = 0;  // Numărul de numere prime găsite
pthread_mutex_t lock; // Mutex pentru sincronizare

// Funcție pentru a verifica dacă un număr este prim
int is_prime(int num) 
{
    if (num < 2)
    {
        return 0;
    }
    int i;
    for (i = 2; i <= sqrt(num); i++) 
    {
        if (num % i == 0)
        {
            return 0;
        }
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
            pthread_mutex_lock(&lock); // Blocăm accesul la tabloul comun
            primes = realloc(primes, (prime_count + 1) * sizeof(int));
            if (primes == NULL) 
            {
                perror("Failed to allocate memory");
                exit(1);
            }
            primes[prime_count++] = i; // Adăugăm numărul prim
            pthread_mutex_unlock(&lock); // Deblocăm accesul
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
    pthread_mutex_init(&lock, NULL); // Inițializăm mutex-ul
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

    pthread_mutex_destroy(&lock); // Distrugem mutex-ul

    // Afișăm tabloul de numere prime
    printf("Prime numbers in the interval [%d, %d]:\n", A, B);
    for (i = 0; i < prime_count; i++) 
    {
        printf("%d ", primes[i]);
    }
    printf("\n");

    free(primes); // Eliberăm memoria alocată pentru tabloul de numere prime
    return 0;
}

/*

gcc -Wall -o vera vera.c -pthread -lm
vera@DESKTOP-0GQBRQR:~$ ./vera 10 50 4
Prime numbers in the interval [10, 50]:
11 13 17 19 23 29 31 37 41 43 47 
*/
