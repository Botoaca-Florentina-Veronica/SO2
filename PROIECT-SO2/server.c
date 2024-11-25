#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Constante folosite
#define SIDE 3      // Dimensiunea tablei de joc (3x3)
#define PORT 8080   // Portul pe care serverul ascultă conexiunile
#define MAX_CLIENTS 2 // Numărul maxim de clienți suportat

// **Arhitectura**
// Serverul este responsabil pentru:
// - Inițializarea tablei de joc.
// - Acceptarea conexiunilor de la clienți.
// - Sincronizarea și trimiterea tablei de joc către clienți.
// - Verificarea câștigătorului/remizei după fiecare mutare.

char board[SIDE][SIDE]; // Reprezentarea tablei de joc ca matrice 2D de caractere

// **1. Funcția de inițializare a tablei de joc**
// Scop: Să reseteze tabla de joc la o stare inițială, cu spații libere.
void initializeBoard() 
{
    int i, j;
    for (i = 0; i < SIDE; i++) 
    {
        for (j = 0; j < SIDE; j++) 
        {
            board[i][j] = ' '; // Fiecare celulă este inițial goală
        }
    }
}

// **2. Funcția de trimitere a tablei către clienți**
// Scop: Să creeze un șir care reprezintă tabla și să-l trimită ambilor clienți.
void sendBoardToClients(int client1_fd, int client2_fd) 
{
    char boardStr[1024]; // Șir pentru a stoca tabla de joc formatată
    snprintf(boardStr, sizeof(boardStr),
        "Tabla de joc:\n %c | %c | %c \n-----------\n %c | %c | %c \n-----------\n %c | %c | %c \n",
        board[0][0], board[0][1], board[0][2],
        board[1][0], board[1][1], board[1][2],
        board[2][0], board[2][1], board[2][2]);

    // Trimiterea tablei ambilor clienți
    send(client1_fd, boardStr, strlen(boardStr), 0);
    send(client2_fd, boardStr, strlen(boardStr), 0);
}

// **3. Funcția de verificare a câștigătorului**
// Scop: Verifică dacă un jucător a câștigat pe rânduri, coloane sau diagonale.
int checkWinner() 
{
    int i;  //i = linie, j = coloana

    // Verificare pe rânduri și coloane
    for (i = 0; i < SIDE; i++) 
    {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ')
        {
            return 1; // Câștigător pe rând
        }
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ')
        {
            return 1; // Câștigător pe coloană
        }
    }
    // Verificare pe diagonale
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ')
    {
        return 1; // Câștigător pe diagonala principală
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ')
    {
        return 1; // Câștigător pe diagonala secundară
    }
    return 0; // Nicio condiție de câștig
}

// **4. Funcția de verificare a remizei**
// Scop: Determină dacă tabla este completă fără să existe un câștigător.
int checkDraw() 
{
    int i, j;
    for (i = 0; i < SIDE; i++) 
    {
        for (j = 0; j < SIDE; j++) 
        {
            if (board[i][j] == ' ')
            {
                return 0; // Cel puțin o celulă este goală
            }
        }
    }
    return 1; // Toate celulele sunt ocupate
}

int main() 
{
    int server_fd, client1_fd, client2_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // **5. Creare și configurare server**
    // 1. Crearea unui socket pentru conexiuni TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Configurarea adresei serverului
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Ascultă pe toate interfețele
    address.sin_port = htons(PORT);
    // Adresa serverului este configurată pentru a asculta pe toate interfețele disponibile (INADDR_ANY) 
    // și portul specificat (8080)

    // 3. Legarea socket-ului la portul specificat
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    //Socket-ul creat este legat la adresa și portul specificat folosind funcția bind()
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 4. Ascultarea conexiunilor
    if (listen(server_fd, MAX_CLIENTS) < 0) 
    {
        // Serverul începe să asculte pentru conexiuni pe socket-ul configurat, cu o coadă de așteptare 
        // specificată (MAX_CLIENTS = 2)
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Așteptând jucători...\n");

    // **6. Acceptarea conexiunilor**
    // Se acceptă conexiunea de la primul client
    if ((client1_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) 
    {
        // Serverul folosește funcția accept() pentru a accepta conexiunea de la primul client
        // Dacă funcția reușește, serverul afișează un mesaj de confirmare
        perror("Accept client 1 failed");
        exit(EXIT_FAILURE);
    }
    printf("Jucătorul 1 conectat!\n");

    // Se acceptă conexiunea de la al doilea client
    if ((client2_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) 
    {
        // Serverul acceptă conexiunea de la al doilea client într-un mod similar
        // Dacă funcția reușește, serverul afișează un mesaj de confirmare
        perror("Accept client 2 failed");
        exit(EXIT_FAILURE);
    }
    printf("Jucătorul 2 conectat!\n");

    // **7. Inițializarea tablei și trimiterea către clienți**
    initializeBoard();
    sendBoardToClients(client1_fd, client2_fd);

    int currentPlayer = 0; // Jucătorul curent (0 pentru X, 1 pentru O)

    // **8. Buclă principală a jocului**
    while (1) 
    {
        int player_fd = currentPlayer == 0 ? client1_fd : client2_fd;
        char buffer[1024] = {0};

        // 8.1. Așteaptă mutarea de la jucătorul curent
        read(player_fd, buffer, sizeof(buffer));
        int move = atoi(buffer) - 1; // Mutarea (1-9) -> index în matrice
        int row = move / SIDE;
        int col = move % SIDE;

        // 8.2. Validarea mutării
        if (move < 0 || move >= SIDE * SIDE || board[row][col] != ' ') 
        {
            char *invalidMoveMsg = "Mutare invalidă! Încercați din nou.\n";
            send(player_fd, invalidMoveMsg, strlen(invalidMoveMsg), 0);
            continue; // Se cere o mutare nouă
        }

        // 8.3. Actualizează tabla
        board[row][col] = (currentPlayer == 0) ? 'X' : 'O';
        sendBoardToClients(client1_fd, client2_fd);

        // 8.4. Verifică starea jocului
        if (checkWinner()) 
        {
            char *winMsg = "Ați câștigat!\n";
            char *loseMsg = "Ați pierdut!\n";
            send(player_fd, winMsg, strlen(winMsg), 0);
            send(player_fd == client1_fd ? client2_fd : client1_fd, loseMsg, strlen(loseMsg), 0);
            break; // Jocul s-a terminat
        }

        if (checkDraw()) 
        {
            char *drawMsg = "Remiză!\n";
            send(client1_fd, drawMsg, strlen(drawMsg), 0);
            send(client2_fd, drawMsg, strlen(drawMsg), 0);
            break; // Jocul s-a terminat cu remiză
        }

        // 8.5. Schimbă jucătorul curent
        currentPlayer = !currentPlayer;
    }

    // **9. Închide conexiunile**
    close(client1_fd);
    close(client2_fd);
    close(server_fd);
    return 0;
}
