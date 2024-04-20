#include <unistd.h>
#include <stdio.h> 
#include <sys/wait.h>

// Dati come argomento dei binari, si esegue con exec ciascuno di essi

int main(int argc, char* argv[]){
    for (int i=1; i<argc; i++) {
        char * arg[] = {NULL};
        if(fork()==0){
            execv(argv[i],arg);
        }
    }

    int childStatus;
    wait(&childStatus);

    return 0;
}
