#include <stdio.h>  //File streams
#include <unistd.h> //File descriptors
#include <fcntl.h>  //Needed to use the flags for the files

#include <string.h>

int main() {
    FILE *in;
    in = fopen("input.txt", "r");

    int out = open("output.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    while (!feof(in)) {
        char c[100];
        fgets(c, 100, in);
        write(out, c, strlen(c));
    }

    fclose(in);
    close(out);

    return 0;
}