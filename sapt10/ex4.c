#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

// Structura pentru a transmite argumentele unui thread
typedef struct {
    int client_socket;
} thread_args_t;

// Funcție pentru a transforma literele mari în mici și invers
void transform_text(char *text) 
{
    for (int i = 0; text[i] != '\0'; i++) 
    {
        if (islower(text[i])) 
        {
            text[i] = toupper(text[i]);
        } 
        else if (isupper(text[i])) 
        {
            text[i] = tolower(text[i]);
        }
    }
}

// Funcție pentru gestionarea conexiunii unui client
void *handle_client(void *args) 
{
    thread_args_t *thread_args = (thread_args_t *)args;
    int client_socket = thread_args->client_socket;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    free(thread_args); // Eliberăm memoria alocată pentru argumente

    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) 
    {
        buffer[bytes_read] = '\0'; // Adăugăm terminator de șir
        transform_text(buffer);    // Transformăm textul
        write(client_socket, buffer, strlen(buffer)); // Trimitem textul transformat înapoi
    }

    close(client_socket); // Închidem conexiunea cu clientul
    return NULL;
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int server_socket;
    struct sockaddr_in server_addr;

    // Creăm socketul serverului
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) 
    {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    // Legăm socketul la adresa și portul specificate
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Punem serverul în modul de ascultare
    if (listen(server_socket, 5) < 0) 
    {
        perror("listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server running on %s:%d\n", ip, port);

    while (1) 
    {
        int client_socket;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Acceptăm o conexiune de la un client
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) < 0) 
        {
            perror("accept failed");
            continue;
        }

        // Alocăm memoria pentru argumentele thread-ului
        thread_args_t *args = malloc(sizeof(thread_args_t));
        if (args == NULL) 
        {
            perror("malloc failed");
            close(client_socket);
            continue;
        }
        args->client_socket = client_socket;

        // Creăm un thread pentru a gestiona conexiunea clientului
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, args) != 0) 
        {
            perror("pthread_create failed");
            free(args);
            close(client_socket);
            continue;
        }

        // Detach thread-ul pentru a elibera resursele automat după terminare
        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}
