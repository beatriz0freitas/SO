#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "command.h"
#include "metaInformation.h"
#include "executer.h"
#include "utils.h"

//quem quer ler - SERVIDOR - normalmente cria o fifo

//TODO: responder paralelo

void handleMessage(MetaInformation *mensagem) {

}

void dserver_sendResponse(const char *fifo_serverToClient, const char *resposta) {

        // Abre o FIFO do cliente para responder
        int fd_client = open(fifo_serverToClient, O_WRONLY);
        if (fd_client == -1) {
            perror("Erro ao abrir FIFO do cliente para resposta");
            // falta fechar o fd sever se der erro;
        }

        //TODO: testar com o bufferedWrite
        write(fd_client, resposta, strlen(resposta) + 1);

        // Fecha os descritores do cliente
        close(fd_client);
}
int main(int argc, char *argv[]) {

    const char *fifo_clientToServer = "../fifos/clientToServer"; // FIFO para o servidor ler as mensagens dos clientes

    // Cria FIFO do servidor (FIFO de leitura do cliente)
    unlink(fifo_clientToServer); // Se já existir, apaga antes de criar
    if (mkfifo(fifo_clientToServer, 0666) == -1 && errno != EEXIST) {
        perror("Erro ao criar fifo_clientToServer");
        exit(1);
    }

    printf("Servidor aguardando clientes...\n");

    while (1) {
        int fd_server = open(fifo_clientToServer, O_RDONLY);
        if (fd_server == -1) {
            perror("Erro ao abrir fifo_clientToServer");
            exit(1);
        }

        char buffer[512]; // Buffer para ler a mensagem do cliente
        ssize_t nbytes = read(fd_server, buffer, sizeof(buffer));

        if (nbytes <= 0) {
            perror("Erro ao ler do FIFO do servidor");
            close(fd_server);
            continue;
        }
        buffer[nbytes] = '\0'; // garante que é uma string

        // Agora sim: separar nome do FIFO e mensagem
        char *fifo_serverToClient = strtok(buffer, " \n");
        char *mensagem = strtok(NULL, "\n");

        if (fifo_serverToClient == NULL || mensagem == NULL) {
            fprintf(stderr, "Mensagem inválida recebida: '%s'\n", buffer);
            close(fd_server);
        }

        printf("Servidor recebeu: '%s' (responder para FIFO '%s')\n", mensagem, fifo_serverToClient);



        // Responde para o cliente
        const char *resposta = "Mensagem recebida com sucesso!";

        dserver_sendResponse(fifo_serverToClient, resposta);

        close(fd_server);
    }

    unlink(fifo_clientToServer);  // Deleta o FIFO do servidor quando terminar (não chega aqui por causa do while(1))
    return 0;

}


