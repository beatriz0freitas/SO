#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "command.h"
#include "metaInformation.h"
#include "utils.h"

void sendMessage (MetaInformation *message, Command *command){
    char *fifo_client = command_get_fifo_client(command);

    //TODO: pasta fifos
	int fd_server = open("fifo/dserver", O_WRONLY);
    if (fd_server == -1) {
        perror("Erro ao abrir FIFO do servidor");
        unlink(fifo_client);
        exit(1);
    }

    //TODO: confirmar se pode ser 512;
    if (write(fd_server, message, sizeof(MetaInformation)) == -1) {
        perror("Erro ao escrever no FIFO do servidor");
        close(fd_server);
        unlink(fifo_client);
        exit(1);
    }

    close(fd_server);

	int fd_client = open(fifo_client, O_RDONLY);
    if (fd_client == -1) {
        perror("Erro ao abrir FIFO do cliente");
        unlink(fifo_client);
        exit(1);
    }

    char buffer[256];
    ssize_t bytesRead = bufferedRead(fd_client, buffer, sizeof(buffer)-1);
    if (bytesRead == -1) {
        perror("Erro ao ler resposta do servidor");
        close(fd_client);
        unlink(fifo_client);
        exit(1);
    }

    //TODO: confirmar se podemos usar printf
    buffer[bytesRead] = '\0';
    printf("Resposta do servidor: %s\n", buffer);

    close(fd_client);
    unlink(fifo_client);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso inválido\n");
        return 1;
    }

    Command *command = command_constroi_de_linha(argc, argv);

    char fifo_client[256];
    snprintf(fifo_client, sizeof(fifo_client), "fifo/client_%d", getpid());

    command_set_fifo_client(command, fifo_client);

    if (mkfifo(fifo_client, 0666) == -1) {
        perror("Erro ao criar FIFO do cliente");
        exit(1);
    }

    MetaInformation *message = metaInformation_new();

    sendMessage(message, command);

    metaInformation_free(message);
    command_free(command);
    unlink(fifo_client);
    return 0;
}