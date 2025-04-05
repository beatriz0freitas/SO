#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "utils.h"
#include <errno.h>



int main (int argc, char * argv[]){
    
    if (mkfifo("fifo/dserver", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Erro ao criar fifo/dserver");
            exit(1);
        }
    }
    
}


