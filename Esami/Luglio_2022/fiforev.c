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

    int fifo = open(argv[1], O_RDONLY);
    char buff[10][1];

    int error = 0;

    int i;
    for (i = 0; i < n; i++)
    {
        int r = read(fifo, buff[i], sizeof(buff[i]));
        if (r == -1)
        {
            fprintf(stderr, "Errore lettura\n");
            close(fifo);
            error = 1;
            break;
        }
    }

    close(fifo);

    for (int k = i - 1; k >= 0; k--)
    {
        printf("%c\n", buff[k][0]);
    }

    if (error == 0)
    {
        return 0;
    }
    else
    {
        exit(1);
    }

    return 0;
}