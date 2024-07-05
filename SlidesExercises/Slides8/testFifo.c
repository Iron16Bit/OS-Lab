#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <signal.h>

int main()
{
    char *fifoPath = "/tmp/test.fifo";
    mkfifo(fifoPath, S_IRUSR | S_IWUSR);

    // int fifoW = open(fifoPath, O_WRONLY);
    // char buff[256];
    // strcpy(buff, "TEST");
    // write(fifoW, buff, strlen(buff)+1);
    // close(fifoW);

    char buff[256];
    while (1)
    {
        int fifoR = open(fifoPath, O_RDONLY);
        int r = read(fifoR, buff, sizeof(buff));
        printf("Read: %sLength: %d\n", buff, r);
        close(fifoR);
    }

    return 0;
}