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
    // Mesaj pentru thread
    printf("\nThread %d is processing indices [%d, %d):\n", data, start, end);
    for (i = start; i < end; i++) 
    {
        partialSum[data] = partialSum[data] + a[i];
        printf("%d ", i);
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
        totalSum = totalSum + partialSum[i];
    }

    printf("\n\nTotal sum: %d\n", totalSum);
    return 0;
}




//sauuuu
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

#define N 2
#define vectorSize 17

int partialSum[N];
int a[vectorSize];

// Struct pentru a stoca intervalele și identificatorul thread-ului
typedef struct {
    int start;
    int end;
    int threadID;
} Interval;

void *threadFunction(void *arg)
{
    Interval *data = (Interval *)arg;

    int start = data->start;
    int end = data->end;
    int threadID = data->threadID;

    partialSum[threadID] = 0;

    // Mesaj pentru thread
    printf("\nThread %d is processing indices [%d, %d):\n", threadID, start, end);
    int i;
    for (i = start; i < end; i++) 
    {
        partialSum[threadID] = partialSum[threadID] + a[i];
        printf("%d ", i);
    }

    free(data); // Eliberăm memoria alocată pentru struct
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

    int IntervalSize = vectorSize / N;
    int rest = vectorSize % N;

    for (i = 0; i < N; i++)
    {
        // Alocare dinamică pentru fiecare structură ThreadData
        Interval *data = malloc(sizeof(Interval));
        if (data == NULL)
        {
            printf("Eroare la alocarea dinamică!!");
            exit(1);
        }

        // Setăm limitele intervalului și ID-ul thread-ului
        data->start = i * IntervalSize;
        if (i == N - 1) 
        {
            data->end = data->start + IntervalSize + rest;
        } 
        else 
        {
            data->end = data->start + IntervalSize;
        }
        data->threadID = i;
        

        if (pthread_create(&threads[i], NULL, threadFunction, data) != 0)
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
        totalSum = totalSum + partialSum[i];
    }

    printf("\n\nTotal sum: %d\n", totalSum);
    return 0;
}


/*
Pentru a împărți vectorul a în segmente egale și a atribui fiecărui thread un segment, 
fiecare thread trebuie să înceapă de la o poziție diferită în vector, pe baza indexului său. 
Iată de ce:

Numărul total de elemente din vector: vectorSize = 17.
Numărul de thread-uri: N = 2.
Fiecare thread trebuie să proceseze aproximativ jumătate din vector. Cum vectorSize / N calculează 
dimensiunea unui segment (în cazul acesta 17 / 2 = 8 pentru fiecare thread), fiecare thread trebuie să 
înceapă de la o poziție diferită în vector.

Detaliile calculului:
Thread-ul 0 (cu i = 0) trebuie să înceapă la start = 0 * (vectorSize / N) = 0. Deci, va procesa primele 
8 elemente (de la indexul 0 la 7).
Thread-ul 1 (cu i = 1) trebuie să înceapă la start = 1 * (vectorSize / N) = 8. Deci, va procesa ultimele 
9 elemente (de la indexul 8 la 16).
Prin urmare, înmulțirea cu i este necesară pentru a calcula unde începe fiecare thread să proceseze în 
vector, astfel încât fiecare thread să primească un segment corect al vectorului.
*/
