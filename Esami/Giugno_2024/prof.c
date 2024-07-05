#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

// Global variables to hold password and destinationPid
char passwd[101];
int destPid;

// Struct for queue message
typedef struct
{
    long mtype;
    char msg[50];
} message;

// Termination handler for SIGTSTP
void sigHandler(int signo)
{
    exit(0); // Terminate program
}

// Thread function
void *thread(void *param)
{
    char *username = (char *)param; // Convert param to our username string

    // Open fifo Readonly. This will be blocking
    int fd = open("/tmp/login.fifo", O_RDONLY);
    char buf[100]; // Buffer to hold message

    // Create queue key and queueId
    key_t queue1Key = ftok("/tmp/login.fifo", 51);
    int queueId = msgget(queue1Key, 0777 | IPC_CREAT | IPC_EXCL);

    // Define variable to host message
    message msg;
    while (1)
    {
        // Read up to 100 bytes from the fifo. This will be blocking.
        int r = read(fd, buf, 100);

        if (r > 0)
        {

            // Terminate the string
            buf[r] = 0;

            // Check if the received string is equal to our password (global variable)
            if (strcmp(passwd, buf) == 0)
            {
                printf("OK\n");
                fprintf(stderr, "%s logged in\n", username);
                fflush(stdout);
                fflush(stderr);
            }
            else
            {
                printf("NO\n");
                fflush(stdout);
            }
            // Set the msg type using oour global variable
            msg.mtype = destPid;
            // Copy the password to the message payload
            strncpy(msg.msg, buf, r + 1);
            // Send the message
            msgsnd(queueId, &msg, sizeof(msg.msg), 0);
        }
    }
}

int main(void)
{
    char username[100];

    // Override SIGTSTP action
    struct sigaction sa;
    sa.sa_handler = sigHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTSTP, &sa, NULL);

    // Open the credential file
    FILE *cred = fopen("/tmp/credentials.txt", "r");
    if (cred == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        return 2;
    }

    // Read both username and password from the file
    fscanf(cred, "%s\n%s", username, passwd);

    // Eliminate the file
    // int esito = remove("/tmp/credentials.txt");
    // if (esito != 0)
    // {
    //     fprintf(stderr, "Error removing file\n");
    //     return 3;
    // }

    // Create new file in READ and WRITE mode with user read and write permission
    int fd = open("/tmp/secret.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    // Write the password to the file and close it
    write(fd, passwd, strlen(passwd));
    close(fd);

    // Create new fifo with read and write privileges for the user
    mkfifo("/tmp/authenticator.fifo", S_IRUSR | S_IWUSR);

    // Open the fifo WRITE only (this is blocking)
    fd = open("/tmp/authenticator.fifo", O_WRONLY);

    char pid[100];
    // Convert pid to string
    sprintf(pid, "%d", getpid());

    // Write pid to fifo
    write(fd, pid, strlen(pid));

    // close pipe
    close(fd);

    printf("%d\n", getpid());
    fflush(stdout);

    // Open fifo in read only. This is blocking
    fd = open("/tmp/clients.fifo", O_RDONLY);
    sleep(2);

    // Create new thread with our username as parameter
    pthread_t tid;
    pthread_create(&tid, NULL, thread, (void *)&username);

    while (1)
    {
        // Wait for incoming message
        int r = read(fd, pid, 100);

        if (r > 0)
        {
            // Terminate string
            pid[r] = 0;

            // Convert pid to integer and set the global variable
            destPid = atoi(pid);

            // Signal the process
            printf("%d\n", destPid);
            kill(destPid, SIGUSR1);
        }
    }
}