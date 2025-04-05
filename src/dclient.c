#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "command.h"
#include "metaInformation.h"
#include "utils.h"
#include "message.h"


void sendMessage (Message *message){
    Command *cmd = message_get_command(message);
    MetaInformation *metaInfo = message_get_metaInformation(message);

    char *fifo_client = command_get_fifo_client(cmd);

    //TODO: pasta fifos
	int fd_server = open("fifo/dserver", O_WRONLY);
    if (fd_server == -1) {
        perror("Erro ao abrir FIFO do servidor");
        exit(1);
    }

    //TODO: confirmar se pode ser 512;
    if (bufferedWrite(fd_server, metaInfo, sizeof(MetaInformation)) == -1) {
        perror("Erro ao escrever no FIFO do servidor");
        close(fd_server);
        exit(1);
    }

    close(fd_server);

	int fd_client = open(fifo_client, O_RDONLY);
    if (fd_client == -1) {
        perror("Erro ao abrir FIFO do cliente");
        exit(1);
    }

    char buffer[256];
    ssize_t bytesRead = bufferedRead(fd_client, buffer, sizeof(buffer)-1);
    if (bytesRead == -1) {
        perror("Erro ao ler resposta do servidor");
        close(fd_client);
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

    Command *cmd = command_constroi_de_linha(argc, argv);
    if (cmd == NULL) {
        fprintf(stderr, "Erro ao criar comando\n");
        return 1;
    }

    MetaInformation *info = metaInformation_new();
    if (info == NULL) {
        fprintf(stderr, "Erro ao criar meta informação\n");
        command_free(cmd);
        return 1;
    }

    Message *message = message_new(cmd, info);

    char fifo_client[256];
    snprintf(fifo_client, sizeof(fifo_client), "fifo/client_%d", getpid());

    command_set_fifo_client(cmd, fifo_client);

    unlink(fifo_client);
    if (mkfifo(fifo_client, 0666) == -1) {
        perror("Erro ao criar FIFO do cliente");
        exit(1);
    }

    sendMessage(message);

    message_free(message);
    unlink(fifo_client);
    return 0;
}