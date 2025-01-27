#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080 // Portul pe care serverul va asculta conexiunile
#define NAME_SIZE 50 // Dimensiunea maximă a numelui unui jucător
#define BUFFER_SIZE 1024 // Dimensiunea bufferului pentru mesaje
#define SIDE 3 // Dimensiunea tablei de joc (3x3)

// Structură care definește starea jocului, incluzând tabla de joc și jucătorul curent ('X' sau 'O')
typedef struct GameState {
    char board[SIDE][SIDE]; // Tabla de joc
    char currentPlayer; // Jucătorul curent ('X' sau 'O')
} GameState;

// Structură care reprezintă un jucător, incluzând socket-ul, numele și pointerul către următorul jucător în coadă
typedef struct Player {
    int socket; // Socket-ul asociat cu jucătorul
    char name[NAME_SIZE]; // Numele jucătorului
    struct Player *next; // Pointer către următorul jucător din coadă
} Player;

// Structură care reprezintă o sesiune de joc între doi jucători, incluzând și starea jocului
typedef struct GameSession {
    Player player1; // Primul jucător
    Player player2; // Al doilea jucător
    GameState game_state; // Starea jocului pentru această sesiune
} GameSession;

// Mutex utilizat pentru a proteja accesul la coada de jucători
pthread_mutex_t player_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
// Coada de așteptare pentru jucători
Player *player_queue = NULL;

// Funcție care inițializează tabla de joc cu spații goale și setează primul jucător ('X')
void initializeBoard(GameState *gameState) 
{
    int i, j;
    for (i = 0; i < SIDE; i++) 
    {
        for (j = 0; j < SIDE; j++) 
        {
            gameState->board[i][j] = ' '; // Fiecare celulă este inițial goală
        }
    }
    gameState->currentPlayer = 'X'; // Jucătorul 'X' începe primul
}

// Funcție care trimite starea actuală a jocului către un client
void sendGameState(int clientSocket, GameState *gameState) 
{
    send(clientSocket, gameState, sizeof(GameState), 0); // Trimite întreaga structură GameState
}

// Funcție care adaugă un nou jucător în coada de așteptare
void add_player_to_queue(Player *new_player) 
{
    pthread_mutex_lock(&player_queue_mutex); // Blochează accesul exclusiv la coadă
    new_player->next = NULL; // Setează următorul jucător ca NULL

    if (!player_queue) 
    { // Dacă coada este goală
        player_queue = new_player; // Noul jucător devine capul cozii
    } 
    else 
    { // Dacă există deja jucători în coadă
        Player *current = player_queue;
        while (current->next) 
        { 
            // Merge la finalul cozii
            current = current->next;
        }
        current->next = new_player; // Adaugă noul jucător la final
    }
    pthread_mutex_unlock(&player_queue_mutex); // Deblochează accesul la coadă
}

// Funcție care formează o pereche de jucători din coada de așteptare
GameSession *pair_players() 
{
    pthread_mutex_lock(&player_queue_mutex); // Blochează accesul exclusiv la coadă
    if (!player_queue || !player_queue->next) 
    { 
        // Dacă nu sunt suficienți jucători în coadă
        pthread_mutex_unlock(&player_queue_mutex); // Deblochează accesul și returnează NULL
        return NULL;
    }

    // Preia primii doi jucători din coadă
    Player *player1 = player_queue;
    Player *player2 = player_queue->next;
    player_queue = player2->next; // Actualizează capul cozii pentru următorii jucători

    // Resetează pointerii de next pentru cei doi jucători
    player1->next = NULL;
    player2->next = NULL;

    // Creează o nouă sesiune de joc și asociază cei doi jucători
    GameSession *game_session = malloc(sizeof(GameSession));
    game_session->player1 = *player1;
    game_session->player2 = *player2;

    free(player1); // Eliberează memoria pentru primul jucător
    free(player2); // Eliberează memoria pentru al doilea jucător
    pthread_mutex_unlock(&player_queue_mutex); // Deblochează accesul la coadă

    return game_session; // Returnează sesiunea de joc
}

