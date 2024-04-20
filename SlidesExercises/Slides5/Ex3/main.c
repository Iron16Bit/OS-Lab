/*
Dati due eseguibili come argomenti del tipo ls e wc si eseguono in due processi distinti: il primo
deve generare uno stdout redirezionato su un file temporaneo, mentre il secondo deve essere
lanciato solo quando il primo ha finito leggendo lo stesso file come stdin.
Ad esempio ./main ls wc deve avere lo stesso effetto di ls | wc.
*/

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Necessario passare esattamente due eseguibili per argomento\n");
        return 1;
    }

    int outfile = open("./tmp.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    char *arg[] = {NULL};

    for (int i = 1; i < 3; i++)
    {
        if (i == 1)
        {
            if (fork() == 0)
            {
                dup2(outfile, 1);
                execl("/bin/sh", "sh", "-c", argv[1], (char *)NULL);
            }
            int childStatus1;
            wait(&childStatus1);
        }
        else
        {
            if (fork() == 0)
            {
                char *cmd = strncat(argv[2], "<tmp.txt", strlen("<tmp.txt") + 1);
                execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
            }
            int childStatus2;
            wait(&childStatus2);
        }
    }

    return 0;
}