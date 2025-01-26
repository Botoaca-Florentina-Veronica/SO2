/*
Ai un fisier care contine calea catre alte fisiere, si trebuie pentru fiecare fisier sa faci un thread si
sa cauti in el de cate ori apar cifre in el. La final sa afisezi numarul total de de aparitii ale cifrelor 
in toate fisierele
*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX_PATH_LENGTH 256

typedef struct {
    char filePath[MAX_PATH_LENGTH];
    int count; // Contor pentru aparițiile cifrelor în acest fișier
} ThreadData;

pthread_mutex_t mutex; // Mutex pentru protejarea accesului la contorul total

int total_count = 0; // Contor global pentru aparițiile cifrelor

void *count_digits(void *arg) 
{
    ThreadData *data = (ThreadData *)arg;
    FILE *file = fopen(data->filePath, "r");
    if (!file) 
    {
        fprintf(stderr, "Eroare la deschiderea fisierului %s\n", data->filePath);
        free(data);
        return NULL;
    }

    char ch;
    int local_count = 0;

    // Citim fiecare caracter din fișier
    while ((ch = fgetc(file)) != EOF) 
    {
        if (ch >= '0' && ch <= '9') 
        {
            local_count++; // Incrementăm contorul pentru cifre
        }
    }

    fclose(file);

    // Actualizăm contorul total în siguranță
    pthread_mutex_lock(&mutex);
    total_count += local_count;
    pthread_mutex_unlock(&mutex);

    data->count = local_count; // Setăm contorul local în structura thread-ului
    free(data); // Eliberăm resursele
    return NULL;
}

int main(int argc, char **argv) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Utilizare: %s <fisier_cu_caii>\n", argv[0]);
        return 1;
    }

    FILE *fileList = fopen(argv[1], "r");
    if (!fileList) 
    {
        fprintf(stderr, "Eroare la deschiderea fisierului cu caii.\n");
        return 1;
    }

    pthread_mutex_init(&mutex, NULL); // Inițializăm mutexul

    pthread_t thread;
    while (!feof(fileList)) 
    {
        ThreadData *data = malloc(sizeof(ThreadData));
        if (!data) 
        {
            fprintf(stderr, "Eroare la alocarea memoriei pentru ThreadData.\n");
            fclose(fileList);
            return 1;
        }

        // Citim calea fișierului
        if (fgets(data->filePath, MAX_PATH_LENGTH, fileList) != NULL) 
        {
            // Eliminăm caracterul de newline
            data->filePath[strcspn(data->filePath, "\n")] = 0;

            // Creăm un thread pentru a număra cifrele în fișier
            if (pthread_create(&thread, NULL, count_digits, data) != 0) 
            {
                fprintf(stderr, "Eroare la crearea thread-ului.\n");
                free(data);
                fclose(fileList);
                return 1;
            }

            // Așteptăm ca thread-ul să termine
            pthread_join(thread, NULL);
        }
    }

    fclose(fileList);

    // Afișăm numărul total de apariții ale cifrelor
    printf("Numarul total de aparitii ale cifrelor: %d\n", total_count);

    pthread_mutex_destroy(&mutex); // Distrugem mutexul
    return 0;
}
