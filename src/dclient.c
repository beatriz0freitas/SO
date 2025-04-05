#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "utils.h"

int main (int argc, char * argv[]){

	if (argc < 2) {
		printf("Missing argument.\n");
		_exit(1);
	}

	int fd_server = open(SERVER, O_RDONLY);
    if (fd_server == -1) {
        perror("Erro ao abrir FIFO do servidor");
        return 1;
    }

	int fd_client = open(CLIENT, O_WRONLY);
    if (fd_client == -1) {
        perror("Erro ao abrir FIFO do cliente");
        return 1;
    }

	return 0;
}

