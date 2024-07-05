#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <signal.h>

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>

// Struct dei messaggi
// Struct for queue message
struct msg_buffer
{
    int mtype;
    char mtext[100];
};

void handler(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGUSR1)
    {
        printf("Received SIGUSR1 from %d\n", info->si_pid);
    }
}

int main()
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    printf("PID secondo processo: %d\n", getpid());
    fflush(stdout);

    mkfifo("/tmp/authenticator.fifo", S_IRUSR | S_IWUSR);
    int fifoL = open("/tmp/authenticator.fifo", O_RDONLY);

    char tmp[256];
    int r = read(fifoL, tmp, sizeof(tmp));
    while (r <= 0)
    {
        fflush(stdout);
        r = read(fifoL, tmp, sizeof(tmp));
    }
    tmp[r] = 0;
    printf("Il padre ha PID: %s\n", tmp);
    fflush(stdout);
    close(fifoL);

    int fifoS = open("/tmp/clients.fifo", O_WRONLY);
    sprintf(tmp, "%d", getpid());
    write(fifoS, tmp, strlen(tmp) + 1);
    close(fifoS);

    int fifoLogin = open("/tmp/login.fifo", O_WRONLY);
    printf("debug\n");
    fflush(stdout);
    char buff[256];

    strcpy(buff, "pippo");
    write(fifoLogin, buff, strlen(buff) + 1);
    sleep(1);

    printf("debug2\n");
    fflush(stdout);

    strcpy(buff, "brill");
    write(fifoLogin, buff, strlen(buff) + 1);
    sleep(1);

    printf("debug3\n");
    fflush(stdout);

    strcpy(buff, "brill22!");
    write(fifoLogin, buff, strlen(buff) + 1);
    sleep(1);

    close(fifoLogin);

    printf("debug4\n");
    fflush(stdout);

    key_t queueKey = ftok("/tmp/login.fifo", 51);
    int queueId = msgget(queueKey, 0777 | IPC_CREAT);

    printf("debug5\n");
    fflush(stdout);

    while (1)
    {
        struct msg_buffer msg;
        msgrcv(queueId, &msg, 50, 0, 0);
        printf("Type: %d\nText: %s\n\n", msg.mtype, msg.mtext);
        fflush(stdout);
    }

    return 0;
}