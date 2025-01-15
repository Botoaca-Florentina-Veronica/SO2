//server.c
/*
Client server
Serveru trimite timpul serverului din secunda in secunda
Serveru poate gestiona max 10 clienti
Clientu se conecteaza la server
Clientu printeaza timpul serverului
Clientu poate trimite la server un nr intre 0-10 ca sa regleze cat de des sa se trimita acele update-uri de la server
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 8080              // Portul pe care serverul ascultă conexiuni
#define BUFFER_SIZE 1024       // Dimensiunea buffer-ului pentru mesaje
#define MAX_CLIENTS 10         // Numărul maxim de clienți care pot fi conectați

// Structură pentru a stoca informațiile despre client
typedef struct {
    int client_fd;              // Descriptorul de fișier pentru socketul clientului
    int update_frequency;        // Frecvența de actualizare în secunde
} client_info_t;

// Funcție pentru a trimite timpul curent unui client
void send_time_to_client(int client_fd) 
{
    char buffer[BUFFER_SIZE];
    time_t current_time = time(NULL); // Obține timpul curent
    snprintf(buffer, sizeof(buffer), "Timpul serverului: %s", ctime(&current_time)); // Formatează timpul
    write(client_fd, buffer, strlen(buffer)); // Trimite timpul clientului
}

// Funcție pentru a gestiona comunicarea cu un client
void *handle_client(void *client_data) 
{
    client_info_t *client_info = (client_info_t *)client_data; // Obține informațiile despre client
    int client_fd = client_info->client_fd; // Descriptorul socketului clientului
    int update_frequency = client_info->update_frequency; // Frecvența de actualizare
    free(client_data); // Eliberează memoria alocată pentru client_info

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while (1) 
    {
        send_time_to_client(client_fd); // Trimite timpul curent
        sleep(update_frequency); // Așteaptă frecvența specificată

        // Verifică dacă clientul a trimis un mesaj
        bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) 
        {
            buffer[bytes_read] = '\0'; // Termină șirul
            int new_frequency;

            // Verifică dacă mesajul este un număr valid pentru frecvență
            if (sscanf(buffer, "%d", &new_frequency) == 1 && new_frequency >= 0 && new_frequency <= 10) 
            {
                update_frequency = new_frequency; // Actualizează frecvența
                snprintf(buffer, sizeof(buffer), "Frecvența actualizată la %d secunde.\n", update_frequency);
                write(client_fd, buffer, strlen(buffer)); // Trimite confirmarea clientului
            } 
            else 
            {
                snprintf(buffer, sizeof(buffer), "Frecvența invalidă. Introduceți un număr între 0 și 10.\n");
                write(client_fd, buffer, strlen(buffer)); // Trimite mesaj de eroare clientului
            }
        } 
        else if (bytes_read == 0) 
        {
            printf("Clientul s-a deconectat.\n");
            break; // Iese din buclă dacă clientul s-a deconectat
        }
    }

    close(client_fd); // Închide socketul clientului
    return NULL;
}

// Funcția principală a serverului
int main(void) 
{
    int server_fd;
    struct sockaddr_in server_bind, client_addr;
    pthread_t threads[MAX_CLIENTS]; // Array pentru a stoca thread-urile
    int active_clients = 0; // Numărul de clienți activi

    // Creează socketul
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Eroare la socket!");
        exit(1);
    }

    memset(&server_bind, 0, sizeof(server_bind)); // Inițializează structura sockaddr_in
    server_bind.sin_family = AF_INET; // Tipul de adresă
    server_bind.sin_addr.s_addr = INADDR_ANY; // Acceptă conexiuni de la orice adresă
    server_bind.sin_port = htons(PORT); // Setează portul

    // Leagă socketul la adresă
    if (bind(server_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0) 
    {
        perror("Eroare la bind!");
        exit(1);
    }

    // Ascultă pentru conexiuni
    if (listen(server_fd, MAX_CLIENTS) < 0) 
    {
        perror("Eroare la listen!");
        exit(1);
    }
    printf("Serverul ascultă conexiuni...\n");

    // Loop principal pentru a accepta clienți
    while (1) 
    {
        if (active_clients >= MAX_CLIENTS) 
        {
            printf("Serverul este ocupat. Aștept să se elibereze un loc.\n");
            sleep(1); // Așteaptă un moment înainte de a verifica din nou
            continue;
        }

        unsigned int client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) 
        {
            perror("Eroare la accept!");
            continue;
        }

        printf("Client conectat.\n");
        client_info_t *client_info = malloc(sizeof(client_info_t)); // Alocă memorie pentru informațiile clientului
        if (!client_info) 
        {
            perror("Eroare la alocarea dinamică!");
            close(client_fd);
            continue;
        }

        client_info->client_fd = client_fd; // Setează descriptorul socketului clientului
        client_info->update_frequency = 1; // Frecvența implicită: 1 secundă

        // Creează un thread pentru a gestiona clientul
        if (pthread_create(&threads[active_clients], NULL, handle_client, client_info) != 0) 
        {
            perror("Eroare la pthread_create!");
            free(client_info); // Eliberează memoria alocată
            close(client_fd);
            continue;
        }
        pthread_detach(threads[active_clients]); // Detach thread-ul pentru a nu bloca
        active_clients++; // Crește numărul de clienți activi
    }

    close(server_fd); // Închide socketul serverului
    return 0;
}


//client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080              // Portul serverului la care se va conecta clientul
#define BUFFER_SIZE 1024       // Dimensiunea buffer-ului pentru mesaje

int main(void) 
{
    int client_fd;                     // Descriptorul socketului clientului
    struct sockaddr_in server_addr;    // Structura pentru adresa serverului
    char buffer[BUFFER_SIZE];           // Buffer pentru a stoca mesajele

    // Creează un socket pentru conexiune
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Eroare la crearea socketului!"); // Mesaj de eroare dacă socketul nu poate fi creat
        exit(1);
    }

    // Inițializează structura sockaddr_in pentru server
    memset(&server_addr, 0, sizeof(server_addr)); // Curăță structura
    server_addr.sin_family = AF_INET;             // Tipul de adresă
    server_addr.sin_port = htons(PORT);            // Setează portul

    // Conectează clientul la server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("Eroare la conectare!"); // Mesaj de eroare dacă conectarea eșuează
        exit(1);
    }

    printf("Conectat la server. Timpul serverului va fi primit la fiecare secundă.\n");
    printf("Introduceți o valoare între 0 și 10 pentru a schimba frecvența actualizării:\n");

    fd_set read_fds; // Setul de descriere a fișierelor pentru select
    int max_fd = client_fd > fileno(stdin) ? client_fd : fileno(stdin); // Determină descriptorul maxim

    // Loop principal pentru a gestiona interacțiunea cu serverul și inputul utilizatorului
    while (1) 
    {
        FD_ZERO(&read_fds); // Curăță setul de descriere a fișierelor
        FD_SET(client_fd, &read_fds); // Adaugă socketul clientului
        FD_SET(fileno(stdin), &read_fds); // Adaugă stdin (inputul standard)

        // Așteaptă pentru activitate pe socketuri
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) 
        {
            perror("Eroare la select!"); // Mesaj de eroare pentru select
            break;
        }

        // Verifică dacă serverul a trimis un mesaj
        if (FD_ISSET(client_fd, &read_fds)) 
        {
            ssize_t bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1); // Citește mesajul de la server
            if (bytes_received <= 0) 
            {
                printf("Conexiunea cu serverul s-a pierdut.\n"); // Mesaj de deconectare
                break;
            }
            buffer[bytes_received] = '\0'; // Termină șirul
            printf("Server: %s", buffer); // Afișează mesajul de la server
        }

        // Verifică dacă utilizatorul a introdus un mesaj
        if (FD_ISSET(fileno(stdin), &read_fds)) 
        {
            if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) 
            { // Citește inputul utilizatorului
                // Trimite mesajul utilizatorului către server
                if (write(client_fd, buffer, strlen(buffer)) < 0) 
                {
                    perror("Eroare la trimiterea datelor!"); // Mesaj de eroare pentru scriere
                    break;
                }
            }
        }
    }

    close(client_fd); // Închide socketul clientului
    return 0; // Termină programul
}
