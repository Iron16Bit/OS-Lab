#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>
#include <fcntl.h>

#include <pthread.h>

struct msg_buff
{
    int mtype;
    char mtext[100];
};

int usr1, usr2, winch, interrupt = 0;
int usr1_pid, usr2_pid = 0;

void handlerLavoratori(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGUSR1)
    {
        if (usr1 == 0)
        {
            usr1 = 1;
            usr1_pid = info->si_pid;
        }
        else if (usr1 == 1)
        {
            usr1 = 2;
        }
    }
    else if (signo == SIGUSR2)
    {
        usr2 = 1;
        usr2_pid = info->si_pid;
    }
    else if (signo == SIGINT)
    {
        interrupt = 1;
    }
}

void handlerPadre(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGWINCH)
    {
        winch = 1;
    }
    else if (signo == SIGUSR1)
    {
        wait(NULL);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usare come: ./PostOffice <n> <path/to/file> <pidInput>\n");
        exit(1);
    }

    int n;
    if (strspn(argv[1], "0123456789") == strlen(argv[1]))
    {
        n = atoi(argv[1]);
        if (n < 1 || n > 10)
        {
            fprintf(stderr, "ERROR %d: <n> deve essere compreso tra 1 e 10\n", n);
            exit(1);
        }
    }
    else
    {
        fprintf(stderr, "ERROR %s: <n> deve essere un numero compreso tra 1 e 10\n", argv[1]);
        exit(1);
    }

    FILE *fd = fopen(argv[2], "r");
    if (fd == NULL)
    {
        fprintf(stderr, "ERROR %s: <path/to/file> punta ad un file non esistente\n", argv[2]);
        exit(1);
    }
    fclose(fd);

    if (strspn(argv[3], "0123456789") == strlen(argv[1]))
    {
        fprintf(stderr, "ERROR %s: <pidInput> non è un PID\n", argv[3]);
        exit(1);
    }
    if (kill(atoi(argv[3]), 0) == -1)
    {
        fprintf(stderr, "ERROR %s: <pidInput> non corrisponde ad un processo esistente\n", argv[3]);
        exit(1);
    }
    int PID = atoi(argv[3]);

    printf("Padre: %d\n", getpid());
    fflush(stdout);

    key_t queueKey = ftok(argv[2], getpid());
    int queueId = msgget(queueKey, 0777 | IPC_CREAT);

    int pipes[n][2];
    int childrenArray[n];

    for (int i = 0; i < n; i++)
    {
        childrenArray[i] = -1;
    }

    int myIndex = 0;

    int children;
    for (int i = 0; i < n; i++)
    {
        myIndex = i;
        pipe(pipes[i]);

        children = fork();
        if (children == 0)
        {
            struct sigaction sa;
            sa.sa_sigaction = handlerLavoratori;
            sa.sa_flags = SA_SIGINFO;
            sigemptyset(&sa.sa_mask);
            sigaction(SIGUSR1, &sa, NULL);
            sigaction(SIGUSR2, &sa, NULL);
            sigaction(SIGINT, &sa, NULL);

            close(pipes[myIndex][1]);

            kill(PID, SIGTERM);

            printf("Figlio_%d: %d\n", i, getpid());
            fflush(stdout);
            break;
        }
        else
        {
            close(pipes[i][0]);
            childrenArray[i] = children;
        }
    }

    if (children == 0)
    {
        while (1)
        {
            if (usr1 == 1)
            {
                kill(usr1_pid, SIGUSR1);
            }
            else if (usr1 == 2)
            {
                kill(getppid(), SIGUSR1);
                printf("Terminato %d\n", getpid());
                fflush(stdout);
                exit(0);
            }

            if (usr2)
            {
                usr2 = 0;
                kill(usr2_pid, SIGUSR2);
            }
            if (interrupt)
            {
                interrupt = 0;
                char buff[100];
                read(pipes[myIndex][0], buff, 100);
                printf("FIGLIO: %d\n", myIndex);
                fflush(stdout);

                struct msg_buff msg;
                msg.mtype = getpid();
                strcpy(msg.mtext, buff);
                msgsnd(queueId, &msg, sizeof(msg.mtext), 0);

                msgrcv(queueId, &msg, 100, 0, 0);
                printf("[MSG]: %s\n", msg.mtext);
                fflush(stdout);
            }

            pause();
        }
    }
    else
    {
        struct sigaction sa;
        sa.sa_sigaction = handlerPadre;
        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGWINCH, &sa, NULL);
        sigaction(SIGUSR1, &sa, NULL);
        while (1)
        {
            if (winch)
            {
                winch = 0;
                fd = fopen(argv[2], "r");
                char buff[100];
                int turn = 0;

                while (fgets(buff, 100, fd))
                {
                    int attuale = turn % n;
                    while (kill(childrenArray[attuale], 0) == -1)
                    {
                        turn += 1;
                        attuale = turn % n;
                    }

                    buff[strcspn(buff, "\n")] = 0;
                    printf("PADRE: %d\n", attuale);
                    fflush(stdout);

                    write(pipes[attuale][1], buff, sizeof(buff));
                    kill(childrenArray[attuale], SIGINT);
                    sleep(1);

                    turn += 1;
                }
                fclose(fd);

                for (int i = 0; i < n; i++)
                {
                    kill(childrenArray[i], SIGTERM);
                    exit(0);
                }
            }

            pause();
        }
    }

    return 0;
}