#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void receiveAndShowBoard(int server_fd) 
{
    char boardStr[1024];
    int bytesReceived = recv(server_fd, boardStr, sizeof(boardStr) - 1, 0);
    if (bytesReceived > 0) 
    {
        boardStr[bytesReceived] = '\0';
        printf("%s\n", boardStr);
    }
}

int main(void) 
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("\nEroare la crearea socket-ului\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) 
    {
        printf("\nAdresă invalidă/Adresă nesuportată\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        printf("\nConexiune eșuată\n");
        return -1;
    }

    printf("Conectat la server! Aștept tabla de joc...\n");

    while (1) 
    {
        receiveAndShowBoard(sock); // Primește și afișează tabla actualizată

        char buffer[1024] = {0};
        printf("Introduceți mutarea (1-9): ");
        fgets(buffer, sizeof(buffer), stdin);

        send(sock, buffer, strlen(buffer), 0); // Trimite mutarea la server

        char response[1024] = {0};
        int bytesReceived = recv(sock, response, sizeof(response) - 1, 0);
        if (bytesReceived > 0) 
        {
            response[bytesReceived] = '\0';
            printf("%s\n", response);

            if (strstr(response, "Ați câștigat!") || strstr(response, "Ați pierdut!") || strstr(response, "Remiză!")) 
            {
                break; // Jocul s-a terminat
            }
        }
    }

    close(sock);
    return 0;
}
