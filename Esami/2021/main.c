#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/msg.h>

#include <signal.h>
#include <fcntl.h>

#include <pthread.h>

// Flag
int usr1, usr2 = 0;

void handler(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGUSR1)
    {
        usr1 = 1;
    }
    else if (signo == SIGUSR2)
    {
        usr2 = 1;
    }
}

struct msg_buff
{
    int mtype;
    char mtext[100];
};

int main(int argc, char *argv[])
{
    int n;

    if (argc != 3)
    {
        fprintf(stderr, "Usare come ./app <path> <n>\n");
        exit(1);
    }
    else
    {
        // Controllare che sia coinvertibile in un numero
        if (strspn(argv[2], "0123456789") != strlen(argv[2]))
        {
            fprintf(stderr, "%s non è un numero\n", argv[2]);
            exit(1);
        }

        n = atoi(argv[2]);
        if (n < 1 || n > 10)
        {
            fprintf(stderr, "Il numero %d deve essere compreso tra 1 e 10\n", n);
        }
    }

    char path[256];
    strcpy(path, argv[1]);
    strcat(path, "/info");

    int esito = mkdir(path, 0755);
    if (esito == -1)
    {
        fprintf(stderr, "Errore nella creazione del folder info\n");
        exit(1);
    }

    int pid;

    char keyPath[256];
    strcpy(keyPath, path);
    strcat(keyPath, "/key.txt");
    if (creat(keyPath, 0755) == -1)
    {
        fprintf(stderr, "Errore nella creazione di key.txt\n");
        exit(1);
    }

    FILE *fd = fopen(keyPath, "w");
    char tmp[100];
    sprintf(tmp, "%d\n", getpid());
    fputs(tmp, fd);
    fflush(fd);

    key_t queueKey = ftok(keyPath, 32);
    msgctl(queueKey, IPC_RMID, NULL);
    int queueId = msgget(queueKey, 0777 | IPC_CREAT);

    struct msg_buff msg;
    sprintf(msg.mtext, "%d", getpid());
    msg.mtype = 1;
    msgsnd(queueId, &msg, sizeof(msg.mtext), 0);

    for (int i = 0; i < n; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            char buff[100];
            sprintf(buff, "/%d.txt", getpid());
            strcat(path, buff);
            creat(path, 0755);

            struct sigaction sa;
            sa.sa_flags = SA_SIGINFO;
            sa.sa_sigaction = handler;
            sigemptyset(&sa.sa_mask);
            sigaction(SIGUSR1, &sa, NULL);
            sigaction(SIGUSR2, &sa, NULL);

            break;
        }
        else
        {
            if (i < n - 1)
            {
                printf("%d ", pid);
                fflush(stdout);
            }
            else
            {
                printf("%d\n", pid);
                fflush(stdout);
            }
        }
    }

    if (pid == 0)
    {
        FILE *childFd = fopen(path, "w");
        while (1)
        {
            if (usr1)
            {
                usr1 = 0;
                fputs("SIGURS1\n", childFd);
                fflush(childFd);
            }
            if (usr2)
            {
                usr2 = 0;
                sprintf(msg.mtext, "%d", getpid());
                msg.mtype = 1;
                msgsnd(queueId, &msg, sizeof(msg.mtext), 0);
            }
            pause();
        }
    }

    return 0;
}