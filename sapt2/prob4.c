/*
Scrie un program in C in care parcurgi un director si daca gasesc un regular file 
si are dimensiunea mai are ca 10 KB sa fac legatura simbolica si sa o denumesc cu 
numele fisierului initial plus extensia “mylink”
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define SIZE_LIMIT 10240  // 10 KB în bytes

int main(int argc, char **argv) 
{
    if (argc != 2) 
    {
        perror("Numar incorect de argumente in linia de comanda!!");
        exit(1);
    }

    DIR *dir = opendir(argv[1]);
    if (dir == NULL) 
    {
        perror("Eroare la deschiderea directorului!!");
        exit(1);
    }

    struct dirent *entry;
    struct stat st;

    // Parcurgem directorul
    while ((entry = readdir(dir)) != NULL) 
    {
        char file_path[BUFFER_SIZE];

        // Construim calea completă a fișierului
        snprintf(file_path, sizeof(file_path), "%s/%s", argv[1], entry->d_name);

        if (stat(file_path, &st) < 0) 
        {
            perror("Eroare la stat!!");
            continue;
        }

        // Verificăm dacă e un fișier regular și dimensiunea sa
        if (S_ISREG(st.st_mode) && st.st_size > SIZE_LIMIT) 
        {
            char link_name[BUFFER_SIZE];

            // Construim numele legăturii simbolice
            snprintf(link_name, sizeof(link_name), "%s/%s.mylink", argv[1], entry->d_name);

            // Creăm legătura simbolică
            if (symlink(file_path, link_name) < 0) 
            {
                perror("Eroare la crearea legaturii simbolice!!");
            } 
            else 
            {
                printf("Legatura simbolica creata: %s -> %s\n", link_name, file_path);
            }
        }
    }

    closedir(dir);
    return 0;
}
