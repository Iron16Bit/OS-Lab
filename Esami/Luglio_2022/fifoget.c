#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/msg.h>

#include <fcntl.h>
#include <signal.h>

#include <pthread.h>

int main(int argc, char *argv[])
{
    // Controllo numero argomenti
    if (argc != 3)
    {
        fprintf(stderr, "Usare come ./<eseguibile> <path> <n>\n");
        exit(1);
    }

    // Controllo che sia possibile creare la fifo
    int esito = mkfifo(argv[1], S_IWUSR | S_IRUSR);
    if (esito == -1)
    {
        fprintf(stderr, "Impossibile creare la fifo\n");
        exit(2);
    }

    // Controllo che il numero sia un numero tra 0 e 10
    if (strspn(argv[2], "0123456789") != strlen(argv[2]))
    {
        fprintf(stderr, "Passare come secondo argomento un numero\n");
        exit(1);
    }
    int n = atoi(argv[2]);
    if (n < 0 || n > 10)
    {
        fprintf(stderr, "<n> deve essere compreso tra 0 e 10 estremi inclusi\n");
        exit(1);
    }

    // Apertura della fifo in lettura
    int fifo = open(argv[1], O_RDONLY);
    char buff;
    while (n > 0)
    {
        int r = read(fifo, &buff, sizeof(buff));
        if (r == -1)
        {
            fprintf(stderr, "Errore lettura\n");
            close(fifo);
            exit(1);
        }
        printf("%c\n", buff);
        n -= 1;
    }
    close(fifo);

    return 0;
}