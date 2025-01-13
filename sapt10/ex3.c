/* 
Să se scrie un program C ce implementează un server TCP. Programul se va lansa în linie de comandă astfel:

./prog <ip> <port>

   programul va implementa un server TCP care, pentru fiecare conexiune, va citi date text și, pentru fiecare 
conexiune va trimite înapoi clientului conectat textul primit cu literele mari schimbate în litere mici și invers.
   Pentru fiecare client conectat programul va creea câte un proces fiu ce se va ocupa de gestionarea 
comunicației cu clientul respectiv
Programul va fi testat cu o componentă client rezentată de utilitarul netcat:

nc <ip> <port>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <signal.h>

#define BUFFER_SIZE 1024

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
void handle_client(int client_socket) 
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) 
    {
        buffer[bytes_read] = '\0'; // Adăugăm terminator de șir
        transform_text(buffer);    // Transformăm textul
        write(client_socket, buffer, strlen(buffer)); // Trimitem textul transformat înapoi
    }

    close(client_socket); // Închidem conexiunea cu clientul
    exit(0); // Procesul fiu se termină
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

    int server_fd;
    struct sockaddr_in server_bind;

    // Ignorăm semnalele trimise de procesele fiu la terminare
    signal(SIGCHLD, SIG_IGN);

    // Creăm socketul serverului
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_bind.sin_family = AF_INET;
    server_bind.sin_port = htons(port);
    server_bind.sin_addr.s_addr = INADDR_ANY;

    // Legăm socketul la adresa și portul specificate
    if (bind(server_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0) 
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Punem serverul în modul de ascultare
    if (listen(server_fd, 5) < 0) 
    {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server running on %s:%d\n", ip, port);

    while (1) 
    {
        int client_fd;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Acceptăm o conexiune de la un client
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) 
        {
            perror("accept failed");
            continue;
        }

        // Creăm un proces fiu pentru a gestiona conexiunea clientului
        pid_t pid = fork();
        if (pid < 0) 
        {
            perror("fork failed");
            close(client_fd);
            continue;
        } 
        else if (pid == 0) 
        {
            // Procesul fiu
            close(server_fd); // Procesul fiu nu are nevoie de socketul serverului
            handle_client(client_fd);
        } 
        else 
        {
            // Procesul părinte
            close(client_fd); // Procesul părinte nu are nevoie de socketul clientului
        }
    }

    close(server_fd);
    return 0;
}
