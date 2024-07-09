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
            char c[1];
            FILE *fd = fopen(argv[i], "r");
            if (fd == NULL)
            {
                fprintf(stderr, "Il file %s non esiste\n", argv[i]);
                exit(1);
            }
            fclose(fd);

            int file = open(argv[i], O_RDONLY);
            int r = 0;
            while ((r = read(file, c, 1)) > 0)
            {
                if (r != 1)
                {
                    fprintf(stderr, "?");
                }
                else
                {
                    if (c[0] != '\n' && c[0] != '\0')
                    {
                        if ((c[0] >= 'A' && c[0] <= 'Z') || (c[0] >= 'a' && c[0] <= 'z') || (c[0] >= '0' && c[0] <= '9'))
                        {
                            printf("%c", c[0]);
                        }
                        else
                        {
                            printf("-");
                        }
                    }
                }
            }
            exit(0);
        }
        wait(&pid);
    }
    printf("\n");

    return 0;
}