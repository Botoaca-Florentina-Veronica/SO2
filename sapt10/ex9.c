//problema test 3
//numara cuvintele si caracterele de pe fiecare linie introdusa de la tastatura de un client
//se vor folosi threaduri, avem maxim 10 clienti ce se pot conecta simultan

//server.c
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int numara_cuvinte(char *linie)
{
    int count = 0;
    char *cuv = malloc(sizeof(BUFFER_SIZE));  //alocam spatiu pentru un cuvant
    if(!cuv)
    {
        perror("Eroare la alocarea dinamica!!");
        exit(1);
    }
    char *token = strtok(linie, " ");  //imparte linia in functie de spatii

    while(token != NULL && strcmp(token, "\0") != 0)
    {
        if(sscanf(token, "%s", cuv) == 1)
        {
            count++;
        }
        token = strtok(NULL, " "); //treci la urmatorul element
    }
    free(cuv);
    return count;
}

int numara_caractere(char *linie)
{
    int i;
    int count = 0;
    for(i=0; linie[i]!= '\0'; i++)
    {
        count++;
    }
    return count;
}


void *handle_client(void *client_socket_ptr)
{
    int client_fd = * (int *) client_socket_ptr;
    free(client_socket_ptr);

    ssize_t bytes_read;
    char buffer[BUFFER_SIZE];

    while((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) != 0)
    {
        //imi citesc folosind functia read, linia pe care o introduce clientul de la tastatura
        //si imi retin in buffer aceasta linie, pe care mai apoi o analizez
        buffer[bytes_read] = '\0';  //adaugam caracterul terminator

        int nr_caractere = numara_caractere(buffer);
        int nr_cuvinte = numara_cuvinte(buffer);
        

        //!!FOARTE IMPORTANT!!!
        //mai sus ordinea apelarii functiilor e foarte importanta, daca aveam mai intai apelata functia
        //numara_cuvinte, si apoi functia numara_caractere, atunci nu s-ar fi numarat corect numarul de caractere
        //de ce? pentru ca in functia numara_cuvinte noi folosim strtok ca sa impartim sirul in functie de spatii, 
        //si deci sa numaram cuvintele, iar facand asta ni se creaza la fiecare cuvant un nou sir, pe care 
        //functia numara_cuvinte il va numara, in loc sa numere caracterele de pe intreaga linie

        //SOLUTII
        /*  putem face o copie temporară a lui buffer înainte de a apela numara_cuvinte
        char buffer_copy[BUFFER_SIZE];
        strcpy(buffer_copy, buffer); 

        int nr_cuvinte = numara_cuvinte(buffer_copy);
        int nr_caractere = numara_caractere(buffer);
        */

        //sau putem face o functie de numarare a cuvintelor diferita, in care doar numaram cate spatii avem,
        //deci numarul de cuvinte ar fi nr_spatii+1
        

        //imi supra-scriu in buffer raspunsul pentru client si il pregatesc pentru a-l trimite
        snprintf(buffer, sizeof(buffer), "Numar cuvinte: %d, numar caractere: %d\n", nr_cuvinte, nr_caractere-1);

        //imi trimit rezultatul catre client
        if(write(client_fd, buffer, strlen(buffer)) < 0)
        {
            perror("Eroare la write in server!!");
            break;
        }
    }

    if (bytes_read == -1)
    {
        perror("Eroare la read!!");
    }

    close(client_fd);
    return NULL;
}


int main(void)
{
    int server_fd;
    struct sockaddr_in server_bind, client_addr;

    //cream socket-ul pentru server
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Eroare la socket in server!!");
        exit(1);
    }

    //facem structura pentru server
    memset(&server_bind, 0, sizeof(server_bind));
    server_bind.sin_family = AF_INET;  //familia de protocoale ipv4
    server_bind.sin_port = htons(PORT);  //convertim adresa portului pentru a putea lucra cu ea
    server_bind.sin_addr.s_addr = INADDR_ANY;  //putem asculta orice adresa

    //facem legatura intre server, port si adresa
    if(bind(server_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0)
    {
        perror("Eroare la bind!!");
        exit(1);
    }

    //ascultam dupa conexiuni(putem avea maxim 10 clienti)
    if(listen(server_fd, 10) < 0)
    {
        perror("Eroare la listen!!");
        exit(1);
    }
    printf("Ascultam dupa conexiuni...");

    //bucla principala a programului
    while(1)
    {
        int *client_socket_ptr = malloc(sizeof(int));
        if(!client_socket_ptr)
        {
            perror("Eroare la alocarea dinamica!!");
            exit(1);
        }

        unsigned int client_addr_len = sizeof(client_addr);
        pthread_t thread;
        
        //acum realizez(accept) conexiunea dintre client si server
        if((*client_socket_ptr = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
            perror("Eroare la accept!!");
            free(client_socket_ptr);
            exit(1);
        }

        if(pthread_create(&thread, NULL, handle_client, client_socket_ptr) < 0)
        {
            perror("Eroare la pthread_create!!");
            exit(1);
        }

        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}

//client.c
#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void)
{
    int client_fd;
    struct sockaddr_in server_bind;

    //cream socket-ul pentru client
    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Eroare la socket in client!!");
        exit(1);
    }

    //facem structura pentru client
    memset(&server_bind, 0, sizeof(server_bind));
    server_bind.sin_family = AF_INET; 
    server_bind.sin_port = htons(PORT);

    //facem conexiunea intre server si client
    if(connect(client_fd, (struct sockaddr *)&server_bind, sizeof(server_bind)) < 0)
    {
        perror("Eroare la connect!!");
        exit(1);
    }

    printf("Client conectat la server, introdu o linie: ");
    //citim acum linia de la client si o retinem intr-un buffer
    char buffer[BUFFER_SIZE];
    
    while(fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        //ne trimitem linia catre server pentru a o analiza si primi un raspuns
        if(write(client_fd, buffer, strlen(buffer)) < 0)
        {
            perror("Eroare la write din client!!");
            exit(1);
        }

        //acum trebuie sa imi retin undeva raspunsul de la server
        ssize_t bytes_recieved = read(client_fd, buffer, sizeof(buffer)-1);
        if(bytes_recieved < 0)
        {
            perror("Eroare la citire!!");
            exit(1);
        }
        buffer[bytes_recieved] = '\0'; //imi adaug caracterul terminator

        //acum ca am informatia necesara, vreau sa imi afisez raspunsul pe terminalul clientului
        printf("Raspuns de la server: %s", buffer);
    }

    close(client_fd);
    return 0;
}
