#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

// Structuri și variabile globale pentru buffer și control
char buffer[BUFFER_SIZE];
int length = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Funcție pentru transformarea literelor
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

// Thread pentru trimiterea datelor pe socket
void *write_to_socket(void *arg) 
{
    int socket_fd = *(int *)arg;

    while (1) 
    {
        pthread_mutex_lock(&mutex);
        if (length > 0) 
        {
            write(socket_fd, buffer, length);
            length = 0; // Resetăm lungimea bufferului
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

// Thread pentru citirea de la stdin
void *read_from_stdin(void *arg) 
{
    while (1) 
    {
        char temp_buffer[BUFFER_SIZE];
        if (fgets(temp_buffer, BUFFER_SIZE, stdin) == NULL) 
        {
            perror("Error reading from stdin");
            continue;
        }

        pthread_mutex_lock(&mutex);
        strncpy(buffer, temp_buffer, BUFFER_SIZE);
        length = strlen(buffer);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

// Thread pentru citirea datelor de la socket
void *read_from_socket(void *arg) 
{
    int socket_fd = *(int *)arg;

    while (1) 
    {
        char temp_buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(socket_fd, temp_buffer, BUFFER_SIZE - 1);

        if (bytes_read <= 0) 
        {
            perror("Error reading from socket");
            break;
        }

        temp_buffer[bytes_read] = '\0';
        transform_text(temp_buffer);

        pthread_mutex_lock(&mutex);
        strncpy(buffer, temp_buffer, BUFFER_SIZE);
        length = strlen(buffer);
        pthread_mutex_unlock(&mutex);
    }

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

    int socket_fd;
    struct sockaddr_in server_addr;

    // Creăm socketul clientului
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) 
    {
        perror("Invalid address");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Conectăm clientul la server
    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("Connection failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", ip, port);

    pthread_t write_thread, stdin_thread, socket_thread;

    // Creăm thread-urile
    pthread_create(&write_thread, NULL, write_to_socket, &socket_fd);
    pthread_create(&stdin_thread, NULL, read_from_stdin, NULL);
    pthread_create(&socket_thread, NULL, read_from_socket, &socket_fd);

    // Așteptăm terminarea thread-urilor
    pthread_join(write_thread, NULL);
    pthread_join(stdin_thread, NULL);
    pthread_join(socket_thread, NULL);

    close(socket_fd);
    return 0;
}
