#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

#define N 2
#define vectorSize 17

int partialSum[N];
int a[vectorSize];

void *threadFunction(void *arg)
{
    int data = *(int *)arg;
    free(arg);

    int start, end, i;
    int rest = vectorSize % N;

    start = data * (vectorSize / N);
    end = start + (vectorSize / N);

    // Ultimul thread preia și restul elementelor
    if (data == N - 1) 
    {
        end = end + rest;
    }

    partialSum[data] = 0;
    for (i = start; i < end; i++) 
    {
        partialSum[data] += a[i];
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[N];
    int i;

    // Inițializare vector:
    for (i = 0; i < vectorSize; i++) 
    {
        a[i] = i + 1;
    }
    
    for (i = 0; i < N; i++)
    {
        int *threadID = malloc(sizeof(int));  // Alocare dinamică pentru fiecare identificator
        if (threadID == NULL)
        {
            printf("Eroare la alocarea dinamică!!");
            exit(1);
        }

        *threadID = i; // Stocăm valoarea identificatorului
        if (pthread_create(&threads[i], NULL, threadFunction, threadID) != 0)
        {
            printf("Eroare la crearea thread-urilor!!");
            exit(EXIT_FAILURE);
        }
    }

    // Aștept toate thread-urile să se termine
    for (i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    }

    int totalSum = 0;
    for (i = 0; i < N; i++)
    {
        totalSum += partialSum[i];
    }

    printf("Total sum: %d\n", totalSum);
    return 0;
}
