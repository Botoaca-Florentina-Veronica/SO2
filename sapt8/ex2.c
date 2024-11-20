/*
   Sa se scrie un program in C care primeste ca si argument un caracter si citeste cate o linie de la 
intrarea standard. Pentru fiecare linie citita programul va crea un thread care va numara de cate ori 
caracterul dat ca si argument se regaseste in linia citita si va printa acest numar la iesirea standard.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct {
    char *line;
    char target;
} ThreadData;

// Funcția care va fi rulată de fiecare thread
void *count_character(void *arg) 
{
    ThreadData *data = (ThreadData *)arg;
    char *line = data->line;
    char target = data->target;

    int count = 0;
    int i;
    for (i = 0; line[i] != '\0'; i++) 
    {
        if (line[i] == target) 
        {
            count++;
        }
    }

    printf("Line: %sCount of '%c': %d\n", line, target, count);

    free(data->line);
    free(data);

    return NULL;
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        printf("Numar incorect de argumente in linia de comanda!!");
        return 1;
    }

    char target = argv[1][0];  //preluam primul caracter din al doilea argument al liniei de comandă (argv[1])
    char buffer[1024];
    pthread_t threads[1024];
    int thread_count = 0;

    // Citirea liniilor de la intrarea standard
    while (fgets(buffer, sizeof(buffer), stdin)) 
    {
        // Alocăm memorie pentru linia curentă
        char *line = strdup(buffer);
        if (line == NULL) 
        {
            perror("Failed to allocate memory");
            exit(1);
        }

        // Eliminăm caracterul de newline, dacă există
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') 
        {
            line[len - 1] = '\0';
        }

        // Pregătim datele pentru thread
        ThreadData *data = malloc(sizeof(ThreadData));
        if (data == NULL) 
        {
            perror("Failed to allocate memory");
            free(line);
            exit(1);
        }
        data->line = line;
        data->target = target;

        // Creăm thread-ul
        if (pthread_create(&threads[thread_count], NULL, count_character, data) != 0) 
        {
            perror("Failed to create thread");
            free(line);
            free(data);
            exit(1);
        }

        thread_count++;
    }

    // Așteptăm toate thread-urile să se termine
    int i;
    for (i = 0; i < thread_count; i++) 
    {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