// Funcție care renderizează tabla de joc într-un format text (pentru afișare către jucători)
void render_board(char board[SIDE][SIDE], char *buffer) 
{
    snprintf(buffer, BUFFER_SIZE,
             " %c | %c | %c \n"
             "-----------\n"
             " %c | %c | %c \n"
             "-----------\n"
             " %c | %c | %c \n",
             board[0][0], board[0][1], board[0][2],
             board[1][0], board[1][1], board[1][2],
             board[2][0], board[2][1], board[2][2]); // Creează o reprezentare text a tablei de joc
}

// Funcție care verifică dacă există un câștigător pe baza stării curente a jocului
int checkWinner(GameState *gameState) 
{
    // Verifică rândurile
    for (int i = 0; i < SIDE; i++) 
    {
        if (gameState->board[i][0] == gameState->board[i][1] && gameState->board[i][1] == gameState->board[i][2] && gameState->board[i][0] != ' ') 
        {
            return 1; // Rând câștigător
        }
        if (gameState->board[0][i] == gameState->board[1][i] && gameState->board[1][i] == gameState->board[2][i] && gameState->board[0][i] != ' ') 
        {
            return 1; // Coloană câștigătoare
        }
    }
    // Verifică diagonalele
    if (gameState->board[0][0] == gameState->board[1][1] && gameState->board[1][1] == gameState->board[2][2] && gameState->board[0][0] != ' ') 
    {
        return 1; // Diagonală principală câștigătoare
    }
    if (gameState->board[0][2] == gameState->board[1][1] && gameState->board[1][1] == gameState->board[2][0] && gameState->board[0][2] != ' ') 
    {
        return 1; // Diagonală secundară câștigătoare
    }
    return 0; // Niciun câștigător
}

// Funcție care verifică dacă jocul este remiză (toate celulele sunt ocupate)
int checkDraw(GameState *gameState) 
{
    int i, j;
    for (i = 0; i < SIDE; i++) 
    {
        for (j = 0; j < SIDE; j++) 
        {
            if (gameState->board[i][j] == ' ') return 0; // Mai există celule libere, nu e remiză
        }
    }
    return 1; // Tabla este completă, jocul este remiză
}

// Fir de execuție care gestionează o sesiune de joc între doi jucători
void *game_session_thread(void *arg) 
{
    GameSession *game_session = (GameSession *)arg; // Obține sesiunea de joc din argument
    initializeBoard(&game_session->game_state); // Inițializează tabla de joc

    char buffer[BUFFER_SIZE];
    // Trimite notificări fiecărui jucător despre cine este adversarul său
    snprintf(buffer, BUFFER_SIZE, "You are paired with %s\n", game_session->player2.name);
    send(game_session->player1.socket, buffer, strlen(buffer), 0);

    snprintf(buffer, BUFFER_SIZE, "You are paired with %s\n", game_session->player1.name);
    send(game_session->player2.socket, buffer, strlen(buffer), 0);

    // Buclă principală a jocului
    while (1) 
    {
        // Determină jucătorul curent și adversarul
        Player *current_player = (game_session->game_state.currentPlayer == 'X') ? &game_session->player1 : &game_session->player2;
        Player *opponent_player = (game_session->game_state.currentPlayer == 'X') ? &game_session->player2 : &game_session->player1;

        // Afișează tabla de joc pentru ambii jucători
        char board_buffer[BUFFER_SIZE];
        render_board(game_session->game_state.board, board_buffer);
        send(current_player->socket, board_buffer, strlen(board_buffer), 0);
        send(opponent_player->socket, board_buffer, strlen(board_buffer), 0);

        // Solicită mutarea de la jucătorul curent
        snprintf(buffer, BUFFER_SIZE, "Your move (%s): ", current_player->name);
        send(current_player->socket, buffer, strlen(buffer), 0);

        int bytes_received = recv(current_player->socket, buffer, BUFFER_SIZE, 0); // Primește mutarea de la jucător
        if (bytes_received <= 0) 
        {
             // Dacă conexiunea cu jucătorul este pierdută
            snprintf(buffer, BUFFER_SIZE, "Player %s disconnected.\n", current_player->name);
            send(opponent_player->socket, buffer, strlen(buffer), 0); // Notifică adversarul
            break;
        }
        buffer[bytes_received] = '\0'; // Asigură terminatorul de șir

        int move = atoi(buffer); // Interpretează mutarea (ca număr)
        int row = (move - 1) / SIDE; // Calcul rând pe baza mutării
        int col = (move - 1) % SIDE; // Calcul coloană pe baza mutării

        if (move < 1 || move > 9 || game_session->game_state.board[row][col] != ' ') 
        { 
            // Verifică validitatea mutării
            send(current_player->socket, "Invalid move. Try again.\n", 25, 0);
            continue; // Solicită jucătorului să încerce din nou
        }

        // Actualizează tabla de joc cu mutarea jucătorului curent
        game_session->game_state.board[row][col] = game_session->game_state.currentPlayer;

        if (checkWinner(&game_session->game_state)) 
        {   // Verifică dacă există un câștigător
            snprintf(buffer, BUFFER_SIZE, "Player %s wins!\n", current_player->name);
            send(current_player->socket, buffer, strlen(buffer), 0);
            send(opponent_player->socket, buffer, strlen(buffer), 0);
            break; // Încheie sesiunea de joc
        }

        if (checkDraw(&game_session->game_state)) 
        {   // Verifică dacă jocul este remiză
            snprintf(buffer, BUFFER_SIZE, "It's a draw!\n");
            send(current_player->socket, buffer, strlen(buffer), 0);
            send(opponent_player->socket, buffer, strlen(buffer), 0);
            break; // Încheie sesiunea de joc
        }

        // Schimbă jucătorul curent ('X' devine 'O' și invers)
        game_session->game_state.currentPlayer = (game_session->game_state.currentPlayer == 'X') ? 'O' : 'X';
    }

    // Închide conexiunile și eliberează memoria pentru sesiune
    close(game_session->player1.socket);
    close(game_session->player2.socket);
    free(game_session);
    return NULL;
}

