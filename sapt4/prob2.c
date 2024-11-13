/*
Să se scrie un program C, care constă în 2 procese înrudite:
procesul copil: va invoca comanda "ls -l" și se va asigura că informația generată de această comandă 
este transmisă părintelui folosind un pipe
procesul părinte: va număra câți octeți a primit prin pipe, va afișa acest număr și după va prelua 
starea procesului copil.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) 
{
    int pipe_fd[2]; // Descriptori pentru pipe
    pid_t pid;
    
    // Creăm pipe-ul
    if (pipe(pipe_fd) == -1) 
    {
        perror("Eroare la crearea pipe-ului");
        exit(1);
    }
    
    // Creăm procesul copil
    pid = fork();
    if (pid == -1) 
    {
        perror("Eroare la fork");
        exit(1);
    }
    
    if (pid == 0) 
    { 
        // Cod pentru procesul copil
        close(pipe_fd[0]); // Închidem capătul de citire al pipe-ului în copil
        
        // Redirecționăm ieșirea standard a copilului către capătul de scriere al pipe-ului
        dup2(pipe_fd[1], STDOUT_FILENO);

        close(pipe_fd[1]); // Închidem capătul de scriere după redirecționare
        
        // Executăm comanda "ls -l"
        execlp("ls", "ls", "-l", NULL);
        
        // Dacă execlp eșuează
        perror("Eroare la execlp");
        exit(0);
    } 
    else 
    { 
        // Cod pentru procesul părinte
        close(pipe_fd[1]); // Închidem capătul de scriere al pipe-ului în părinte
        
        // Citim datele primite prin pipe și calculăm numărul de octeți
        char buffer[1024];
        int bytes_read;
        int total = 0;
        
        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) 
        {
            total = total + bytes_read;
        }
        
        // Închidem capătul de citire al pipe-ului
        close(pipe_fd[0]);
        
        // Afișăm numărul total de octeți primiți
        printf("Numărul de octeți primiți prin pipe: %d\n", total_bytes);
        
        // Așteptăm terminarea procesului copil și preluăm starea
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) 
        {
            printf("Procesul copil s-a terminat cu codul de ieșire: %d\n", WEXITSTATUS(status));
        }
    }
    
    return 0;
}

/*
   Funcția `execlp` din C este parte din familia de funcții `exec` folosite pentru a înlocui un proces curent
 cu un alt program. Când o funcție `exec` este apelată cu succes, programul curent este complet înlocuit 
 cu programul specificat. `execlp` se folosește în special atunci când dorim să rulăm un program sau un 
 fișier executabil din sistem, cum ar fi `gcc`, `ls`, `cat` etc., oferind o cale simplă de a rula comenzi 
 dintr-un program C.

### Sintaxa funcției `execlp`

```c
int execlp(const char *file, const char *arg, ..., (char *) NULL);
```

### Parametrii

1. **file**: Numele programului/executabilului pe care dorim să îl rulăm, de exemplu `"gcc"` sau `"ls"`. 
`execlp` va căuta automat acest program în directoarele specificate în variabila de mediu `PATH`, deci 
nu e nevoie de calea completă dacă programul e în unul din aceste directoare.

2. **arg**: Lista de argumente care trebuie trimise către programul specificat. Primul argument este în 
mod obișnuit numele programului, așa cum apare în linia de comandă (`argv[0]`). Următoarele argumente 
sunt opțiuni sau parametri pe care programul le va folosi.

3. **NULL**: Argumentul final trebuie să fie întotdeauna `(char *) NULL` pentru a marca sfârșitul listei
de argumente.

### Exemplu de utilizare

Să presupunem că dorim să rulăm o comandă `gcc` pentru a compila un fișier C (`program.c`) 
într-un executabil numit `program`.

```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Invocăm gcc pentru a compila "program.c" și a genera un executabil "program"
    execlp("gcc", "gcc", "program.c", "-o", "program", (char *) NULL);

    // Dacă execlp eșuează, codul de mai jos va fi executat
    perror("Eroare la execlp");
    exit(1);
}
```

### Explicații

1. **`execlp("gcc", "gcc", "program.c", "-o", "program", (char *) NULL);`**: 
    - Primul argument (`"gcc"`) este numele programului pe care vrem să-l rulăm.
    - Al doilea argument (`"gcc"`) este echivalent cu `argv[0]` în linia de comandă și, în mod obișnuit, 
    este identic cu numele programului.
    - Al treilea argument (`"program.c"`) este fișierul sursă de compilat.
    - Al patrulea argument (`"-o"`) și al cincilea argument (`"program"`) specifică numele executabilului 
    rezultat.
    - Ultimul argument, `(char *) NULL`, marchează sfârșitul listei de argumente.

2. **Ce se întâmplă dacă `execlp` reușește?** Dacă `execlp` reușește, programul curent este înlocuit 
de `gcc`, și codul de după `execlp` nu va fi executat.

3. **Ce se întâmplă dacă `execlp` eșuează?** Dacă apare o eroare (de exemplu, dacă `gcc` nu este găsit),
 `execlp` va returna `-1`, iar controlul se întoarce în programul original. În acest caz, mesajul de 
 eroare `Eroare la execlp` va fi afișat folosind `perror`, iar programul se va încheia cu `exit(1)`.

### De ce să folosim `execlp`?

`execlp` este util pentru că:

- **Nu necesită specificarea completă a căii**: `execlp` caută programul în directoarele specificate 
în variabila `PATH`.
- **Simplitate în lansarea programelor**: Ne permite să rulăm programe externe direct din codul C.
- **Înlocuiește procesul curent**: După apelul `execlp`, procesul curent este înlocuit de noul program, 
economisind resurse.

### Alte funcții `exec`

Funcția `execlp` este una dintre variantele din familia `exec`. Iată alte câteva opțiuni:

- **`execvp`**: Folosește un vector (un tablou de șiruri) pentru a specifica argumentele.
- **`execl`**: Necesită calea completă a programului, fără a verifica în `PATH`.
- **`execv`**: Funcționează cu vector de argumente și necesită calea completă.

De exemplu, `execvp` este frecvent utilizat în locul lui `execlp` atunci când avem argumentele 
într-un vector, deoarece este mai flexibil pentru procesarea listelor de argumente.
*/
