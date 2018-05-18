#include <ctype.h>
#include <stdlib.h>

#include <unistd.h>


int main(int argc, char** argv){
    for (int i = 1; i < argc; i++) {
        pid_t c = fork();

        if ( c == 0) {
            execlp("./noob","./noob",argv[i],(char*)0);
        }
    }
}
