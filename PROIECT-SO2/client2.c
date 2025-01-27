#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080 // Portul folosit pentru conexiunea cu serverul
#define BUFFER_SIZE 1024 // Dimensiunea bufferului pentru mesaje

int main(void) {
    int sock = 0; // Descriptorul de socket
    struct sockaddr_in serv_addr; // Structură care definește adresa serverului

    // Crearea unui socket pentru comunicație
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        // Dacă socket-ul nu poate fi creat, afișează un mesaj de eroare și oprește programul
        printf("\nEroare la crearea socket-ului\n");
        return -1;
    }

    // Configurarea adresei serverului
    serv_addr.sin_family = AF_INET; // Familia de adrese IPv4
    serv_addr.sin_port = htons(PORT); // Portul serverului, convertit în format rețea

    // Conversia adresei IP de la format text la format binar
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) 
    {
        // Dacă adresa este invalidă sau nesuportată, afișează un mesaj de eroare
        printf("\nAdresă invalidă/Adresă nesuportată\n");
        return -1;
    }

    // Conectarea la server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        // Dacă conexiunea eșuează, afișează un mesaj de eroare
        printf("\nConexiune eșuată\n");
        return -1;
    }

    printf("Conectat la server!\n"); // Afișează un mesaj de confirmare a conexiunii

    char playerName[50]; // Buffer pentru numele jucătorului
    printf("Introduceți numele dvs.: "); // Solicită numele jucătorului
    fgets(playerName, sizeof(playerName), stdin); // Citește numele de la utilizator
    playerName[strcspn(playerName, "\n")] = '\0'; // Elimină newline-ul de la sfârșit
    send(sock, playerName, strlen(playerName), 0); // Trimite numele către server

    // Bucla principală pentru interacțiunea cu serverul
    while (1) 
    {
        char buffer[BUFFER_SIZE] = {0}; // Buffer pentru mesaje
        // Primește un mesaj de la server
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) 
        {
            buffer[bytesReceived] = '\0'; // Asigură terminatorul de șir
            printf("%s\n", buffer); // Afișează mesajul primit de la server

            // Verifică dacă serverul solicită o mutare
            if (strstr(buffer, "Your move")) 
            {
                // Solicită utilizatorului să introducă o mutare
                printf("Introduceți mutarea (1-9): ");
                fgets(buffer, sizeof(buffer), stdin); // Citește mutarea de la utilizator
                send(sock, buffer, strlen(buffer), 0); // Trimite mutarea către server
            }
        }
    }

    close(sock); // Închide socket-ul după terminarea jocului
    return 0; // Ieșire cu succes
}
