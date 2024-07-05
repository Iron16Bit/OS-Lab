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

// Variabile globale con la password e l'ultimo PID ricevuto
char password[256];
int PID;

// Struct dei messaggi
struct msg_buffer
{
    int mtype;
    char mtext[100];
};

// Funzione eseguita dal thread
void *threadFun(void *param)
{
    // Creazione della fifo (esiste già)
    mkfifo("/tmp/login.fifo", S_IRUSR | S_IWUSR);

    // Apertura della fifo in lettura
    int fifoL = open("/tmp/login.fifo", O_RDONLY);

    // Creazione della coda di messaggi
    key_t queueKey1 = ftok("/tmp/login.fifo", 51);
    int queueId1 = msgget(queueKey1, 0777 | IPC_CREAT);

    // Itera in attesa di messaggi sulla coda
    while (1)
    {
        char tmp[256];
        int r = read(fifoL, tmp, sizeof(tmp));
        if (r > 0)
        {
            tmp[strcspn(tmp, "\n")] = 0;
            printf("password: %s\n", tmp);
            if (strcmp(password, tmp) == 0)
            {
                printf("OK\n");
                fflush(stdout);
                fprintf(stderr, "%s logged in\n", (char *)param);
                fflush(stderr);
            }
            else
            {
                printf("NO\n");
                fflush(stdout);
            }

            // Mandare us messaggio su queueId1 per ogni tentativo di accesso
            struct msg_buffer sendMsg;
            sendMsg.mtype = PID;
            strcpy(sendMsg.mtext, tmp);
            msgsnd(queueId1, &sendMsg, sizeof(sendMsg.mtext), 0);
        }
    }
    close(fifoL);
}

// Handler dei segnali del main
void mainHandler(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGTSTP)
    {
        exit(0);
    }
}

int main()
{
    printf("PID: %d\n", getpid());
    fflush(stdout);

    // Controllare che il file con le credenziali esista
    FILE *fd = fopen("/tmp/credentials.txt", "r");
    if (fd == NULL)
    {
        fprintf(stderr, "Il file /tmp/credentials.txt non esiste\n");
        exit(1);
    }

    // Lettura di username[0] e password[1]
    char buff[2][256];
    fgets(buff[0], 256, fd);
    fgets(buff[1], 256, fd);
    fclose(fd);

    buff[0][strcspn(buff[0], "\n")] = 0;
    buff[1][strcspn(buff[1], "\n")] = 0;

    // Copia password nella variabile globale
    strcpy(password, buff[1]);

    // Controllare se /tmp/secret.txt esiste e nel caso eliminarlo prima di ricrearlo
    FILE *secret = fopen("/tmp/secret.txt", "r");
    if (secret != NULL)
    {
        remove("/tmp/secret.txt");
        fclose(secret);
    }
    creat("/tmp/secret.txt", 0777);
    secret = fopen("/tmp/secret.txt", "w");

    // Spawna il thread
    pthread_t t_id;
    char arg[256];
    strcpy(arg, buff[0]);
    pthread_create(&t_id, NULL, threadFun, (void *)arg);

    // Scrivere la password[1] su /tmp/secret.txt
    fputs(buff[1], secret);
    fclose(secret);

    // Eliminare il file /tmp/credentials.txt
    // TODO: remove("/tmp/credentials.txt"); riattivare

    // Definiamo la sigaction
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = mainHandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTSTP, &sa, NULL);

    // Apriamo la fifo /tmp/clients.fifo (già esistente)
    // creat("/tmp/clients.fifo", 0777);
    mkfifo("/tmp/clients.fifo", S_IRUSR | S_IWUSR);
    mkfifo("/tmp/authenticator.fifo", S_IRUSR | S_IWUSR);

    // La fifo viene aperta in scrittura
    printf("%d\n", open("/tmp/authenticator.fifo", O_WRONLY));
    int fifoS = open("/tmp/authenticator.fifo", O_WRONLY);

    // Il programma comunica il proprio pid sulla fifo
    char tmp[256];
    sprintf(tmp, "%d", getpid());
    write(fifoS, tmp, strlen(tmp) + 1);
    close(fifoS);

    // Apre la fifo in lettura
    int fifoL = open("/tmp/clients.fifo", O_RDONLY);

    sleep(5); // Per evitare che il PID venga riletto da questo programma

    while (1)
    {
        int r = read(fifoL, tmp, sizeof(tmp));
        if (r > 0)
        {
            tmp[r] = 0;
            int destPID = atoi(tmp);
            kill(destPID, SIGUSR1);
            printf("Sent SIGUSR1 to %d\n", destPID);
            PID = destPID;
            fflush(stdout);
        }
    }

    close(fifoL);

    return 0;
}