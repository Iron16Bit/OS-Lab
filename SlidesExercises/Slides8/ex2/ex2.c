/*
Creare un programma che prenda come argomento 'n' il numero di figli da
generare. Ogni figlio creato comunicherà al genitore (tramite pipe) un numero
casuale e il genitore calcolerà la somma di tutti questi numeri, inviandola a
stdout.
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <wait.h>
#include <signal.h>

// Following 2 needed for random number generation
#include <time.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 2 || !strspn(argv[1], "0123456789") == strlen(argv[1]))
    { // CHeck that we pass an argument and that it is a number
        printf("Must pass as argument only the desired number of children\n");
        return 1;
    }

    printf("Desired number of children: %d\n", atoi(argv[1]));

    int fd;
    char *fifoName = "/tmp/fifo";
    mkfifo(fifoName, S_IRUSR | S_IWUSR);

    int total = 0;
    char buf[10];

    for (int i = 0; i < atoi(argv[1]); i++)
    {
        int children = fork();
        if (children == 0)
        {
            fd = open(fifoName, O_WRONLY);
            srand(time(NULL) ^ (getpid() << 16));
            int generatedNumber = rand() % 10 + 1;

            char tmp[50];
            sprintf(tmp, "%d", generatedNumber); // Converts int to string

            printf("Child_%d generated number %d\n", i + 1, generatedNumber);

            write(fd, tmp, strlen(tmp) + 1);
            close(fd);
            exit(EXIT_SUCCESS);
        }
        else
        {
            fd = open(fifoName, O_RDONLY);
            read(fd, buf, sizeof(buf));
            total += atoi(buf);
            close(fd);
        }
    }

    printf("Parent calculated %d as total sum\n", total);

    return 0;
}