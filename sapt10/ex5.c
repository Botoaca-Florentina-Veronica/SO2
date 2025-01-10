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

// Definirea portului serverului și dimensiunea buffer-ului
#define PORT 8080
#define BUFFER_SIZE 1024

// Funcția care gestionează fiecare linie primită de la client
void *handle_client_line(void *client_socket_ptr) 
{
    // Extragem socket-ul clientului din argumentul funcției
    int client_fd = *(int *)client_socket_ptr;
    free(client_socket_ptr); // Eliberăm memoria alocată pentru pointer

    char buffer[BUFFER_SIZE] = {0}; // Buffer pentru stocarea mesajului primit de la client
    int bytes_read = read(client_fd, buffer, BUFFER_SIZE); // Citim datele de la client

    if (bytes_read > 0) 
    {
        buffer[bytes_read] = '\0'; // Adăugăm terminatorul de șir pentru a crea un string valid
        int length = strlen(buffer); // Calculăm lungimea mesajului primit

        // Afișăm mesajul primit și lungimea acestuia pe server
        printf("Received: %s (length: %d)\n", buffer, length);

        char response[BUFFER_SIZE];
        // Pregătim răspunsul (lungimea mesajului primit)
        snprintf(response, BUFFER_SIZE, "%d", length);

        // Trimitem răspunsul înapoi la client
        send(client_fd, response, strlen(response), 0);
    }

    // Încheiem thread-ul returnând NULL (resursele se eliberează automat deoarece am folosit pthread_detach)
    return NULL;
}

int main(void) 
{
    int server_fd, client_fd; // Descriptorii de socket pentru server și client
    struct sockaddr_in server_bind, client_addr; // Structuri pentru informațiile despre server și client

    // Creăm socket-ul serverului
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Error creating socket"); // Dacă apare o eroare, o afișăm și ieșim
        exit(1);
    }

    // Pregătim structura pentru server
    memset(&server_bind, 0, sizeof(server_bind)); // Inițializăm structura cu 0
    server_bind.sin_family = AF_INET; // Utilizăm familia de protocoale IPv4
    server_bind.sin_addr.s_addr = INADDR_ANY; // Acceptăm conexiuni de la orice adresă
    server_bind.sin_port = htons(PORT); // Setăm portul serverului (convertim în rețea)

    // Asociem socket-ul creat cu adresa și portul specificate
    if (bind(server_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0) 
    {
        perror("Error binding socket"); // Dacă apare o eroare la bind, o afișăm și ieșim
        exit(1);
    }

    // Punem serverul în modul de ascultare (acceptă până la 5 conexiuni simultane în coadă)
    if (listen(server_fd, 5) < 0) 
    {
        perror("Error listening on socket"); // Dacă apare o eroare la listen, o afișăm și ieșim
        exit(1);
    }
    printf("Server listening on port %d...\n", PORT); // Confirmăm că serverul ascultă conexiuni

    // Lungimea structurii clientului
    unsigned int client_addr_len = sizeof(client_addr);

    // Acceptăm o conexiune de la client
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) 
    {
        perror("Error accepting connection"); // Dacă apare o eroare la accept, o afișăm și ieșim
        exit(1);
    }

    // Bucla principală a serverului
    while (1) 
    {
        // Alocăm memorie pentru a stoca descriptorul socket-ului clientului
        int *client_socket_ptr = malloc(sizeof(int));
        if (!client_socket_ptr) 
        {
            perror("Error allocating memory for client socket");
            continue; // Dacă nu reușim să alocăm memorie, trecem la următorul ciclu
        }

        *client_socket_ptr = client_fd; // Stocăm descriptorul socket-ului clientului

        // Creăm un thread nou pentru a procesa linia primită de la client
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client_line, client_socket_ptr) != 0) 
        {
            perror("Error creating thread"); // Dacă apare o eroare la crearea thread-ului, o afișăm
            free(client_socket_ptr); // Eliberăm memoria alocată
            continue; // Trecem la următoarea iterație
        }

        // Marcăm thread-ul ca "detached", astfel încât resursele sale să fie eliberate automat
        pthread_detach(thread);
    }

    // Închidem socket-urile (nu se va ajunge aici deoarece serverul rulează în buclă infinită)
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

// Definirea portului serverului și dimensiunea buffer-ului
#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) 
{
    int sockfd = 0; // Descriptorul socket-ului pentru client
    struct sockaddr_in server_fd; // Structura pentru stocarea informațiilor despre server
    char line[BUFFER_SIZE]; // Buffer pentru a citi datele introduse de utilizator
    char buffer[BUFFER_SIZE]; // Buffer pentru a stoca răspunsurile primite de la server

    // Crearea unui socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        // Dacă apare o eroare la crearea socket-ului, afișăm mesajul și terminăm execuția
        perror("Error creating socket");
        exit(1);
    }

    // Inițializăm structura `server_fd` cu informațiile necesare
    memset(&server_fd, 0, sizeof(server_fd)); // Setăm toate câmpurile structurii la 0
    server_fd.sin_family = AF_INET; // Utilizăm familia de protocoale IPv4
    server_fd.sin_addr.s_addr = INADDR_ANY; // Adresa serverului (oricare adresă locală)
    server_fd.sin_port = htons(PORT); // Setăm portul serverului (convertit în format rețea)

    // Conectarea la server
    if (connect(sockfd, (struct sockaddr *)&server_fd, sizeof(server_fd)) < 0) 
    {
        // Dacă apare o eroare la conectare, afișăm mesajul și terminăm execuția
        perror("Error connecting to server");
        exit(1);
    }
    printf("Connected to server\n"); // Confirmăm conectarea reușită

    // Bucla principală pentru interacțiunea clientului cu serverul
    while (1) 
    {
        printf("Enter a line: "); // Solicităm utilizatorului să introducă o linie de text
        if (!fgets(line, BUFFER_SIZE, stdin)) 
        {
            // Dacă apare o eroare la citirea datelor de la tastatură
            printf("Error reading input");
            break;
        }

        // Trimitem linia introdusă către server
        send(sockfd, line, strlen(line), 0);

        // Citim răspunsul serverului
        int bytes_read = read(sockfd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) 
        {
            // Dacă serverul s-a deconectat, afișăm mesajul și terminăm execuția
            printf("Server disconnected\n");
            exit(1);
        }

        buffer[bytes_read] = '\0'; // Adăugăm terminatorul de șir pentru a crea un string valid
        printf("Server response: %s\n", buffer); // Afișăm răspunsul primit de la server
    }

    // Închidem socket-ul
    close(sockfd);
    return 0;
}
