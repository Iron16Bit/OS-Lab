#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/msg.h>

#include <signal.h>
#include <fcntl.h>

#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

char instruction[3][256];

// Flag per i thread
int f_kill, f_queue, f_fifo, quit = 0;

// Flag ack SIGURS1
int usr1 = 0;

// Handler dei segnali
void handler(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGUSR1)
    {
        usr1 = 1;
        printf("Ricevuto SIGUSR1\n");
    }
}

void *kill_t(void *param)
{
    while (quit == 0)
    {
        if (f_kill)
        {
            f_kill = 0;
            kill(atoi(instruction[2]), atoi(instruction[1]));
            printf("[Kill]\nSent %s to %s\n", instruction[1], instruction[2]);
        }
    }
}

void *queue_t(void *param)
{
    struct msg_buff
    {
        char mtype[100];
        char mtext[100];
    };

    // Creazione della coda
    key_t queueKey = ftok((char *)param, 1);
    int queueId = msgget(queueKey, 0777 | IPC_CREAT);

    while (quit == 0)
    {
        if (f_queue)
        {
            f_queue = 0;

            struct msg_buff msg;
            strcpy(msg.mtype, instruction[1]);
            strcpy(msg.mtext, instruction[2]);
            printf("[Message]\nType:%s\nText:%s\n", instruction[1], instruction[2]);
            msgsnd(queueId, &msg, sizeof(msg.mtext), 0);
        }
    }
}

void *fifo_t(void *param)
{
    while (quit == 0)
    {
        if (f_fifo)
        {
            f_fifo = 0;

            mkfifo(instruction[1], O_CREAT | 0777);

            // Fifo aperta in scrittura
            int fd = open(instruction[1], O_WRONLY);
            write(fd, instruction[2], strlen(instruction[2]) + 1);
        }
    }
}

int main(int argc, char *argv[])
{
    printf("[%d]\n", getpid());
    if (argc != 2)
    {
        fprintf(stderr, "Usare come ./a.out /path/to/file\n");
        exit(1);
    }

    FILE *fd = fopen(argv[1], "r");
    if (fd == NULL)
    {
        fprintf(stderr, "Il percorso per il file non è esistente\n");
        exit(1);
    }

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    pthread_t t1;
    pthread_create(&t1, NULL, kill_t, NULL);

    pthread_t t2;
    char arg[256];
    strcpy(arg, argv[1]);
    pthread_create(&t2, NULL, queue_t, (void *)arg);

    pthread_t t3;
    pthread_create(&t3, NULL, fifo_t, NULL);

    char buff[256];

    while (fscanf(fd, "%s %s %s", instruction[0], instruction[1], instruction[2]) != EOF)
    {

        // Legge istruzione per istruzione dal file
        printf("READ: %s %s %s\n", instruction[0], instruction[1], instruction[2]);

        if (strcmp(instruction[0], "kill") == 0)
        {
            f_kill = 1;
        }
        else if (strcmp(instruction[0], "queue") == 0)
        {
            f_queue = 1;
        }
        else if (strcmp(instruction[0], "fifo") == 0)
        {
            f_fifo = 1;
        }
        else
        {
            printf("%s non riconosciuto\n", instruction[0]);
        }

        while (usr1 == 0)
        {
            pause();
        }
        usr1 = 0;
    }

    quit = 1;

    return 0;
}