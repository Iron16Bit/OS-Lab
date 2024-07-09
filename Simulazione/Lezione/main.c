
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <wait.h>
#include <signal.h>
#include <stdlib.h>

void repeater(int signo, siginfo_t *info, void *empty);
void relayer(int signo, siginfo_t *info, void *empty);
void logger(int signo, int pid);
void interrupter(int signo, siginfo_t *info, void *empty);

// void tmp(int queueId);
// void handler(int signo, siginfo_t *info, void *empty);

struct msg_buffer
{
    long mtype;
    char mtext[100];
};

char path[100];

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./SignalProxy <path>\n");
        exit(1);
    }
    strcpy(path, argv[1]);
    strcpy(path, "logFile");

    // Remove and recreate support files to clean them
    remove("/tmp/queue");
    creat("/tmp/queue", 0777);
    remove(path);
    creat(path, 0777);

    // Create the different handlers
    struct sigaction sa1;        // Define sigaction struct
    sa1.sa_sigaction = repeater; // Assign handler to struct field
    sa1.sa_flags = SA_SIGINFO;   // Initialise flags
    sigemptyset(&sa1.sa_mask);   // Define an empty mask
    sigaction(SIGUSR1, &sa1, NULL);

    struct sigaction sa2;       // Define sigaction struct
    sa2.sa_sigaction = relayer; // Assign handler to struct field
    sa2.sa_flags = SA_SIGINFO;  // Initialise flags
    sigemptyset(&sa2.sa_mask);  // Define an empty mask
    sigaction(SIGUSR2, &sa2, NULL);

    struct sigaction sa3;           // Define sigaction struct
    sa3.sa_sigaction = interrupter; // Assign handler to struct field
    sa3.sa_flags = SA_SIGINFO;      // Initialise flags
    sigemptyset(&sa3.sa_mask);      // Define an empty mask
    sigaction(SIGINT, &sa3, NULL);

    // Get queue
    key_t queue1Key = ftok("/tmp/queue", 1);           // Get queue key
    int queueId = msgget(queue1Key, 0777 | IPC_CREAT); // Create queue

    // tmp(queueId);

    printf("IO SONO: %d\n", getpid());
    while (1)
    {
        struct msg_buffer received_msg;
        int esito = msgrcv(queueId, &received_msg, sizeof(received_msg.mtext), 0, 0);
        if (esito)
        {
            kill(atoi(received_msg.mtext), SIGALRM);
        }
    }

    return 0;
}

void repeater(int signo, siginfo_t *info, void *empty)
{
    int senderPID = info->si_pid;
    kill(senderPID, SIGUSR1);
    logger(signo, senderPID);
}

void relayer(int signo, siginfo_t *info, void *empty)
{
    int child = fork();
    if (!child)
    {
        int senderPID = info->si_pid;
        kill(senderPID, SIGUSR2);
        logger(signo, senderPID);
    }
}

void interrupter(int signo, siginfo_t *info, void *empty)
{
    int openFile = open(path, O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
    write(openFile, "STOP\n", strlen("STOP\n"));
    close(openFile);
    exit(0);
}

void logger(int signo, int pid)
{
    if (!fork())
    {
        sleep(3);
        char toWrite[100];
        sprintf(toWrite, "%d-%d\n", pid, signo);
        int openFile = open(path, O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
        write(openFile, toWrite, strlen(toWrite));
        close(openFile);
    }
}

// Solo per testare la lettura della coda
// void tmp(int queueId)
// {
//     struct sigaction sa;
//     sa.sa_sigaction = handler;
//     sa.sa_flags = SA_SIGINFO;
//     sigemptyset(&sa.sa_mask);
//     sigaction(SIGALRM, &sa, NULL);

//     struct msg_buffer send_message;
//     char text[100];
//     sprintf(text, "%d", getpid());
//     strcpy(send_message.mtext, text);
//     send_message.mtype = 1;
//     msgsnd(queueId, &send_message, strlen(send_message.mtext), 0);
// }

// void handler(int signo, siginfo_t *info, void *empty)
// {
//     printf("Ricevuto sigalarm\n");
// }