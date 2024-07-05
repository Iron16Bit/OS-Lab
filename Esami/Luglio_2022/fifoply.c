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

    int cifre = 0;
    int i;
    for (i = 0; i < 10; i++)
    {
        int r = read(fifo, buff[i], sizeof(buff[i]));
        if (r == -1)
        {
            break;
        }
        char c = buff[i][0];
        if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9') {
            cifre += 1;
        }
        if (cifre == n) {
            break;
        }
    }

    int totale = 0;
    char tmp[100] = "";
    for (int j = i - 1; j >= 0; j--)
    {
        char c = buff[j][0];
        if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9')
        {
            // è una cifra
            char singleChar[2];
            singleChar[0] = c;
            singleChar[1] = 0;
            strcat(tmp, singleChar);
            strcat(tmp, "+");
            totale += (c - '0');
        }
        else
        {
            // è una lettera
            printf("%c\n", buff[j][0]);
        }
    }

    tmp[strlen(tmp) - 1] = '=';
    char cifreTotale[100];
    sprintf(cifreTotale, "%d", totale);
    strcat(tmp, cifreTotale);
    printf("%s\n", tmp);

    if (cifre == n)
    {
        return 0;
    }
    else
    {
        exit(1);
    }
}