#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>

#include <fcntl.h>
#include <signal.h>

#include <pthread.h>

int main(int argc, char *argv[])
{
    if (argc > 11 || argc == 1)
    {
        fprintf(stderr, "Passare da 1 a 10 percorsi come argomento\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++)
    {
        int pid = fork();
        if (pid == 0)
        {
            char c;
            FILE *fd = fopen(argv[i], "r");
            if (fd == NULL)
            {
                fprintf(stderr, "Il file %s non esiste\n", argv[i]);
                exit(1);
            }

            while ((c = fgetc(fd)) != EOF)
            {
                if (c != '\n' && c != '\0')
                {
                    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
                    {
                        printf("%c", c);
                    }
                    else
                    {
                        printf("-");
                    }
                }
            }
            exit(0);
        }
        wait(&pid);
    }

    return 0;
}