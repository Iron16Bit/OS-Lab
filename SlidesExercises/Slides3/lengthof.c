#include <string.h>
#include <stdio.h>

// Ritorna la lunghezza della stringa passata come argomento
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Necessario passare una stringa come argomento \n");
        return 1;
    }

    int len = strlen(argv[1]);

    printf("La stringa è lunga %d caratteri \n", len);

    return 0;
}