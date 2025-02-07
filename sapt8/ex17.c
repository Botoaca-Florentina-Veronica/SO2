//scrie un cod in C in care sa primesti ca argument in linia de comanda calea catre un
//fisier si un numar N care reprezinta numarul de threaduri. Pentru fiecare thread, 
//imparte continutul fisierului in N parti egale si numara cate cuvinte apare in fiecare.
//Rezultatul total va fi afisat pe ecran

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/stat.h>
#include<fcntl.h>

#define BUFFER_SIZE 1024

int total_count = 0;
pthread_mutex_t mutex;

typedef struct{
    int length;
    char *segment;
}ThreadData;


void *handle_function(void *args)
{
    ThreadData *data = (ThreadData *) args;
    int i;
    int local_count = 0;
    for(i=0; i<data->length; i++)
    {
        if(data->segment[i] == ' ')
        {
            local_count ++;
        }
    }

    //modificam in siguranta totalul de cuvinte
    pthread_mutex_lock(&mutex);
    total_count = total_count + local_count;
    pthread_mutex_unlock(&mutex);

    free(data);
    return NULL;
}


int main(int argc, char **argv)
{
    if(argc != 3)
    {
        perror("Numar incorect de argumente in linia de comanda!!");
        exit(1);
    }

    int N = atoi(argv[1]);  //numarul de threaduri
    const char *file_path = argv[2];

    int fd = open(file_path, O_RDONLY);
    if(fd < 0)
    {
        perror("Eroare la deschiderea fisierului!!");
        exit(1);
    }

    //obtinem marimea fisierului
    struct stat st;
    if(fstat(fd, &st) < 0)
    {
        perror("Eroare la fstat!!");
        exit(1);
    }

    size_t file_size = st.st_size;
    int segment_size = file_size / N;  //marimea unui segment pentru un thread

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fd, buffer, file_size);
    if(bytes_read < 0)
    {
        perror("Eroare la citirea din fisier!");
        exit(1);
    }
    buffer[bytes_read] = '\0';
    close(fd);


    int i;
    pthread_t threads[N];
    pthread_mutex_init(&mutex, NULL);
    for(i=0; i<N; i++)
    {
        ThreadData *data = malloc(sizeof(ThreadData));
        if(!data)
        {
            perror("Eroare la alocarea dinamica!!");
            exit(1);
        }

        //daca ma aflu la ultimul thread, imi adaug la el restul ramas din impartirea
        //dimensiunii fisierului la numarul de threaduri
        if(i == N - 1)
        {
            data->length = file_size - i * segment_size;
        }
        else
        {
            data->length = segment_size;
        }

        data->segment = malloc(data->length + 1);  //alocam memorie pentru un segment
        if(!data->segment)
        {
            perror("Eroare la alocarea dinamica pentru segment!!");
            exit(1);
        }
        
        //buffer este un pointer către o zonă de memorie unde a fost citit întregul 
        //conținut al fișierului. Deci, daca ma aflu la inceputul fisierului, vreau ca sa 
        //copiez continutul memoriei aflata la segment_size * i, catre destinatia 
        //data->segment, totodata, nedepasind valoarea data->length
        memcpy(data->segment, buffer + segment_size * i,  data->length);
        data->segment[data->length] = '\0';

        if(pthread_create(&threads[i], NULL, handle_function, data) != 0)
        {
            perror("Eroare la pthread_create!!");
            exit(1);
        }
    }

    //asteptam sa se finlizeze toate thread-urile
    for(i=0; i<N; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("Numarul total de cuvinte din fisier este: %d\n", total_count + 1);
    pthread_mutex_destroy(&mutex);
    return 0;
}
