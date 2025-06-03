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


void dclient_sendMessage (const char* fifo_serverToClient, Message *msg) {
    // FIFO para enviar a mensagem para o servidor
    const char *fifo_clientToServer = "fifos/clientToServer";

    // Abre o FIFO do servidor para escrita
    int fd_server = open(fifo_clientToServer, O_WRONLY);
    if (fd_server == -1) {
        perror("Erro ao abrir fifo_server para escrita");
        unlink(fifo_serverToClient);
        exit(1);
    }

    ssize_t bytesWritten = bufferedWrite(fd_server, msg, sizeof(Message));
    if (bytesWritten != sizeof(Message)) {
        fprintf(stderr, "[CLIENTE] Erro ao escrever a mensagem completa para o servidor\n");
        close(fd_server);
        unlink(fifo_serverToClient);
        exit(1);
    }

    close(fd_server);
    printf("[CLIENTE] Mensagem enviada para o servidor\n");
}

void dclient_receiveMessage (const char* fifo_serverToClient) {
    // Abre o FIFO do cliente para leitura da resposta do servidpor
	int fd_client = open(fifo_serverToClient, O_RDONLY);
    if (fd_client == -1) {
        perror("Erro ao abrir FIFO do cliente para leitura");
        unlink(fifo_serverToClient);
        exit(1);
    }

    char buffer[512];
    ssize_t bytesRead = bufferedRead(fd_client, buffer, sizeof(buffer)-1);
    if (bytesRead == -1) {
        perror("Erro ao ler resposta do servidor");
    }

    printf("[CLIENTE] Resposta do servidor: %s\n", buffer);

    close(fd_client); 
    unlink(fifo_serverToClient);
}


int main(int argc, char *argv[]) {

    //mensagem de debug
    char cwd_buffer[256];
    if (getcwd(cwd_buffer, sizeof(cwd_buffer)) != NULL) {
        printf("[DEBUG]: Servidor a correr no diretório: %s\n", cwd_buffer);
    } else {
        perror("[DEBUG]: Erro ao obter o diretório atual (getcwd)");
    }

    if (argc < 2) {
        fprintf(stderr, "Uso inválido\n");
        exit(1);
    }

    Command cmd = command_constroi_de_linha(argc, argv);
    if (cmd.flag == CMD_INVALID) {
        fprintf(stderr, "Comando inválido\n");
        exit(1);
    }

    MetaInformation info = metaInformation_new();
    
    // Criação de um FIFO único para cada cliente
    char fifo_serverToClient[256];
    snprintf(fifo_serverToClient, sizeof(fifo_serverToClient), "fifos/serverToClient_%d", getpid());

    // Cria o FIFO para resposta
    unlink(fifo_serverToClient); // Se já existir, apaga antes de criar
    if (mkfifo(fifo_serverToClient, 0666) == -1) {
        perror("Erro ao criar FIFO do cliente");
        exit(1);
    }

    Message message;
    message_init(&message, &cmd, &info);
    strncpy(message.fifo_client, fifo_serverToClient, sizeof(message.fifo_client) - 1);

    dclient_sendMessage (fifo_serverToClient, &message);
    dclient_receiveMessage (fifo_serverToClient);

    printf("[CLIENT PID %d] Finished operations, exiting.\n", getpid()); 

    return 0;
}

