/*
  Împarte un vector în segmente egale, fiecare fir calculează suma segmentului său, iar programul 
principal calculează suma totală.
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

#define N 2
#define vectorSize 18

int partialSum[N];
int a[vectorSize];


void *threadFunction(void *arg)
{
    int data = *(int *)arg;
    free(arg);

    int start, end, i;
    start = data * (vectorSize/N);
    end = start + (vectorSize/N);

    partialSum[data] = 0;
    for(i=start; i<end; i++)
    {
        partialSum[data] = partialSum[data] + a[i];
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[N];
    int i;

    //initializare vector:
    for(i=0; i<vectorSize; i++)
    {
        a[i] = i+1;
    }
    
    for(i=0; i<N; i++)
    {
        int *threadID = malloc(sizeof(int));  //imi aloc dinamic spatiu pentru fiecare identificator
        //am nevoie sa fac asta pentru a putea da ca parametru ulterior in pthread_create adresa unui astfel de element
        //daca nu vreau sa aloc dinamic, pot creea in main un tablou de tip int threadIds[N] 
        if(threadID == NULL)
        {
            printf("Eroare la alocarea dinamica!!");
            exit(1);
        }

        *threadID = i; // Stocăm valoarea identificatorului
        if(pthread_create(&threads[i], NULL, threadFunction, threadID) != 0)
        {
            printf("Eroare la creearea thread-urilor!!");
            exit(EXIT_FAILURE);
        }
    }

    //astept toate thread-urile sa se termine
    for(i=0; i<N; i++)
    {
        pthread_join(threads[i], NULL);
    }

    int totalSum = 0;
    for(i=0; i<N; i++)
    {
        totalSum = totalSum + partialSum[i];
    }

    printf("Total sum: %d", totalSum);
    return 0;
}
