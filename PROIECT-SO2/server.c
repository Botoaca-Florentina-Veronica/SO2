#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SIDE 3       // Dimensiunea tablei de joc (3x3)
#define PORT 8080    // Portul pe care serverul ascultă conexiunile
#define MAX_CLIENTS 2 // Numărul maxim de clienți per joc

// **Arhitectura**
// Serverul este responsabil pentru:
// - Inițializarea tablei de joc.
// - Acceptarea conexiunilor de la clienți.
// - Sincronizarea și trimiterea tablei de joc către clienți.
// - Verificarea câștigătorului/remizei după fiecare mutare.

// Structură pentru gestionarea datelor unui joc
typedef struct {
    int client1_fd;                 // File descriptor pentru primul client
    int client2_fd;                 // File descriptor pentru al doilea client
    char client1_name[64];          // Numele primului jucător
    char client2_name[64];          // Numele celui de-al doilea jucător
    char board[SIDE][SIDE];         // Tabla de joc
    int currentPlayer;              // Jucătorul curent (0 sau 1)
} Game;

// Funcția de inițializare a tablei de joc
// Scop: Să reseteze tabla de joc la o stare inițială, cu spații libere
void initializeBoard(char board[SIDE][SIDE]) 
{
    int i, j;
    for (i = 0; i < SIDE; i++) 
    {
        for (j = 0; j < SIDE; j++) 
        {
            board[i][j] = ' ';  // Fiecare celulă este inițializată ca spațiu liber
        }
    }
}

// Funcția de trimitere a tablei către clienți
// Scop: Să sincronizeze starea tablei de joc cu ambii clienți
void sendBoardToClients(Game *game) 
{
    char boardStr[1024];
    snprintf(boardStr, sizeof(boardStr),
             "Tabla de joc:\n %c | %c | %c \n-----------\n %c | %c | %c \n-----------\n %c | %c | %c \n",
             game->board[0][0], game->board[0][1], game->board[0][2],
             game->board[1][0], game->board[1][1], game->board[1][2],
             game->board[2][0], game->board[2][1], game->board[2][2]);

    send(game->client1_fd, boardStr, strlen(boardStr), 0); // Trimite tabla către primul client
    send(game->client2_fd, boardStr, strlen(boardStr), 0); // Trimite tabla către al doilea client
}

