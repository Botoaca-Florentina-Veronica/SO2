/*
 Sa se scrie un program care primeste ca si argumente doua numere ce reprezinta captele unui interval
A si B si un alt treilea numar N ce va reprezenta un numar de thread-uri pe care programul le va crea. 
Programul va imparti intervalul [A, B] in N intervale egale. Apoi programul va crea N threaduri. 
Fiecare thread va procesa cate un subinterval din intervalul [A,B] si va identifica numere care sa fie
puteri ale lui 2 din subinerval. In momentul in care un thread va identifica un numar putere de 2 din
subinterval aceasta il va adauga intr-un tablou comun tuturor thread-urile. Cand toate thread-urile au 
terminat calculul programul principal va afisa la sfarsit tabloul cu toate numerele prime gasite.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int start;
    int end;
} ThreadData;

int *numere_puteri_2; // Tabloul global pentru numerele care sunt puteri ale lui 2
int numere_puteri_2_count = 0; // Contor pentru numerele care sunt puteri ale lui 2
int capacity = 100; // Capacitate inițială a tabloului
pthread_mutex_t mutex; // Mutex pentru protejarea accesului la tabloul global

int este_putere_a_lui_2(int num) 
{
    // Verificăm dacă numărul este 0 sau negativ
    if (num <= 0) 
    {
        return 0; // Numerele negative și 0 nu sunt puteri ale lui 2
    }

    // Continuăm să împărțim numărul la 2 cât timp este par
    while (num % 2 == 0) 
    {
        num = num / 2; // Împărțim numărul la 2
    }

    // Dacă numărul a ajuns la 1, înseamnă că a fost o putere a lui 2
    if (num == 1) 
    {
        return 1; // Este putere a lui 2
    } 
    else 
    {
        return 0; // Nu este putere a lui 2
    }
}

void *handle_function(void *arg) 
{
    ThreadData *data = (ThreadData *)arg;
    int start = data->start;
    int end = data->end;

    int i;
    for (i = start; i <= end; i++) 
    {
        if (este_putere_a_lui_2(i))
         { 
            // Verificăm dacă numărul este putere a lui 2
            pthread_mutex_lock(&mutex); // Blocăm mutexul
            // Verificăm dacă avem nevoie de mai mult spațiu în tablou
            if (numere_puteri_2_count >= capacity) 
            {
                capacity *= 2; // Dublăm capacitatea
                numere_puteri_2 = realloc(numere_puteri_2, capacity * sizeof(int));
                if (!numere_puteri_2) 
                {
                    fprintf(stderr, "Eroare la realloc pentru numere puteri de 2!!\n");
                    exit(1);
                }
            }
            numere_puteri_2[numere_puteri_2_count++] = i; // Adăugăm numărul putere a lui 2 în tablou
            pthread_mutex_unlock(&mutex); // Deblocăm mutexul
        }
    }

    free(data);
    return NULL;
}

int main(int argc, char **argv) 
{
    if (argc != 4) 
    {
        fprintf(stderr, "Numar incorect de argumente in linia de comanda!!\n");
        exit(1);
    }

    int i;
    int A = atoi(argv[1]);
    int B = atoi(argv[2]);
    int N = atoi(argv[3]);

    if (A > B || N <= 0) 
    {
        fprintf(stderr, "Valori invalide pentru A, B sau N!!\n");
        exit(1);
    }

    numere_puteri_2 = malloc(capacity * sizeof(int)); // Alocăm memorie pentru tabloul de numere puteri de 2
    if (!numere_puteri_2) 
    {
        fprintf(stderr, "Eroare la alocarea memoriei pentru numere puteri de 2!!\n");
        exit(1);
    }

    pthread_mutex_init(&mutex, NULL); // Inițializăm mutexul

    int lungime_interval = (B - A + 1) / N;
    int rest = (B - A + 1) % N;

    pthread_t threads[N];
    int current_start = A;

    for (i = 0; i < N; i++) 
    {
        ThreadData *data = malloc(sizeof(ThreadData));
        if (!data) 
        {
            fprintf(stderr, "Eroare la alocarea dinamica!!\n");
            exit(1);
        }

        data->start = current_start;
        data->end = current_start + lungime_interval - 1;

        if (i == N - 1) 
        {
            data->end = data->end + rest; // Adăugăm restul la ultimul thread
        }
        current_start = data->end + 1;

        if (pthread_create(&threads[i], NULL, handle_function, data) != 0) 
        {
            fprintf(stderr, "Eroare la pthread_create!!\n");
            exit(1);
        }
    }

    // Așteptăm până se finalizează toate thread-urile
    for (i = 0; i < N; i++) 
    {
        pthread_join(threads[i], NULL);
    }

    // Afișăm tabloul cu numerele care sunt puteri ale lui 2 găsite
    printf("Numere puteri de 2 gasite:\n");
    for (i = 0; i < numere_puteri_2_count; i++) 
    {
        printf("%d ", numere_puteri_2[i]);
    }
    printf("\n");

    // Curățăm resursele
    free(numere_puteri_2);
    pthread_mutex_destroy(&mutex);
    return 0;
}
