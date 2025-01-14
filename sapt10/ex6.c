/*
Creează un server și un client care schimbă mesaje simple
Scrie un program în care:

Serverul ascultă pe un port specific (ex.: 8080).
Clientul trimite un mesaj ("Hello, Server!") la server.
Serverul primește mesajul și răspunde cu un mesaj propriu ("Hello, Client!").
Cerere suplimentară: Folosește TCP (protocolul SOCK_STREAM).
*/

//server.c
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) 
{
    int server_fd, client_fd;
    struct sockaddr_in server_bind, client_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Eroare la crearea socket-ului!");
        exit(1);
    }

    memset(&server_bind, 0, sizeof(server_bind));
    server_bind.sin_family = AF_INET;
    server_bind.sin_addr.s_addr = INADDR_ANY;
    server_bind.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0) 
    {
        perror("Eroare la bind!");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 5) < 0) 
    {
        perror("Eroare la listen!");
        close(server_fd);
        exit(1);
    }

    printf("Serverul asteapta conexiuni...\n");

    unsigned int client_addr_len = sizeof(client_addr);

    while (1) 
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) 
        {
            perror("Eroare la accept!");
            continue;
        }

        char buffer[BUFFER_SIZE];
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) 
        {
            buffer[bytes_received] = '\0';
            printf("Mesaj primit de la client: %s\n", buffer);

            const char *response = "Hello, Client!";
            send(client_fd, response, strlen(response), 0);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}

//client.c
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) 
{
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Crearea socket-ului
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Eroare la crearea socket-ului!");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Conectarea la server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("Eroare la conectarea la server!");
        close(client_fd);
        exit(1);
    }

    // Trimiterea mesajului către server
    const char *message = "Hello, Server!";
    send(client_fd, message, strlen(message), 0);

    // Primirea răspunsului de la server
    int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) 
    {
        buffer[bytes_received] = '\0';
        printf("Răspuns de la server: %s\n", buffer);
    }

    close(client_fd);
    return 0;
}