// Funcția de verificare a câștigătorului
// Scop: Să determine dacă există un câștigător pe tabla de joc
int checkWinner(char board[SIDE][SIDE]) 
{
    int i;
    for (i = 0; i < SIDE; i++) 
    {
        // Verificare linii
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ')
        {
            return 1;
        }
        // Verificare coloane
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ')
        {
            return 1;
        }
    }
    // Verificare diagonale
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ')
    {
        return 1;
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ')
    {
        return 1;
    }
    return 0; // Niciun câștigător detectat
}

// Funcția de verificare a remizei
// Scop: Să determine dacă toate celulele sunt ocupate și jocul este remiză
int checkDraw(char board[SIDE][SIDE]) 
{
    int i, j;
    for (i = 0; i < SIDE; i++) 
    {
        for (j = 0; j < SIDE; j++) 
        {
            if (board[i][j] == ' ') return 0; // Mai există mutări disponibile
        }
    }
    return 1; // Toate celulele sunt ocupate, deci este remiză
}

// Funcția care gestionează un joc
// Scop: Să coordoneze jocul între doi jucători
void *handleGame(void *arg) 
{
    Game *game = (Game *)arg; // Conversia argumentului la structura Game
    char buffer[1024];
    int currentPlayerFD;

    initializeBoard(game->board);
    // După ce am acceptat conexiunile de la clienți, serverul trebuie să trimită și să primească mesaje
    // de la aceștia. De exemplu, serverul trimite tabla de joc ambilor clienți:
    sendBoardToClients(game);  

    while (1) 
    {
        currentPlayerFD = (game->currentPlayer == 0) ? game->client1_fd : game->client2_fd; // Determină jucătorul curent
        char *playerName = (game->currentPlayer == 0) ? game->client1_name : game->client2_name;

        // Așteaptă mutarea jucătorului curent
        bzero(buffer, sizeof(buffer));  // Resetează bufferul
        read(currentPlayerFD, buffer, sizeof(buffer)); 
        //În acest caz, read() citește mișcarea jucătorului curent 
        //(ca un număr) și o convertește într-o locație pe tabla de joc (linie și coloană)

        int move = atoi(buffer) - 1;    // Convertirea mutării în index
        int row = move / SIDE;          // Determină linia
        int col = move % SIDE;          // Determină coloana

        // Validarea mutării
        if (move < 0 || move >= SIDE * SIDE || game->board[row][col] != ' ') 
        {
            char *invalidMoveMsg = "Mutare invalidă! Încercați din nou.\n";
            send(currentPlayerFD, invalidMoveMsg, strlen(invalidMoveMsg), 0); // Notifică jucătorul despre mutarea invalidă
            continue;  // Se cere o mutare nouă
        }

        // Actualizarea tablei
        game->board[row][col] = (game->currentPlayer == 0) ? 'X' : 'O';
        sendBoardToClients(game); // Trimite tabla actualizată către clienți

        // Verificare câștigător/remiză
        if (checkWinner(game->board)) 
        {
            char winMsg[128], loseMsg[128];
            snprintf(winMsg, sizeof(winMsg), "%s a câștigat!\n", playerName); // Mesaj pentru câștigător
            snprintf(loseMsg, sizeof(loseMsg), "%s a pierdut!\n", (game->currentPlayer == 0) ? game->client2_name : game->client1_name); // Mesaj pentru învins

            send(currentPlayerFD, winMsg, strlen(winMsg), 0); // Trimite mesajul câștigătorului
            send((currentPlayerFD == game->client1_fd) ? game->client2_fd : game->client1_fd, loseMsg, strlen(loseMsg), 0); // Trimite mesajul învinsului
            break; // Termină jocul
        }

        if (checkDraw(game->board)) 
        {
            char *drawMsg = "Remiză!\n";
            send(game->client1_fd, drawMsg, strlen(drawMsg), 0); // Notifică ambii jucători despre remiză
            send(game->client2_fd, drawMsg, strlen(drawMsg), 0);
            break; // Jocul s-a terminat cu remiză
        }

        // Schimbă jucătorul curent
        game->currentPlayer = !game->currentPlayer;
    }

    // Închide conexiunile la finalul jocului
    close(game->client1_fd);
    close(game->client2_fd);
    free(game); // Eliberare memorie alocată pentru joc
    pthread_exit(NULL); // Terminarea threadului
}

int main() 
{
    int server_fd;
    struct sockaddr_in address; // Structura pentru configurarea serverului
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("Socket failed"); // Eroare la crearea socketului
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET; // Setează familia de adrese la IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Ascultă pe toate interfețele
    address.sin_port = htons(PORT); // Setează portul (8080) pe care serverul va asculta
    // Funcția htons convertește portul într-un format adecvat rețelei (Network Byte Order)

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    {
        // După ce socket-ul este creat, serverul trebuie să se asocieze cu o adresă IP și un port
        // Acesta este procesul de bind (legare) care asociază socketul cu adresa și portul specificate anterior
        perror("Bind failed"); // Eroare la legarea socketului de adresă/port
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) 
    {
        // Serverul începe să asculte pentru conexiuni multiple pe socket-ul configurat,
        // cu o coadă de așteptare specificată, max 10
        perror("Listen failed"); // Eroare la configurarea serverului pentru a asculta conexiuni
        exit(EXIT_FAILURE);
    }

    printf("Server pornit, aștept conexiuni...\n");

    while (1) 
    {
        Game *game = (Game *)malloc(sizeof(Game)); // Alocare memorie pentru un nou joc
        pthread_t thread_id;

        // Acceptă primul jucător
        game->client1_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        read(game->client1_fd, game->client1_name, sizeof(game->client1_name)); // Citește numele jucătorului 1
        printf("Jucătorul 1 conectat: %s\n", game->client1_name);

        // Acceptă al doilea jucător
        game->client2_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        read(game->client2_fd, game->client2_name, sizeof(game->client2_name)); // Citește numele jucătorului 2
        printf("Jucătorul 2 conectat: %s\n", game->client2_name);

        /*
            Funcția accept(server_fd, ...) blochează execuția până când un client se conectează.
         Când un client se conectează, accept returnează un descriptor de fișier (client1_fd pentru primul client).
         După ce conexiunea este stabilită, se citește numele jucătorului de la client folosind funcția read.
         Același proces se repetă pentru al doilea client, iar descriptorul pentru acesta este salvat în client2_fd.
         Mesajele de succes ("Jucătorul 1 conectat: <nume>" și "Jucătorul 2 conectat: <nume>") sunt afișate pentru
         a confirma conectarea cu succes a ambilor jucători.


         client1_fd și client2_fd sunt „canalele” prin care serverul comunică cu fiecare dintre cei doi clienți
        */


        // Inițializează jocul
        game->currentPlayer = 0; // Jucătorul 1 începe

        // Creează un thread pentru a gestiona jocul
        pthread_create(&thread_id, NULL, handleGame, (void *)game);
        pthread_detach(thread_id); // Eliberare automată a resurselor threadului
    }

    close(server_fd); // Închide socketul serverului
    return 0;
}
