#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

// **Arhitectura**
// Clientul este responsabil pentru:
// - Conectarea la server.
// - Primiterea tablei actualizate.
// - Trimiterea mutărilor către server.
// - Afișarea feedback-ului primit de la server.

// **1. Funcția de primire și afișare a tablei**
// Scop: Primește tabla de joc de la server și o afișează.

void receiveAndShowBoard(int server_fd) 
{
    char boardStr[1024];
    int bytesReceived = recv(server_fd, boardStr, sizeof(boardStr) - 1, 0);
    if (bytesReceived > 0) 
    {
        boardStr[bytesReceived] = '\0'; // Termină șirul primit
        printf("%s\n", boardStr); // Afișează tabla
    }
}

int main(void) 
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Crearea și conectarea la server
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("\nEroare la crearea socket-ului\n");
        return -1;
    }

    // Configurarea adresei serverului
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

    // Trimiterea numelui utilizatorului
    char playerName[50];
    printf("Introduceți numele dvs.: ");
    fgets(playerName, sizeof(playerName), stdin);
    playerName[strcspn(playerName, "\n")] = '\0'; // Elimină newline
    send(sock, playerName, strlen(playerName), 0); // Trimite numele serverului

    // Buclă principală a clientului
    while (1) 
    {
        receiveAndShowBoard(sock); // Primește și afișează tabla actualizată

        char buffer[1024] = {0};
        printf("Introduceți mutarea (1-9): ");
        fgets(buffer, sizeof(buffer), stdin);

        // Trimite mutarea către server
        send(sock, buffer, strlen(buffer), 0);

        char response[1024] = {0};
        int bytesReceived = recv(sock, response, sizeof(response) - 1, 0); // Primește răspunsul de la server
        if (bytesReceived > 0) 
        {
            response[bytesReceived] = '\0';
            printf("%s\n", response);

            // Încheierea jocului în caz de victorie, înfrângere sau remiză
            if (strstr(response, "Ați câștigat!") || 
                strstr(response, "Ați pierdut!") || 
                strstr(response, "Remiză!")) 
            {
                break;
            }
        }
    }

     //Când jocul se termină (din cauza unei victorii, înfrângeri sau remize), clientul închide conexiunea
    close(sock);
    return 0;
}
