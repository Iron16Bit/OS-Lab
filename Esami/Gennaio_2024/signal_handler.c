#include <signal.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/msg.h>
#include <sys/wait.h>

// Counter dei segnali
int n_xcpu, n_ttou, n_ttin = 0;

// Flag dei segnali del parent
int xcpu, ttou, ttin, fpe, usr1, usr2, xfsz, quit, prof = 0;

// Flag dei segnali del children
int c_usr1 = 0;

// Struct per inviare messaggi nella code
struct msg_buffer
{
    long mtype;
    char mtext[100];
};

// Handler del quit per i children
void quitter(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGQUIT)
    {
        fprintf(stderr, "QUITTING\n");
        exit(0);
    }
}

// Handler di SIGINFO1 per i children
void printer(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGUSR1)
    {
        c_usr1 = 1;
    }
}

// Handler dei segnali
void handler(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGXCPU)
    {
        xcpu = 1;
    }
    else if (signo == SIGTTOU)
    {
        ttou = 1;
    }
    else if (signo == SIGTTIN)
    {
        ttin = 1;
    }
    else if (signo == SIGFPE)
    {
        fpe = info->si_pid;
    }
    else if (signo == SIGUSR1)
    {
        usr1 = info->si_pid;
    }
    else if (signo == SIGUSR2)
    {
        usr2 = 1;
    }
    else if (signo == SIGXFSZ)
    {
        xfsz = 1;
    }
    else if (signo == SIGQUIT)
    {
        quit = 1;
    }
    else if (signo == SIGPROF)
    {
        prof = 1;
    }
}

// Varie utility per l'array di PID
int getYoungest(int *a)
{
    int retVal = -1;
    int tmp = 0;
    for (int i = 0; i < 10; i++)
    {
        if (a[i] != __INT_MAX__ && a[i] > tmp)
        {
            tmp = a[i];
            retVal = i;
        }
    }
    return retVal;
}

int getOldest(int *a)
{
    int retVal = -1;
    int tmp = __INT_MAX__;
    for (int i = 0; i < 10; i++)
    {
        if (a[i] != __INT_MAX__ && a[i] < tmp)
        {
            tmp = a[i];
            retVal = i;
        }
    }
    return retVal;
}

int removePID(int *a, int PID)
{
    for (int i = 0; i < 10; i++)
    {
        if (a[i] == PID)
        {
            a[i] = __INT_MAX__;
            return 0;
        }
    }
    return -1;
}

int sizeChildren(int *a)
{
    int retVal = 10;
    for (int i = 0; i < 10; i++)
    {
        if (a[i] == __INT_MAX__)
        {
            retVal -= 1;
        }
    }
    return retVal;
}

int addPID(int *a, int PID)
{
    for (int i = 0; i < 10; i++)
    {
        if (a[i] == __INT_MAX__)
        {
            a[i] = PID;
            return 0;
        }
    }
    return -1;
}

int getPID(int *a, int PID)
{
    for (int i = 0; i < 10; i++)
    {
        if (a[i] == PID)
        {
            return i;
        }
    }
    return -1;
}

int getPrevYoungest(int *a)
{
    int youngestIndex = getYoungest(a);
    int PID = a[youngestIndex];
    int retVal = -1;
    int tmp = 0;
    for (int i = 0; i < 10; i++)
    {
        if (a[i] != __INT_MAX__ && a[i] > tmp && a[i] < PID)
        {
            retVal = i;
            tmp = a[i];
        }
    }
    return retVal;
}

int getFirstEmpty(int *a)
{
    for (int i = 0; i < 10; i++)
    {
        if (a[i] == __INT_MAX__)
        {
            return i;
        }
    }
    return -1;
}

