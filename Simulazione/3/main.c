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

int countWords(char *buff)
{
    int retVal = 0;
    int len;
    while ((len = strcspn(buff, " ")) != strlen(buff))
    {
        retVal += 1;
        for (int i = 0; i <= len; i++)
        {
            buff[i] = 'a';
        }
    }
    retVal += 1;

    return retVal;
}

int main(int argc, char *argv[])
{

    // Flags di counter
    int accessFile, n_flags = 0;
    char filePath[256];

    // Settiamo le flags
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-l") == 0)
            {
                n_flags += 1;
            }
            else if (strcmp(argv[i], "-c") == 0)
            {
                n_flags += 1;
            }
            else
            {
                fprintf(stderr, "Opzione non esistente\n");
                exit(1);
            }
        }
        else
        {
            accessFile = 1;
            strcpy(filePath, argv[i]);
        }
    }

    int pid = fork();
    if (pid == 0)
    {
        // Variabili con i counter
        int n_words = 0;
        int n_lines = 0;

        char buff[256];
        if (accessFile == 1)
        {
            // Accesso al file
            FILE *fd = fopen(filePath, "r");
            if (fd == NULL)
            {
                fprintf(stderr, "File non esistente\n");
                exit(1);
            }

            // Lettura
            while (fgets(buff, 256, fd) > 0)
            {
                n_words += countWords(buff);
                n_lines++;
            }

            fclose(fd);
        }
        else
        {
            while (fgets(buff, 256, stdin) > 0)
            {
                n_words += countWords(buff);
                n_lines++;
            }
        }

        for (int i = 1; i < n_flags + 1; i++)
        {
            if (strcmp(argv[i], "-l") == 0)
            {
                printf("%d", n_lines);
                fflush(stdout);
            }
            else if (strcmp(argv[i], "-c") == 0)
            {
                printf("%d", n_words);
                fflush(stdout);
            }
            printf(" ");
            fflush(stdout);
        }
        printf("\n");
        fflush(stdout);

        exit(0);
    }
    wait(&pid);

    return 0;
}