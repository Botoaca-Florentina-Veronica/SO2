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
    //recv() primește date de la server. Datele sunt stocate în boardStr
    if (bytesReceived > 0) 
    {
        //Dacă se primesc date valide, acestea sunt afișate sub forma unei table de joc
        boardStr[bytesReceived] = '\0'; // Termină șirul primit
        printf("%s\n", boardStr); // Afișează tabla
    }
}

int main(void) 
{
    int sock = 0;
    struct sockaddr_in serv_addr;  //Adresa serverului la care clientul vrea să se conecteze trebuie 
    //configurată. Acest lucru se face folosind o structură de tip sockaddr_in

    // **2. Crearea și conectarea la server**
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("\nEroare la crearea socket-ului\n");
        return -1;
    }

    // Configurarea adresei serverului
    serv_addr.sin_family = AF_INET;  //setează familia de adrese la IPv4
    serv_addr.sin_port = htons(PORT); //setează portul la care clientul vrea să se conecteze (același port ca pe server: 8080)

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) 
    {
        //convertește adresa IP a serverului din format text ("127.0.0.1") în format binar, necesar pentru conexiune
        printf("\nAdresă invalidă/Adresă nesuportată\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        //inițiază o conexiune TCP către serverul specificat prin serv_addr
        printf("\nConexiune eșuată\n");
        return -1;
    }

    printf("Conectat la server! Aștept tabla de joc...\n");

    // **3. Buclă principală a clientului**
    while (1) 
    {
        receiveAndShowBoard(sock); // Primește și afișează tabla actualizată

        char buffer[1024] = {0};
        printf("Introduceți mutarea (1-9): ");
        fgets(buffer, sizeof(buffer), stdin);
        // citește mutarea introdusă de utilizator din consolă

        send(sock, buffer, strlen(buffer), 0); 
        // trimite mutarea către server prin socketul sock

        char response[1024] = {0};
        int bytesReceived = recv(sock, response, sizeof(response) - 1, 0); //primește răspunsul de la server
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

    //Când jocul se termină (din cauza unei victorii, înfrângeri sau remize), clientul închide conexiunea
    close(sock);
    return 0;
}
