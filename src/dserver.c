#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "utils.h"


int main (int argc, char * argv[]){

	//garante que nao existem FIFOs anteriores com o memso nome
	unlink(SERVER);
    if (mkfifo(SERVER, 0666) == -1) {
        perror("Erro ao criar FIFO do servidor");
        return 1;
    }

	unlink(CLIENT);
    if (mkfifo(CLIENT, 0666) == -1) {
        perror("Erro ao criar FIFO do cliente");
        return 1;
    }
	
	while (1) {
        // Abrir FIFO do servidor para leitura
        int fd_server = open(SERVER, O_RDONLY);
        if (fd_server == -1) {
            perror("Erro ao abrir FIFO do servidor");
            return 1;
        }
    }
	return 0;
}


