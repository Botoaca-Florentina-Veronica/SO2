#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SIDE 3
#define PORT 8080
#define MAX_CLIENTS 2

char board[SIDE][SIDE]; // Tabla de joc

// Inițializează tabla de joc cu spații libere
void initializeBoard() {
    for (int i = 0; i < SIDE; i++)
        for (int j = 0; j < SIDE; j++)
            board[i][j] = ' ';
}

// Funcție pentru trimiterea tablei de joc către clienți
void sendBoardToClients(int client1_fd, int client2_fd) {
    char boardStr[1024];
    snprintf(boardStr, sizeof(boardStr),
        "Tabla de joc:\n %c | %c | %c \n-----------\n %c | %c | %c \n-----------\n %c | %c | %c \n",
        board[0][0], board[0][1], board[0][2],
        board[1][0], board[1][1], board[1][2],
        board[2][0], board[2][1], board[2][2]);

    send(client1_fd, boardStr, strlen(boardStr), 0);
    send(client2_fd, boardStr, strlen(boardStr), 0);
}

// Funcție pentru verificarea câștigătorului
int checkWinner() {
    for (int i = 0; i < SIDE; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ')
            return 1;
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ')
            return 1;
    }
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ')
        return 1;
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ')
        return 1;
    return 0;
}

// Verifică dacă tabla este completă (remiză)
int checkDraw() {
    for (int i = 0; i < SIDE; i++)
        for (int j = 0; j < SIDE; j++)
            if (board[i][j] == ' ')
                return 0; // Mai sunt locuri libere
    return 1; // Tabla e completă
}

int main() {
    int server_fd, client1_fd, client2_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creare socket server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Asociază socket-ul la port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Așteptând jucători...\n");

    // Acceptă conexiuni de la doi clienți
    if ((client1_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("Accept client 1 failed");
        exit(EXIT_FAILURE);
    }
    printf("Jucătorul 1 conectat!\n");

    if ((client2_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("Accept client 2 failed");
        exit(EXIT_FAILURE);
    }
    printf("Jucătorul 2 conectat!\n");

    initializeBoard();  // Inițializarea tablei de joc
    sendBoardToClients(client1_fd, client2_fd);  // Trimite tabla inițială ambilor clienți

    int currentPlayer = 0;
    while (1) {
        int player_fd = currentPlayer == 0 ? client1_fd : client2_fd;
        char buffer[1024] = {0};

        // Așteaptă mutarea de la jucătorul curent
        read(player_fd, buffer, sizeof(buffer));
        int move = atoi(buffer) - 1;
        int row = move / SIDE;
        int col = move % SIDE;

        if (move < 0 || move >= SIDE * SIDE || board[row][col] != ' ') {
            char *invalidMoveMsg = "Mutare invalidă! Încercați din nou.\n";
            send(player_fd, invalidMoveMsg, strlen(invalidMoveMsg), 0);
            continue;
        }

        // Actualizează tabla și trimite ambilor clienți
        board[row][col] = (currentPlayer == 0) ? 'X' : 'O';
        sendBoardToClients(client1_fd, client2_fd);

        // Verifică starea jocului
        if (checkWinner()) {
            char *winMsg = "Ați câștigat!\n";
            char *loseMsg = "Ați pierdut!\n";
            send(player_fd, winMsg, strlen(winMsg), 0);
            send(player_fd == client1_fd ? client2_fd : client1_fd, loseMsg, strlen(loseMsg), 0);
            break;
        }

        if (checkDraw()) {
            char *drawMsg = "Remiză!\n";
            send(client1_fd, drawMsg, strlen(drawMsg), 0);
            send(client2_fd, drawMsg, strlen(drawMsg), 0);
            break;
        }

        currentPlayer = !currentPlayer; // Schimbă jucătorul
    }

    close(client1_fd);
    close(client2_fd);
    close(server_fd);
    return 0;
}
