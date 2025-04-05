#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "comando.h"
#include "metaInformation.h"
#include "utils.h"

void sendMessage (Information *message, Command *comando){
    //TODO: pasta fifos
	int fd_server = open("fifo/dserver", O_WRONLY);
    if (fd_server == -1) {
        perror("Erro ao abrir FIFO do servidor");
        exit(1);
    }

    write(fd_server, message, sizeof(Information));
    close(fd_server);

    //TODO: adicionar fifo_client ao comando
	int fd_client = open(getFIFO(comando->fifo_client), O_RDONLY);
    if (fd_client == -1) {
        perror("Erro ao abrir FIFO do cliente");
        exit(1);
    }
    char buffer[256];
    ssize_t bytesRead = bufferedRead(fd_client, buffer, sizeof(buffer));

    //TODO: confirmar se podemos usar printf
    buffer[bytesRead] = '\0';
    printf("Resposta do servidor: %s\n", buffer);

    close(fd_client);
    unlink(getFIFIO(comando->fifo_client));
}

