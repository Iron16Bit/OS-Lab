#include <string.h>
#include <stdio.h>

// Dati una serie di stringhe passate come argomenti, setta le corrispettivi flag ad 1
int main(int argc, char **argv) {
    int speedUp = 0;
    int saveOnExit = 0;
    int debug = 0;

    if (debug) {
        printf("Debuggo\n");
    }

    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "speedUp") == 0) {
            speedUp = 1;
        } else if (strcmp(argv[i], "saveOnExit") == 0) {
            saveOnExit = 1;
        } else if (strcmp(argv[i], "debug") == 0) {
            debug = 1;
        } else {
            printf("La flag %s non esiste \n", argv[i]);
            return 1;
        }
    }

    printf("Sono state settate a true le seguenti flag:\n");
    if (speedUp) {
        printf("speedUp\n");
    }
    if (saveOnExit) {
        printf("saveOnExit\n");
    }
    if (debug) {
        printf("debug\n");
    }

    return 0;
}