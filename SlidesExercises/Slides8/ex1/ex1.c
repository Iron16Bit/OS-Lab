/*
Impostare una comunicazione bidirezionale tra due processi con due livelli di
complessità:
    - Alternando almeno due scambi (P1 → P2, P2 → P1, P1 → P2, P2 → P1)
    - Estendendo il caso a mo’ di “ping-pong”, fino a un messaggio convenzionale di “fine comunicazione”
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define READ 0
#define WRITE 1

int stop = 0;

void myHandler(int sigNum)
{
    if (sigNum == SIGINT)
    {
        stop = 1;
        printf("Interrupt\n");
    }
}

int main()
{
    signal(SIGINT, myHandler);

    int pipe1[2]; // P -> reads, C -> writes
    int pipe2[2]; // P -> writes, C -> reads
    char buf1[50];
    char buf2[50];
    pipe(pipe1);
    pipe(pipe2);                     // Create two unnamed pipes
    write(pipe1[WRITE], "Hello", 5); // Writes to pipe

    int child = fork();

    if (child)
    {
        close(pipe1[READ]);
        close(pipe2[WRITE]);

        int stopPipe1 = 0;

        while (stopPipe1 != 1)
        {
            read(pipe2[READ], &buf1, 50); // Read from pipe
            printf("Child read: %s\n", buf1);

            if (strcmp(buf1, "STOP") == 0)
            {
                stopPipe1 = 1;
            }

            write(pipe1[WRITE], buf1, 50); // Writes to pipe

            if (stop == 1)
            {
                write(pipe1[WRITE], "STOP", 5); // Writes to pipe
            }
        }

        close(pipe1[WRITE]);
        close(pipe2[READ]);
    }
    else
    {
        close(pipe1[WRITE]);
        close(pipe2[READ]);

        int stopPipe2 = 0;

        while (stopPipe2 != 1)
        {
            read(pipe1[READ], &buf2, 50); // Read from pipe
            printf("Parent read: %s\n", buf2);

            if (strcmp(buf2, "STOP") == 0)
            {
                stopPipe2 = 1;
                printf("Read stop\n");
            }

            write(pipe2[WRITE], buf2, 50); // Writes to pipe

            if (stop == 1)
            {
                write(pipe2[WRITE], "STOP", 5); // Writes to pipe
            }
        }

        close(pipe1[READ]);
        close(pipe2[WRITE]);
    }

    while (wait(NULL) > 0)
        ;
}