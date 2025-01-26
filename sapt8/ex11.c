/*
 Sa se scrie un program care primeste ca si argumente doua numere ce reprezinta captele unui interval
A si B si un alt treilea numar N ce va reprezenta un numar de thread-uri pe care programul le va crea. 
Programul va imparti intervalul [A, B] in N intervale egale. Apoi programul va crea N threaduri. 
Fiecare thread va procesa cate un subinterval din intervalul [A,B] si va identifica numere care sa fie
puteri ale lui 2 din subinerval. In momentul in care un thread va identifica cate un numar prim il va 
printa la iesirea standard.
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<pthread.h>

typedef struct{
    int start;
    int end;
}ThreadData;

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
    ThreadData *data = (ThreadData *) arg;

    int start = data->start;
    int end = data->end;

    int i;
    for(i=start; i<=end; i++)
    {
        if(este_putere_a_lui_2(i))
        {
            printf("Thread [%d - %d]: %d\n", start, end, i);
        }
    }

    free(data);
    return NULL;
}


int main(int argc, char **argv)
{
    if(argc != 4)
    {
        printf("Numar incorect de argumente in linia de comanda!!");
        exit(1);
    }

    int A = atoi(argv[1]);
    int B = atoi(argv[2]);
    int N = atoi(argv[3]);

    int lungime_interval = (B-A+1)/N;
    int rest = (B-A+1)%N;

    pthread_t threads[N];

    int i;
    int current_start = A;
    for(i=0; i<N; i++)
    {
        ThreadData *data = malloc(sizeof(ThreadData));
        if(!data)
        {
            printf("Eroare la alocarea dinamica!!");
            exit(1);
        }

        data->start = current_start;
        data->end = current_start + lungime_interval - 1;

        //adaugam restul la ultimul thread
        if(i == N-1)
        {
            data->end = data->end + rest;
        }
        current_start = data->end + 1;
        
        if(pthread_create(&threads[i], NULL, handle_function, data) != 0)
        {
            printf("Eroare la pthread_create!!");
            exit(1);
        }

    }

    //asteptam pana se finalizeaza toate threadurile
    for(i=0; i<N; i++)
    {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
