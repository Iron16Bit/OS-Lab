#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/msg.h>

#include <signal.h>
#include <fcntl.h>

// Flag dei segnali dei lavoratori
int l_usr1, l_usr2, l_int = 0;

// Flag segnali padre
int winch = 0;

// Struct dei messaggi
struct msg_buff
{
    long mtype;
    char mtext[100];
};

// Utility per controllare se è un numero tra 1 e 10
int numIsOk(char *s)
{
    int n = strlen(s);
    if (n > 2)
    {
        return -1;
    }

    if (n == 2)
    {
        if (s[0] != '1')
        {
            return -1;
        }
        else
        {
            if (s[1] >= '0' && s[1] <= '9')
            {
                return 0;
            }
        }
    }

    if (n == 1)
    {
        if (s[0] == '0')
        {
            return -1;
        }
        else
        {
            if (s[0] >= '0' && s[0] <= '9')
            {
                return 0;
            }
        }
    }

    return -1;
}

// Handler dei segnali dei lavoratori
void handlerLavoratori(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGUSR1)
    {
        if (l_usr1 == 0)
        {
            l_usr1 = info->si_pid;
        }
        else
        {
            l_usr1 = -1;
        }
    }
    else if (signo == SIGUSR2)
    {
        l_usr2 = info->si_pid;
    }
    else if (signo == SIGINT)
    {
        l_int = 1;
    }
}

// Handler segnali padre
void handlerPadre(int signo, siginfo_t *info, void *empty)
{
    if (signo == SIGWINCH)
    {
        winch = 1;
    }
}

int main(int argc, char *argv[])
{
    // Controllare il numero di argomenti
    if (argc != 4)
    {
        fprintf(stderr, "Usare in questo modo: ./PostOffice 1-10 path PID\n");
        exit(1);
    }

    // Controllare che il numero sia valido
    if (numIsOk(argv[1]) == -1)
    {
        fprintf(stderr, "Il numero deve essere tra 1 e 10, %s non è accettato\n", argv[1]);
        exit(1);
    }
    int num = atoi(argv[1]);

    // Controllare che il percorso del file sia valido
    FILE *fd = fopen(argv[2], "r");
    if (fd == NULL)
    {
        fprintf(stderr, "Il percorso deve essere di un file esistente, %s non è valido\n", argv[2]);
        exit(1);
    }
    fclose(fd);

    // Controllare che il PID sia valido
    if (strspn(argv[3], "0123456789") != strlen(argv[3]))
    { // Controlla che tutti i caratteri nella stringa siano numeri
        fprintf(stderr, "Il PID inserito %s non è un PID\n", argv[3]);
        exit(1);
    }
    int PID = atoi(argv[3]);
    if (kill(PID, 0) == -1)
    { // Prova a pingare il processo
        fprintf(stderr, "Il PID inserito %s non è un di un processo attivo\n", argv[3]);
        exit(1);
    }

    // array con i PID dei lavoratori
    int lavoratori[10];

    // array di pipe con i lavoratori
    int pipes[10][2];

    // Creazione coda
    key_t queueKey = ftok(argv[2], getpid());
    int queueId = msgget(queueKey, 0777 | IPC_CREAT);

    // Handler segnali per il padre
    struct sigaction sa1;
    sa1.sa_sigaction = handlerPadre;
    sa1.sa_flags = SA_SIGINFO;
    sigemptyset(&sa1.sa_mask);
    sigaction(SIGWINCH, &sa1, NULL);

    printf("Parent PID: %d\n", getpid());

    // Creo n(num) lavoratori
    int children_pid;
    int myIndex; // Index negli array di lavoratori e pipe
    for (int i = 0; i < num; i++)
    {
        // Apre pipe con l'i-esimo lavoratore
        pipe(pipes[i]);

        children_pid = fork();
        if (children_pid == 0)
        {
            printf("PID_%d: %d\n", i, getpid());

            struct sigaction sa;
            sa.sa_sigaction = handlerLavoratori;
            sa.sa_flags = SA_SIGINFO;
            sigemptyset(&sa.sa_mask);
            sigaction(SIGUSR1, &sa, NULL);
            sigaction(SIGUSR2, &sa, NULL);
            sigaction(SIGINT, &sa, NULL);
            kill(PID, SIGTERM);

            // Chiude pipe in scrittura
            close(pipes[i][1]);

            myIndex = i;

            break;
        }
        else
        {
            lavoratori[i] = children_pid;

            // Chiude pipe in lettura
            close(pipes[i][0]);
        }
    }

    // Gestione delle flag dei segnali da parte dei lavoratori
    while (!children_pid)
    {
        if (l_usr1 > 0)
        {
            int dst = l_usr1;
            l_usr1 = -2;
            kill(dst, SIGUSR1);
            printf("SIGUSR1 inviato a %d\n", dst);
        }
        else if (l_usr1 == -1)
        {
            printf("Lavoratore %d terminato\n", getpid());
            exit(0);
        }
        if (l_usr2)
        {
            int dst = l_usr2;
            l_usr2 = 0;
            kill(dst, SIGUSR2);
            printf("[%d]: SIGUSR2 inviato a %d\n", getpid(), dst);
        }
        if (l_int)
        {
            l_int = 0;

            char buff[100];
            read(pipes[myIndex][0], buff, sizeof(buff));
            printf("Read: %s\n", buff);

            struct msg_buff msg;
            msg.mtype = getpid();
            strcpy(msg.mtext, buff);
            msgsnd(queueId, &msg, sizeof(msg), 0);
        }
        pause();
    }

    while (children_pid)
    {
        if (winch)
        {
            winch = 0;

            // Leggere file riga per riga ed inviare messaggio con ogni riga nella coda
            FILE *fd = fopen(argv[2], "r+");
            char str[100];

            int turn = 0;
            while (fgets(str, 100, fd))
            {
                str[strcspn(str, "\n")] = 0;

                write(pipes[turn % num][1], str, sizeof(str));
                int dest = lavoratori[turn % num];
                kill(dest, SIGINT);

                sleep(1);

                turn += 1;
            }
            fclose(fd);

            for (int i=0; i<num; i++) {
                kill(lavoratori[i], SIGTERM);
                exit(0);
            }
        }
        pause();
    }

    return 0;
}