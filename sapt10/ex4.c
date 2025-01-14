#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<ctype.h>
#include<pthread.h>
#include<sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void transform_text(char *cuvant)
{
    int i;
    for(i = 0; i < strlen(cuvant); i++)
    {
        if(islower(cuvant[i]))
        {
            cuvant[i] = toupper(cuvant[i]);
        }
        else if(isupper(cuvant[i]))
        {
            cuvant[i] = tolower(cuvant[i]);
        }
    }
}

void *handle_client(void *client_socket_ptr)
{
    int client_fd = *(int *)client_socket_ptr;
    free(client_socket_ptr);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        transform_text(buffer);

        if(write(client_fd, buffer, strlen(buffer)) < 0)
        {
            perror("Eroare la scrierea datelor către client");
            break;
        }
    }
    if(bytes_read < 0)
    {
        perror("Eroare la citirea datelor de la client");
    }

    close(client_fd);
    return NULL;
}

int main(void)
{
    int server_fd;
    struct sockaddr_in server_bind, client_addr;

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Eroare la socket!");
        exit(1);
    }

    memset(&server_bind, 0, sizeof(server_bind));
    server_bind.sin_family = AF_INET;
    server_bind.sin_addr.s_addr = INADDR_ANY;
    server_bind.sin_port = htons(PORT);

    if(bind(server_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0)
    {
        perror("Eroare la bind!");
        exit(1);
    }

    if(listen(server_fd, 5) < 0)
    {
        perror("Eroare la listen!");
        exit(1);
    }
    printf("Serverul asculta conexiuni...\n");

    while(1)
    {
        int *client_socket_ptr = malloc(sizeof(int));
        if(!client_socket_ptr)
        {
            perror("Eroare la alocarea dinamica!");
            continue;
        }

        unsigned int client_addr_len = sizeof(client_addr);
        *client_socket_ptr = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if(*client_socket_ptr < 0)
        {
            perror("Eroare la accept!");
            free(client_socket_ptr);
            continue;
        }

        pthread_t thread;
        if(pthread_create(&thread, NULL, handle_client, client_socket_ptr) != 0)
        {
            perror("Eroare la pthread_create!");
            close(*client_socket_ptr);
            free(client_socket_ptr);
            continue;
        }
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}
