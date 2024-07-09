#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/msg.h>

#include <signal.h>
#include <fcntl.h>

#include <pthread.h>

void handler(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGUSR1)
    {
        printf("SIGURS1\n");
    }
    else if (signo == SIGUSR2)
    {
        printf("SIGUSR2\n");
    }
}

// Error function
void error(int PID)
{
    fprintf(stderr, "Errore durante l'esecuzione dell'azione\n");
    kill(PID, SIGUSR2);
    exit(1);
}

// Struct dei messaggi
struct msg_buff
{
    int mtype;
    char mtext[100];
};

int main(int argc, char *argv[])
{
    if (argc != 4 && argc != 5)
    {
        fprintf(stderr, "Usare come: ./coda <nome> <azione> [<valore>] <PID>\n");
        exit(1);
    }

    char nome[33];
    strcpy(nome, argv[1]);
    char azione[33];
    strcpy(azione, argv[2]);
    char valore[33];
    char PID[33];

    if (argc == 4)
    {
        strcpy(PID, argv[3]);
    }
    else
    {
        strcpy(valore, argv[3]);
        strcpy(PID, argv[4]);
    }

    struct msg_buff msg;

    // TODO rimuovere, fa parte del testing
    sprintf(PID, "%d", getpid());
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    // argv[2] = azione
    if (strcmp(azione, "new") == 0) // Crea la queue
    {
        if (argc != 4)
        {
            error(atoi(PID));
        }
        key_t queueKey = ftok(nome, 1);
        int queueId = msgget(queueKey, 0777 | IPC_CREAT);
    }
    else if (strcmp(azione, "put") == 0)
    {
        if (argc != 5)
        {
            error(atoi(PID));
        }
        key_t queueKey = ftok(nome, 1);
        int queueId = msgget(queueKey, 0777 | IPC_CREAT);

        msg.mtype = 0;
        strcpy(msg.mtext, valore);

        int esito = msgsnd(queueId, &msg, sizeof(msg.mtext), 0);
        if (esito == -1)
        {
            error(atoi(PID));
        }
    }
    else if (strcmp(azione, "get") == 0)
    {
        if (argc != 4)
        {
            error(atoi(PID));
        }
        key_t queueKey = ftok(nome, 1);
        int queueId = msgget(queueKey, 0777 | IPC_CREAT);

        int read = msgrcv(queueId, &msg, 100, 0, IPC_NOWAIT);
        if (read > 0)
        {
            printf("%s\n", msg.mtext);
        }
    }
    else if (strcmp(azione, "del") == 0)
    {
        if (argc != 4)
        {
            error(atoi(PID));
        }

        key_t queueKey = ftok(nome, 1);
        int queueId = msgget(queueKey, 0777 | IPC_CREAT);

        if (msgctl(queueId, IPC_RMID, NULL) == -1)
        {
            error(atoi(PID));
        }
    }
    else if (strcmp(azione, "emp") == 0)
    {
        if (argc != 4)
        {
            error(atoi(PID));
        }
        key_t queueKey = ftok(nome, 1);
        int queueId = msgget(queueKey, 0777 | IPC_CREAT);

        int read = msgrcv(queueId, &msg, 100, 0, IPC_NOWAIT);
        while (read > 0)
        {
            printf("%s\n", msg.mtext);
            read = msgrcv(queueId, &msg, 100, 0, IPC_NOWAIT);
        }
    }
    else if (strcmp(azione, "mov") == 0)
    {
        if (argc != 5)
        {
            error(atoi(PID));
        }

        // Coda di partenza
        key_t queueKeySTART = ftok(nome, 1);
        int queueIdSTART = msgget(queueKeySTART, 0777 | IPC_CREAT);

        // Coda di destinazione
        key_t queueKeyEND = ftok(valore, 1);
        int queueIdEND = msgget(queueKeyEND, 0777 | IPC_CREAT);

        int messaggiSpostati = 0;
        int read = msgrcv(queueIdSTART, &msg, 100, 0, IPC_NOWAIT);
        // while (read > 0)
        // {
        //     printf("%s\n", msg.mtext);
        //     fflush(stdout);
        //     messaggiSpostati += 1;

        //     int esito = msgsnd(queueIdEND, &msg, sizeof(msg.mtext), 0);
        //     if (esito == -1)
        //     {
        //         error(atoi(PID));
        //     }

        //     read = msgrcv(queueIdSTART, &msg, 100, 0, IPC_NOWAIT);
        // }

        while (msgrcv(queueIdSTART, &msg, sizeof(msg.mtext), 0, IPC_NOWAIT) != -1)
        {
            printf("%s\n", msg.mtext);
            if (msgsnd(queueIdEND, &msg, strlen(msg.mtext), 0) == -1)
            {
                error(atoi(PID));
            }
        }

        printf("%d\n", messaggiSpostati);
        fflush(stdout);

        if (msgctl(queueIdSTART, IPC_RMID, NULL) == -1)
        {
            error(atoi(PID));
        }
    }
    else
    {
        fprintf(stderr, "Azione %s non riconosciuta\n", azione);
        kill(atoi(PID), SIGUSR2);
        exit(1);
    }

    kill(atoi(PID), SIGUSR1);

    return 0;
}