#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h> //File descriptors
#include <fcntl.h>  //Needed to use the flags for the files

// Dati come argomento dei binari, si esegue con exec ciascuno di essi e se ne salvano i flussi
// di stdout e stderr in un unico file

int main(int argc, char *argv[])
{
    freopen("output.txt", "w+", stdout);
    freopen("output.txt", "w+", stderr);

    for (int i = 1; i < argc; i++)
    {
        char *arg[] = {NULL};
        if (fork() == 0)
        {
            execv(argv[i], arg);
        }
        else
        {
            fprintf(stderr, "This is the parent node\n");
        }
    }

    int childStatus;
    wait(&childStatus);

    return 0;
}
