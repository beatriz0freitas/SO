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

	//enviar pedido ao servidor
	int fd_server = open(SERVER, O_WRONLY);
    if (fd_server == -1) {
        perror("Erro ao abrir FIFO do servidor");
        return 1;
    }
	
	//Abrir FIFO cliente para leitura de respostas
	int fd_client = open(CLIENT, O_RDONLY);
    if (fd_client == -1) {
        perror("Erro ao abrir FIFO do cliente para leitura");
        return 1;
    }

	return 0;
}