// Fir de execuție care gestionează conexiunea unui client
void *handle_client(void *arg) 
{
    int client_socket = *(int *)arg; // Preia socket-ul clientului din argument
    free(arg); // Eliberează memoria alocată pentru socket
    char buffer[BUFFER_SIZE];

    // Primește numele jucătorului
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    buffer[strcspn(buffer, "\n")] = 0; // Elimină newline-ul din nume
    printf("Player connected with name: %s\n", buffer);

    // Creează un nou jucător și îl adaugă în coada de așteptare
    Player *new_player = malloc(sizeof(Player));
    new_player->socket = client_socket;
    strcpy(new_player->name, buffer);
    add_player_to_queue(new_player);

    // Încearcă să formeze o pereche de jucători și să creeze o sesiune de joc
    GameSession *game_session = pair_players();
    if (game_session) 
    {
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, game_session_thread, game_session); // Creează un fir pentru sesiunea de joc
        pthread_detach(thread_id); // Detasează firul pentru a rula independent
    }

    return NULL;
}

// Funcția principală care rulează serverul
int main() 
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr; // Structuri pentru adresa serverului și clientului
    socklen_t client_addr_len = sizeof(client_addr);

    // Creează socket-ul serverului
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configurează adresa serverului
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Ascultă pe toate interfețele
    server_addr.sin_port = htons(PORT); // Setează portul

    // Leagă socket-ul la adresa specificată
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Pune serverul în modul de ascultare
    if (listen(server_socket, 10) < 0) 
    {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // Bucla principală a serverului pentru acceptarea conexiunilor
    while (1) 
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len); // Acceptă o conexiune de la un client
        if (client_socket < 0) 
        {
            perror("Client connection failed");
            continue; // Continuă cu următoarea conexiune
        }
        printf("New client connected.\n");

        // Creează un fir separat pentru a gestiona clientul
        pthread_t thread_id;
        int *client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_socket;
        pthread_create(&thread_id, NULL, handle_client, client_sock_ptr);
        pthread_detach(thread_id); // Detasează firul pentru a rula independent
    }

    close(server_socket); // Închide socket-ul serverului (nu se ajunge aici în mod normal)
    return 0;
}
