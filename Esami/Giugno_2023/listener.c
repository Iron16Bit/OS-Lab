#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>
#include <fcntl.h>

// Handler dei segnali
void handler(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGUSR1)
    {
        printf("Ricevuto SIGUSR1\n");
    }
}

int main()
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    printf("[%d]\n", getpid());

    mkfifo("/tmp/file.fifo", 0777 | IPC_CREAT);
    int fd = open("/tmp/file.fifo", O_RDONLY);

    struct msg_buff
    {
        char mtype[100];
        char mtext[100];
    };

    // Creazione della coda
    key_t queueKey = ftok("tmp.txt", 1);
    // if (queueKey == -1)
    // {
    //     printf("La queue esiste già\n");
    //     exit(1);
    // }
    int queueId = msgget(queueKey, 0777 | IPC_CREAT);

    char buff[256];
    while (1)
    {
        struct msg_buff msg;
        msgrcv(queueId, &msg, 100, 0, 0);
        printf("Received message:\nType:%s\nText:%s\n", msg.mtype, msg.mtext);

        int r = read(fd, buff, sizeof(buff));
        if (r > 0)
        {
            printf("Read from pipe: %s\n", buff);
        }
    }

    return 0;
}