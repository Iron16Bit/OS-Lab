#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <signal.h>

#include <pthread.h>

// Flag dei segnali
int usr2, usr1 = 0;

// Utility per i segnali
int usr2_dst, usr1_dst = 0;

// Handler dei segnali
void handler(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGUSR1)
    {
        usr1 = 1;
        usr1_dst = info->si_pid;
    }
    else if (signo == SIGUSR2)
    {
        usr2 = 1;
        usr2_dst = info->si_pid;
    }
}

// Struct dei messaggi
struct msg {
    int mtype;
    char mtext[100];
};

// Path to log file
char path[100];

// Funzione spawnata dal thread
void *thread_fun(void *param)
{
    sleep(3);
    FILE *fd = fopen(path, "a");

    fputs((char *)param, fd);

    fclose(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usare come ./SignalProxy <pathToLogFile>\n");
        exit(1);
    }

    FILE *fd = fopen(argv[1], "r");
    if (fd == NULL)
    {
        fprintf(stderr, "Il file indicato da path non esiste\n");
        exit(1);
    }
    fclose(fd);
    strcpy(path, argv[1]);

    printf("[%d\n]", getpid());

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    key_t queueKey = ftok(path, 1);
    int queueId = msgget(queueKey, 0777 | IPC_CREAT);

    struct msg msg_buff;

    while (1)
    {
        if (usr1)
        {
            usr1 = 0;
            kill(usr1_dst, SIGUSR1);

            int signo = SIGUSR1;
            char buff[100];
            sprintf(buff, "#%d#%d\n", usr1_dst, signo);

            pthread_t t;
            pthread_create(&t, NULL, thread_fun, (void *)buff);
        }
        if (usr2)
        {
            usr2 = 0;
            int children = fork();
            if (!children)
            {
                kill(usr2_dst, SIGUSR2);
                exit(0);
            }
            else
            {
                int signo = SIGUSR2;
                char buff[100];
                sprintf(buff, "#%d#%d\n", usr1_dst, signo);

                pthread_t t;
                pthread_create(&t, NULL, thread_fun, (void *)buff);
            }
        }
        
        int r = msgrcv(queueId, &msg_buff, 100, 0, IPC_NOWAIT);
        if (r > 0) {
            int PID = atoi(msg_buff.mtext);
            kill(PID, SIGALRM);
        }
    }

    return 0;
}