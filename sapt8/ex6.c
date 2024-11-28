#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

#define NUM_CHARS 256  // Numărul total de caractere ASCII

// Histogramă globală
int global_histogram[NUM_CHARS] = {0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Structură pentru parametrii unui thread
typedef struct {
    char *filepath;
} ThreadArg;

// Funcția de histograme pe fișier
void* create_histogram(void *arg) 
{
    ThreadArg *threadArg = (ThreadArg*)arg;
    FILE *file = fopen(threadArg->filepath, "r");
    if (!file) 
    {
        perror("Eroare deschidere fișier");
        free(threadArg->filepath);
        free(threadArg);
        pthread_exit(NULL);
    }

    int local_histogram[NUM_CHARS] = {0};
    int c;
    while ((c = fgetc(file)) != EOF) 
    {
        if (c >= 0 && c < NUM_CHARS) 
        {
            local_histogram[c]++;
        }
    }
    fclose(file);

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < NUM_CHARS; i++) 
    {
        global_histogram[i] += local_histogram[i];
    }
    pthread_mutex_unlock(&mutex);

    free(threadArg->filepath);
    free(threadArg);
    pthread_exit(NULL);
}

// Funcție recursivă pentru a scana directorul
void scan_directory(const char *dirpath) 
{
    DIR *dir = opendir(dirpath);
    if (!dir) 
    {
        perror("Eroare deschidere director");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
        {
            continue;
        }

        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);

        struct stat statbuf;
        if (stat(fullpath, &statbuf) == -1) 
        {
            perror("Eroare stat");
            continue;
        }

        if (S_ISREG(statbuf.st_mode)) 
        {
            ThreadArg *arg = malloc(sizeof(ThreadArg));
            arg->filepath = strdup(fullpath);
            pthread_t tid;
            pthread_create(&tid, NULL, create_histogram, arg);
            pthread_detach(tid);
        } 
        else if (S_ISDIR(statbuf.st_mode)) 
        {
            scan_directory(fullpath);
        }
    }
    closedir(dir);
}

// Funcție pentru scrierea histogramei în fișierul de ieșire
void write_histogram_to_file(const char *output_file) 
{
    FILE *file = fopen(output_file, "w");
    if (!file) 
    {
        perror("Eroare deschidere fișier de ieșire");
        return;
    }

    for (int i = 0; i < NUM_CHARS; i++)
    {
        if (global_histogram[i] > 0) 
        {
            fprintf(file, "%c: %d\n", (isprint(i) ? i : '.'), global_histogram[i]);
        }
    }
    fclose(file);
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Utilizare: %s <director> <fișier ieșire>\n", argv[0]);
        return EXIT_FAILURE;
    }

    scan_directory(argv[1]);
    sleep(1);  // Așteaptă finalizarea thread-urilor
    write_histogram_to_file(argv[2]);

    pthread_mutex_destroy(&mutex);
    return EXIT_SUCCESS;
}
