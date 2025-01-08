#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

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

    // Creăm pipe-urile pentru comunicarea între procese
    int pipe_stdin[2], pipe_socket[2];

    if (pipe(pipe_stdin) < 0 || pipe(pipe_socket) < 0) 
    {
        perror("Pipe creation failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Creăm procesul pentru read_from_stdin
    pid_t pid_stdin = fork();
    if (pid_stdin == 0) 
    {
        close(pipe_stdin[0]); // Închidem capătul de citire al pipe-ului

        char buffer[BUFFER_SIZE];
        while (fgets(buffer, BUFFER_SIZE, stdin)) 
        {
            write(pipe_stdin[1], buffer, strlen(buffer));
        }

        close(pipe_stdin[1]);
        exit(0);
    }

    // Creăm procesul pentru read_from_socket
    pid_t pid_socket = fork();
    if (pid_socket == 0) 
    {
        close(pipe_socket[0]); // Închidem capătul de citire al pipe-ului

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        while ((bytes_read = read(socket_fd, buffer, BUFFER_SIZE - 1)) > 0) 
        {
            buffer[bytes_read] = '\0';
            transform_text(buffer);
            write(pipe_socket[1], buffer, strlen(buffer));
        }

        close(pipe_socket[1]);
        exit(0);
    }

    // Creăm procesul pentru write_to_socket
    pid_t pid_write = fork();
    if (pid_write == 0) 
    {
        close(pipe_stdin[1]); // Închidem capătul de scriere al pipe-ului
        close(pipe_socket[1]); // Închidem capătul de scriere al pipe-ului

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        while (1) 
        {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(pipe_stdin[0], &read_fds);
            FD_SET(pipe_socket[0], &read_fds);

            int max_fd = (pipe_stdin[0] > pipe_socket[0]) ? pipe_stdin[0] : pipe_socket[0];

            if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) > 0) 
            {
                if (FD_ISSET(pipe_stdin[0], &read_fds)) 
                {
                    bytes_read = read(pipe_stdin[0], buffer, BUFFER_SIZE - 1);
                    if (bytes_read > 0) 
                    {
                        buffer[bytes_read] = '\0';
                        write(socket_fd, buffer, bytes_read);
                    }
                }

                if (FD_ISSET(pipe_socket[0], &read_fds)) 
                {
                    bytes_read = read(pipe_socket[0], buffer, BUFFER_SIZE - 1);
                    if (bytes_read > 0) 
                    {
                        buffer[bytes_read] = '\0';
                        write(socket_fd, buffer, bytes_read);
                    }
                }
            }
        }

        close(pipe_stdin[0]);
        close(pipe_socket[0]);
        exit(0);
    }

    // Închidem capetele pipe-urilor în procesul principal
    close(pipe_stdin[0]);
    close(pipe_stdin[1]);
    close(pipe_socket[0]);
    close(pipe_socket[1]);

    // Așteptăm terminarea proceselor copil
    waitpid(pid_stdin, NULL, 0);
    waitpid(pid_socket, NULL, 0);
    waitpid(pid_write, NULL, 0);

    close(socket_fd);
    return 0;
}
