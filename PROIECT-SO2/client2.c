#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) {
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

    printf("Conectat la server!\n");

    char playerName[50];
    printf("Introduceți numele dvs.: ");
    fgets(playerName, sizeof(playerName), stdin);
    playerName[strcspn(playerName, "\n")] = '\0';
    send(sock, playerName, strlen(playerName), 0);

    while (1) 
    {
        char buffer[BUFFER_SIZE] = {0};
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) 
        {
            buffer[bytesReceived] = '\0';
            printf("%s\n", buffer);

            if (strstr(buffer, "Your move")) 
            {
                printf("Introduceți mutarea (1-9): ");
                fgets(buffer, sizeof(buffer), stdin);
                send(sock, buffer, strlen(buffer), 0);
            }
        }
    }

    close(sock);
    return 0;
}
