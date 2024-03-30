#include <stdio.h>  //File streams
#include <unistd.h> //File descriptors
#include <fcntl.h>  //Needed to use the flags for the files

#include <string.h>

int main() {
    int in = open("input.txt", O_RDONLY);

    FILE *out;
    out = fopen("output.txt", "w");

    char toWrite[100];
    int bytesRead = 0;

    do {
        bytesRead = read(in, toWrite, 99);
        toWrite[bytesRead] = 0;
        fputs(toWrite, out);
    } while (bytesRead > 0);

    close(in);
    fclose(out);

    return 0;
}