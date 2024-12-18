#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

// Structura pentru a transmite parametrii thread-urilor
typedef struct {
    int sockfd;
} ThreadArgs;

void *send_thread(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    int sockfd = threadArgs->sockfd;
    char buffer[BUFFER_SIZE];

    // Citire linie de la tastatură și trimitere către server
    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        if (write(sockfd, buffer, strlen(buffer)) < 0) {
            perror("write");
            break;
        }
    }

    // Închide conexiunea după terminarea citirii de la stdin
    shutdown(sockfd, SHUT_WR);
    return NULL;
}

void *receive_thread(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    int sockfd = threadArgs->sockfd;
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    // Citire mesaje de la server și afișare pe consolă
    while ((bytesRead = read(sockfd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytesRead] = '\0'; // Asigură terminatorul de șir
        printf("Server: %s", buffer);
    }

    if (bytesRead < 0) {
        perror("read");
    }

    return NULL;
}

int main(void) {
    int sockfd;
    struct sockaddr_in server;

    // Creare socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    // Configurare adresă server
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(4555);

    // Conectare la server
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    printf("Connected to the server.\n");

    // Creare threaduri
    pthread_t sendThread, receiveThread;
    ThreadArgs threadArgs = {sockfd};

    if (pthread_create(&sendThread, NULL, send_thread, &threadArgs) != 0) {
        perror("pthread_create send");
        close(sockfd);
        exit(1);
    }

    if (pthread_create(&receiveThread, NULL, receive_thread, &threadArgs) != 0) {
        perror("pthread_create receive");
        close(sockfd);
        exit(1);
    }

    // Așteptare terminare threaduri
    pthread_join(sendThread, NULL);
    pthread_join(receiveThread, NULL);

    // Închidere socket
    close(sockfd);

    return 0;
}


/*
Mai jos este un exemplu de program în C care implementează cerința utilizând două threaduri: unul pentru a citi de la tastatură și a trimite datele către server prin socket, iar celălalt pentru a citi mesajele de la server și a le afișa pe ecran. Pentru a realiza acest lucru, am utilizat biblioteca `pthread` pentru gestionarea threadurilor.

### Explicație

1. **Structura programului**:
   - **`send_thread`**: Citește date de la tastatură și le trimite serverului. Când se termină intrarea standard (EOF), închide partea de scriere a socketului folosind `shutdown`.
   - **`receive_thread`**: Citește date de la server și le afișează pe consolă până când conexiunea este închisă de server.

2. **Sincronizare**:
   - Cele două threaduri funcționează independent unul de celălalt. Dacă un thread se termină, celălalt poate continua până când nu mai are date de procesat.

3. **Comportament pe multiple terminale**:
   - Într-un terminal, rulați serverul folosind comanda:
     ```bash
     nc -l -p 4555 -s 127.0.0.1 -v
     ```
   - În alt terminal, rulați programul client:
     ```bash
     ./program
     ```

4. **Mesaje de la server**:
   - Programul client afișează orice mesaj trimis de server prin socket. 

Acest cod permite testarea unei comunicări bidirecționale între client și server utilizând două threaduri pentru gestionarea intrării și ieșirii.

*/
