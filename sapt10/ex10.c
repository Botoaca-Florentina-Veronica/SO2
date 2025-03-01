//problema client-server care numara cate cuvinte introduce clientul in terminal
#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080

typedef struct{
    int client_fd;
}ThreadData;

int numara_cuvinte(char *linie)
{
    int i;
    int total = 0;
    for(i=0; i<=strlen(linie); i++)
    {
        if(linie[i] == ' ')
        {
            total = total + 1;
        }
    }
    return total;
}


void *handle_function(void *args)
{
    ThreadData *data = (ThreadData *)args;
    int client_fd = data->client_fd;

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    //citim raspunsul dat de client in terminal
    while((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';  //adaugam caracterul terminator
        int total_cuvinte = numara_cuvinte(buffer);

        //retinem rezultatul in buffer
        snprintf(buffer, sizeof(buffer), "Numarul total de cuvinte introduse este egal cu: %d\n", total_cuvinte + 1);

        //trimitem inapoi catre client raspunsul dat de server in urma analizei acestuia
        if(write(client_fd, buffer, strlen(buffer)) < 0)
        {
            perror("Eroare la write!!");
            exit(1);
        }
    }

    close(client_fd);
    return NULL;
}

int main(void)
{
    int server_fd;
    struct sockaddr_in server_bind;

    //cream socket-ul pentru server
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Eroare la socket!!");
        exit(1);
    }

    //facem structura pentru server
    memset(&server_bind, 0, sizeof(server_bind));
    server_bind.sin_family = AF_INET; //familia de protocoale ipv4
    server_bind.sin_port = htons(PORT);
    server_bind.sin_addr.s_addr = INADDR_ANY;

    //acum fac legatura dintre socket, adresa si port
    if(bind(server_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0)
    {
        perror("Eroare la bind!!");
        exit(1);
    }

    //acum ascultam conexiuni intre server si clienti(max 10)
    if(listen(server_fd, 10) < 0)
    {
        perror("Eroare la listen!!");
        exit(1);
    }
    printf("Server running...\n");
    
    //bucla principala a programului
    while(1)
    {
        int client_fd;
        struct sockaddr_in client_addr;
        unsigned int client_addr_len = sizeof(client_addr);

        //realizez o conexiune intre server si client
        if((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
            perror("Eroare la accept!!");
            exit(1);
        }

        //alocam memorie pentru argumentele thread-ului:
        ThreadData *args = malloc(sizeof(ThreadData));
        if(!args)
        {
            perror("Eroare la alocarea dinamica!!");
            exit(1);
        }
        args->client_fd = client_fd;

        pthread_t thread;
        if(pthread_create(&thread, NULL, handle_function, args) != 0)
        {
            perror("Eroare la pthread_create!!");
            exit(1);
        }

        //eliberam automat resursele dupa terminare:
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}
