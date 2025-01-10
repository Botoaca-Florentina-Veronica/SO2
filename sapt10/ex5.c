//server.c

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *handle_client_line(void *client_socket_ptr) 
{
    int client_fd = *(int *)client_socket_ptr;
    free(client_socket_ptr);

    char buffer[BUFFER_SIZE] = {0};
    int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
    if (bytes_read > 0) 
    {
        buffer[bytes_read] = '\0'; // Asigură terminarea stringului
        int length = strlen(buffer);
        printf("Received: %s (length: %d)\n", buffer, length);

        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "%d", length);
        send(client_fd, response, strlen(response), 0);
    }
    return NULL;
}

int main(void) 
{
    int server_fd, client_fd;
    struct sockaddr_in server_bind, client_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Error creating socket");
        exit(1);
    }

    memset(&server_bind, 0, sizeof(server_bind));
    server_bind.sin_family = AF_INET;
    server_bind.sin_addr.s_addr = INADDR_ANY;
    server_bind.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0) 
    {
        perror("Error binding socket");
        exit(1);
    }

    if (listen(server_fd, 5) < 0) 
    {
        perror("Error listening on socket");
        exit(1);
    }
    printf("Server listening on port %d...\n", PORT);

    unsigned int client_addr_len = sizeof(client_addr);
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) 
    {
        perror("Error accepting connection");
        exit(1);
    }

    while (1) 
    {
        int *client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_fd;

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client_line, client_socket_ptr);
        pthread_detach(thread);
    }

    close(server_fd);
    close(client_fd);
    return 0;
}


//client.c
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) 
{
    int sockfd = 0;
    struct sockaddr_in server_fd;
    char line[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Error creating socket");
        exit(1);
    }

    memset(&server_fd, 0, sizeof(server_fd));
    server_fd.sin_family = AF_INET;
    server_fd.sin_addr.s_addr = INADDR_ANY;
    server_fd.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&server_fd, sizeof(server_fd)) < 0) 
    {
        perror("Error connecting to server");
        exit(1);
    }
    printf("Connected to server\n");

    while (1) 
    {
        printf("Enter a line: ");
        if (!fgets(line, BUFFER_SIZE, stdin)) 
        {
            printf("Error reading input");
            break;
        }

        send(sockfd, line, strlen(line), 0);

        int bytes_read = read(sockfd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) 
        {
            printf("Server disconnected\n");
            exit(1);
        }

        buffer[bytes_read] = '\0'; // Asigură terminarea stringului
        printf("Server response: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}

