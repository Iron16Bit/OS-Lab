
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

struct msg_buffer
{
    long mtype;
    char mtext[100];
};

void handler(int signo, siginfo_t * info, void *empty);

int main()
{
    // Get queue
    key_t queue1Key = ftok("/tmp/queue", 1);           // Get queue key
    int queueId = msgget(queue1Key, 0777 | IPC_CREAT); // Create queue

    // SIGALRM handler to check if the message queue is read and processed by main
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    struct msg_buffer send_message;
    char text[100];
    sprintf(text, "%d", getpid());
    strcpy(send_message.mtext, text);
    send_message.mtype = 1;
    msgsnd(queueId, &send_message, strlen(send_message.mtext), 0);
    printf("%s\n", send_message.mtext);

    while(1);

    return 0;
}

void handler(int signo, siginfo_t * info, void *empty) {
    printf("Received sigalarm\n");
}