#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define NAME_SIZE 50
#define BUFFER_SIZE 1024
#define SIDE 3

typedef struct GameState {
    char board[SIDE][SIDE];
    char currentPlayer; // 'X' or 'O'
} GameState;

typedef struct Player {
    int socket;
    char name[NAME_SIZE];
    struct Player *next;
} Player;

typedef struct GameSession {
    Player player1;
    Player player2;
    GameState game_state;
} GameSession;

pthread_mutex_t player_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
Player *player_queue = NULL;

void initializeBoard(GameState *gameState) {
    for (int i = 0; i < SIDE; i++) {
        for (int j = 0; j < SIDE; j++) {
            gameState->board[i][j] = ' ';
        }
    }
    gameState->currentPlayer = 'X'; // X starts first
}

void sendGameState(int clientSocket, GameState *gameState) {
    send(clientSocket, gameState, sizeof(GameState), 0);
}

void add_player_to_queue(Player *new_player) {
    pthread_mutex_lock(&player_queue_mutex);
    new_player->next = NULL;
    if (!player_queue) {
        player_queue = new_player;
    } else {
        Player *current = player_queue;
        while (current->next) {
            current = current->next;
        }
        current->next = new_player;
    }
    pthread_mutex_unlock(&player_queue_mutex);
}

GameSession *pair_players() {
    pthread_mutex_lock(&player_queue_mutex);
    if (!player_queue || !player_queue->next) {
        pthread_mutex_unlock(&player_queue_mutex);
        return NULL;
    }

    Player *player1 = player_queue;
    Player *player2 = player_queue->next;
    player_queue = player2->next;

    player1->next = NULL;
    player2->next = NULL;

    GameSession *game_session = malloc(sizeof(GameSession));
    game_session->player1 = *player1;
    game_session->player2 = *player2;

    free(player1);
    free(player2);
    pthread_mutex_unlock(&player_queue_mutex);

    return game_session;
}

void render_board(char board[SIDE][SIDE], char *buffer) {
    snprintf(buffer, BUFFER_SIZE,
             " %c | %c | %c \n"
             "-----------\n"
             " %c | %c | %c \n"
             "-----------\n"
             " %c | %c | %c \n",
             board[0][0], board[0][1], board[0][2],
             board[1][0], board[1][1], board[1][2],
             board[2][0], board[2][1], board[2][2]);
}

int checkWinner(GameState *gameState) {
    for (int i = 0; i < SIDE; i++) {
        if (gameState->board[i][0] == gameState->board[i][1] && gameState->board[i][1] == gameState->board[i][2] && gameState->board[i][0] != ' ') {
            return 1;
        }
        if (gameState->board[0][i] == gameState->board[1][i] && gameState->board[1][i] == gameState->board[2][i] && gameState->board[0][i] != ' ') {
            return 1;
        }
    }
    if (gameState->board[0][0] == gameState->board[1][1] && gameState->board[1][1] == gameState->board[2][2] && gameState->board[0][0] != ' ') {
        return 1;
    }
    if (gameState->board[0][2] == gameState->board[1][1] && gameState->board[1][1] == gameState->board[2][0] && gameState->board[0][2] != ' ') {
        return 1;
    }
    return 0;
}

int checkDraw(GameState *gameState) {
    for (int i = 0; i < SIDE; i++) {
        for (int j = 0; j < SIDE; j++) {
            if (gameState->board[i][j] == ' ') return 0;
        }
    }
    return 1;
}

void *game_session_thread(void *arg) {
    GameSession *game_session = (GameSession *)arg;
    initializeBoard(&game_session->game_state);

    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "You are paired with %s\n", game_session->player2.name);
    send(game_session->player1.socket, buffer, strlen(buffer), 0);

    snprintf(buffer, BUFFER_SIZE, "You are paired with %s\n", game_session->player1.name);
    send(game_session->player2.socket, buffer, strlen(buffer), 0);

    while (1) {
        Player *current_player = (game_session->game_state.currentPlayer == 'X') ? &game_session->player1 : &game_session->player2;
        Player *opponent_player = (game_session->game_state.currentPlayer == 'X') ? &game_session->player2 : &game_session->player1;

        char board_buffer[BUFFER_SIZE];
        render_board(game_session->game_state.board, board_buffer);
        send(current_player->socket, board_buffer, strlen(board_buffer), 0);
        send(opponent_player->socket, board_buffer, strlen(board_buffer), 0);

        snprintf(buffer, BUFFER_SIZE, "Your move (%s): ", current_player->name);
        send(current_player->socket, buffer, strlen(buffer), 0);

        int bytes_received = recv(current_player->socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            snprintf(buffer, BUFFER_SIZE, "Player %s disconnected.\n", current_player->name);
            send(opponent_player->socket, buffer, strlen(buffer), 0);
            break;
        }
        buffer[bytes_received] = '\0';

        int move = atoi(buffer);
        int row = (move - 1) / SIDE;
        int col = (move - 1) % SIDE;

        if (move < 1 || move > 9 || game_session->game_state.board[row][col] != ' ') {
            send(current_player->socket, "Invalid move. Try again.\n", 25, 0);
            continue;
        }

        game_session->game_state.board[row][col] = game_session->game_state.currentPlayer;

        if (checkWinner(&game_session->game_state)) {
            snprintf(buffer, BUFFER_SIZE, "Player %s wins!\n", current_player->name);
            send(current_player->socket, buffer, strlen(buffer), 0);
            send(opponent_player->socket, buffer, strlen(buffer), 0);
            break;
        }

        if (checkDraw(&game_session->game_state)) {
            snprintf(buffer, BUFFER_SIZE, "It's a draw!\n");
            send(current_player->socket, buffer, strlen(buffer), 0);
            send(opponent_player->socket, buffer, strlen(buffer), 0);
            break;
        }

        game_session->game_state.currentPlayer = (game_session->game_state.currentPlayer == 'X') ? 'O' : 'X';
    }

    close(game_session->player1.socket);
    close(game_session->player2.socket);
    free(game_session);
    return NULL;
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];

    recv(client_socket, buffer, BUFFER_SIZE, 0);
    buffer[strcspn(buffer, "\n")] = 0;
    printf("Player connected with name: %s\n", buffer);

    Player *new_player = malloc(sizeof(Player));
    new_player->socket = client_socket;
    strcpy(new_player->name, buffer);
    add_player_to_queue(new_player);

    GameSession *game_session = pair_players();
    if (game_session) 
    {
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, game_session_thread, game_session);
        pthread_detach(thread_id);
    }

    return NULL;
}

int main() 
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0) 
    {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) 
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) 
        {
            perror("Client connection failed");
            continue;
        }
        printf("New client connected.\n");

        pthread_t thread_id;
        int *client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_socket;
        pthread_create(&thread_id, NULL, handle_client, client_sock_ptr);
        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}
