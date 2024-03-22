#include <string.h>
#include <stdio.h>

char* stringrev(char* inputStr);
int stringpos(char* str, char chr);

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Necessario passare una stringa ed un carattere come argomento \n");
        return 1;
    } else if (argc > 3) {
        printf("Necessario passare SOLO una stringa ed un carattere come argomento \n");
        return 1;
    }

    if (strlen(argv[2]) > 1) {
        printf("%s non è un carattere, ma stringa \n", argv[2]);
        return 1;
    }

    printf("La stringa passata come argomento è %s \n", argv[1]);
    printf("Il carattere %s si trova in posizione %d \n", argv[2], stringpos(argv[1], argv[2][0]));
    char* inv = stringrev(argv[1]);
    printf("La stringa invertita è %s \n", inv);
    printf("Nell'inversa, %s si trova in posizione %d \n", argv[2], stringpos(inv, argv[2][0]));

    return 0;
}

char* stringrev(char* str) {
    int len = strlen(str);
    for (int i=0, j = len-1; i<=j; i++, j--) {
        char c = str[i];
        str[i] = str[j];
        str[j] = c;
    }

    return str;
}

int stringpos(char* str, char chr) {
    for (int i=0; i<strlen(str); i++) {
        if (str[i] == chr) {
            return i;
        }
    }

    return -1;
}