int main()
{
    // TODO: rimuovere, stampa PID solo per il debug
    printf("PID: %d\n", getpid());

    // Definire la sigaction (quale handler usare per quali segnali)
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGXCPU, &sa, NULL);
    sigaction(SIGTTOU, &sa, NULL);
    sigaction(SIGTTIN, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGXFSZ, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGPROF, &sa, NULL);

    // Creare la queue dei messaggi
    remove("/tmp/queue");
    creat("/tmp/queue", 0777);
    key_t queueKey = ftok("/tmp/queue", 5);
    int queueId = msgget(queueKey, 0777 | IPC_CREAT);

    // Queue usata per i figli
    remove("tmp/childrenQueue");
    creat("tmp/childrenQueue", 0777);
    key_t childrenQKey = ftok("/tmp/childrenQueue", 1);
    int childrenQId = msgget(childrenQKey, 0777 | IPC_CREAT);

    // Array con i PID dei figli
    int childrenPID[10];
    for (int i = 0; i < 10; i++)
    {
        childrenPID[i] = __INT_MAX__;
    }

    // Array con le pipe con i figli
    int childrenPipes[10][2];

    // Handler segna solamente quando sono arrivati dei segnali, qui gestiamo effettivamente il segmale
    while (1)
    {
        if (xcpu)
        {
            xcpu = 0;
            n_xcpu += 1;
            printf("SIGXCPU_%d\n", n_xcpu);
        }
        if (ttou)
        {
            ttou = 0;
            n_ttou += 1;
            printf("SIGTTOU_%d\n", n_ttou);
        }
        if (ttin)
        {
            ttin = 0;
            n_ttin += 1;
            printf("SIGTTIN_%d\n", n_ttin);
        }
        if (fpe)
        {
            int tmp = fpe;
            fpe = 0;

            // Create message
            struct msg_buffer msg;
            msg.mtype = tmp;
            strcpy(msg.mtext, "SIGFPE");

            // Push message on queue
            msgsnd(queueId, &msg, sizeof(msg.mtext), 0);

            // TODO controllo del messggio inviato, va rimosso
            struct msg_buffer received;
            msgrcv(queueId, &received, sizeof(received.mtext), 0, 0);
            printf("%s from %ld\n", received.mtext, received.mtype);
        }
        if (usr1)
        {
            int tmp = usr1;
            usr1 = 0;

            // Create message
            struct msg_buffer msg;
            msg.mtype = tmp;
            strcpy(msg.mtext, "SIGUSR1");

            // Push message on queue
            msgsnd(queueId, &msg, sizeof(msg.mtext), 0);

            // TODO controllo del messggio inviato, va rimosso
            struct msg_buffer received;
            msgrcv(queueId, &received, sizeof(received.mtext), 0, 0);
            printf("%s from %ld\n", received.mtext, received.mtype);
        }
        if (usr2)
        {
            usr2 = 0;

            if (sizeChildren(childrenPID) < 10)
            {
                // Get position in the PID array of the new children
                int newPos = getFirstEmpty(childrenPID);

                // Create pipe with new children
                if (pipe(childrenPipes[newPos]) == -1) // Pipe exists, close it
                {
                    close(childrenPipes[newPos][1]);
                }
                pipe(childrenPipes[newPos]); // Recreate it

                int children = fork();
                if (!children)
                {
                    // Send msg to queue to allow the parent to know its pid
                    struct msg_buffer msg;
                    msg.mtype = getpid();
                    sprintf(msg.mtext, "tmp");
                    msgsnd(childrenQId, &msg, sizeof(msg.mtext), 0);
                    printf("Born children %d\n", getpid());

                    // Define handler for SIGQUIT signal
                    struct sigaction sa1;
                    sa1.sa_flags = SA_SIGINFO;
                    sa1.sa_sigaction = quitter;
                    sigaction(SIGQUIT, &sa1, NULL);

                    // Add SIGUSR1 handler
                    struct sigaction sa2;
                    sa2.sa_flags = SA_SIGINFO;
                    sa2.sa_sigaction = printer;
                    sigaction(SIGUSR1, &sa2, NULL);
                    sigaction(SIGUSR2, &sa2, NULL);

                    // Open pipe with parent
                    close(childrenPipes[newPos][1]); // Close write, will only read

                    // Read PID received from pipe
                    char buff[50];
                    while (1)
                    {
                        if (c_usr1)
                        {
                            c_usr1 = 0;
                            read(childrenPipes[newPos][0], &buff, 50);
                            fprintf(stderr, "%s\n", buff);
                        }
                    }
                }

                close(childrenPipes[newPos][0]); // Close read, will only write

                // Add PID of children to array
                struct msg_buffer msg;
                msgrcv(childrenQId, &msg, sizeof(msg), 0, 0);
                int res = addPID(childrenPID, msg.mtype);
                printf("Created children %ld\n", msg.mtype);
            }
            else
            {
                printf("Too many children...\n");
            }
        }
        if (xfsz)
        {
            xfsz = 0;
            if (sizeChildren(childrenPID) > 0)
            {
                int tobeKilled = getOldest(childrenPID);
                if (tobeKilled == -1)
                {
                    printf("No children available...\n");
                }
                else
                {
                    kill(childrenPID[tobeKilled], SIGTERM);
                    printf("Killed children %d\n", childrenPID[tobeKilled]);
                    int esito = removePID(childrenPID, childrenPID[tobeKilled]);
                }
            }
            else
            {
                printf("No children available...\n");
            }
        }
        if (quit)
        {
            int tobeQuitted = getOldest(childrenPID);
            while (tobeQuitted != -1)
            {
                printf("Killing %d...\n", childrenPID[tobeQuitted]);
                kill(childrenPID[tobeQuitted], SIGQUIT);
                wait(&childrenPID[tobeQuitted]);
                int esito = removePID(childrenPID, childrenPID[tobeQuitted]);
                tobeQuitted = getOldest(childrenPID);
            }
            exit(0);
        }
        if (prof)
        {
            prof = 0;
            if (sizeChildren(childrenPID) == 1)
            {
                char buff[50];
                sprintf(buff, "%d", 0);
                int indexYoungest = getYoungest(childrenPID);
                write(childrenPipes[indexYoungest][1], buff, strlen(buff) + 1);
                kill(childrenPID[indexYoungest], SIGUSR1);
            }
            else if (sizeChildren(childrenPID) > 1)
            {
                char buff[50];
                int indexYoungest = getYoungest(childrenPID);
                int indexPrevYoungest = getPrevYoungest(childrenPID);
                printf("%d -> %d\n", indexYoungest, indexPrevYoungest);
                sprintf(buff, "%d", childrenPID[indexPrevYoungest]);
                write(childrenPipes[indexYoungest][1], buff, strlen(buff) + 1);
                kill(childrenPID[indexYoungest], SIGUSR1);
            }
            else
            {
                printf("Not enough children...\n");
            }
        }
        pause();
    }

    return 0;
}