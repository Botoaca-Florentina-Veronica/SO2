//server.c

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Funcție care calculează suma numerelor dintr-un șir de caractere
int calculate_sum(char *line) 
{
    int sum = 0, num;
    char *token = strtok(line, " "); // Împarte linia în funcție de spații
    while (token != NULL && strcmp(token, "=") != 0) 
    {
        if (sscanf(token, "%d", &num) == 1) 
        { // Verifică dacă este un număr valid
            sum = sum + num; // Adaugă numărul la sumă
        }
        token = strtok(NULL, " "); // Treci la următorul element
    }
    return sum; // Returnează suma
}

// Funcție pentru a gestiona comunicarea cu un client
void *handle_client(void *client_socket_ptr) 
{
    int client_fd = *(int *)client_socket_ptr;
    free(client_socket_ptr);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) 
    {
        buffer[bytes_read] = '\0'; // Adaugă caracterul de terminare
        int sum = calculate_sum(buffer); // Calculează suma numerelor

        // Pregătește răspunsul
        snprintf(buffer, sizeof(buffer), "Suma este: %d\n", sum);

        // Trimite rezultatul clientului
        if (write(client_fd, buffer, strlen(buffer)) < 0) 
        {
            perror("Eroare la scrierea datelor către client");
            break;
        }
    }

    close(client_fd);
    return NULL;
}

// Funcția principală
int main(void) 
{
    int server_fd;
    struct sockaddr_in server_bind, client_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Eroare la socket!");
        exit(1);
    }

    memset(&server_bind, 0, sizeof(server_bind));
    server_bind.sin_family = AF_INET;
    server_bind.sin_addr.s_addr = INADDR_ANY;
    server_bind.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0) 
    {
        perror("Eroare la bind!");
        exit(1);
    }

    if (listen(server_fd, 5) < 0) 
    {
        perror("Eroare la listen!");
        exit(1);
    }
    printf("Serverul ascultă conexiuni...\n");

    while (1) 
    {
        int *client_socket_ptr = malloc(sizeof(int));
        if (!client_socket_ptr) 
        {
            perror("Eroare la alocarea dinamică!");
            continue;
        }

        unsigned int client_addr_len = sizeof(client_addr);
        if((*client_socket_ptr = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
            perror("Eroare la accept!!");
            exit(1);
        }
        

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, client_socket_ptr) != 0) 
        {
            perror("Eroare la pthread_create!");
            continue;
        }
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}


//client.c

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) 
{
    int client_fd;
    struct sockaddr_in server_bind;
    char buffer[BUFFER_SIZE];

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Eroare la crearea socketului!");
        exit(1);
    }

    memset(&server_bind, 0, sizeof(server_bind));
    server_bind.sin_family = AF_INET;
    server_bind.sin_port = htons(PORT);

    if (connect(client_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0) 
    {
        perror("Eroare la conectare!");
        exit(1);
    }

    printf("Conectat la server. Introduceți o linie de numere separate prin spațiu, urmate de '=' (ex: '1 2 3 ='):\n");
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) 
    {
        if (write(client_fd, buffer, strlen(buffer)) < 0) 
        {
            perror("Eroare la trimiterea datelor!");
            break;
        }

        ssize_t bytes_received = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_received < 0) 
        {
            perror("Eroare la primirea răspunsului!");
            break;
        }
        buffer[bytes_received] = '\0';
        printf("Răspuns de la server: %s\n", buffer);
    }

    close(client_fd);
    return 0;
}
