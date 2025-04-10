#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "command.h"
#include "metaInformation.h"
#include "utils.h"
#include "message.h"


void dclient_sendMessage (const char* fifo_serverToClient){

    // FIFO para enviar a mensagem para o servidor
    const char *fifo_clientToServer = "../fifos/clientToServer";

    // Abre o FIFO do servidor para escrita
    int fd_server = open(fifo_clientToServer, O_WRONLY);
    if (fd_server == -1) {
        perror("Erro ao abrir fifo_server para escrita");
        unlink(fifo_serverToClient);
    }

    // Envia a mensagem (com o nome do FIFO de resposta)
    char mensagem[512];
    snprintf(mensagem, sizeof(mensagem), "%s %s", fifo_serverToClient, "Olá, servidor!");  // FIFO + mensagem
    write(fd_server, mensagem, strlen(mensagem) + 1);
    close(fd_server);  // Fecha o FIFO do servidor após enviar a mensagem

}


/*
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
*/

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

    // Criação de um FIFO único para cada cliente
    char fifo_serverToClient[256];
    snprintf(fifo_serverToClient, sizeof(fifo_serverToClient), "../fifos/serverToClient_%d", getpid());

    // Cria o FIFO para resposta
    unlink(fifo_serverToClient); // Se já existir, apaga antes de criar
    if (mkfifo(fifo_serverToClient, 0666) == -1) {
        perror("Erro ao criar FIFO do cliente");
        return 1;
    }


    dclient_sendMessage (fifo_serverToClient);

    // Abre o FIFO do cliente para ler a resposta
    int fd_client = open(fifo_serverToClient, O_RDONLY);
    if (fd_client == -1) {
        perror("Erro ao abrir fifo_cliente para leitura");
        unlink(fifo_serverToClient);
        return 1;
    }

    // Lê a resposta do servidor
    char buffer[256];
    ssize_t nbytes = read(fd_client, buffer, sizeof(buffer));
    if (nbytes <= 0) {
        perror("Erro ao ler resposta do FIFO");
        close(fd_client);
        unlink(fifo_serverToClient);
        return 1;
    }
    buffer[nbytes] = '\0';

    printf("Cliente recebeu: %s\n", buffer);

    close(fd_client);  // Fecha o FIFO do cliente
    unlink(fifo_serverToClient);  // Remove o FIFO do cliente

    return 0;
}

