#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "executer.h"
#include "message.h"
#include "utils.h"

//quem quer ler - SERVIDOR - normalmente cria o fifo


//TODO: responder paralelo

void dserver_sendResponse(const char *fifo_serverToClient, const char *resposta) {

    // Abre o FIFO do cliente para responder
    int fd_client = open(fifo_serverToClient, O_WRONLY);
    if (fd_client == -1) {
        perror("Erro ao abrir FIFO do cliente para resposta");
        // falta fechar o fd sever se der erro;
        exit(1);
    }

    //TODO: testar com o bufferedWrite
    if (bufferedWrite(fd_client, resposta, strlen(resposta) + 1) == -1) {
        perror("Erro ao enviar resposta");
    }
    
    close(fd_client);
}

void dserver_handleMessage(Message *msg, Executer *executer, MetaInformationDataset *dataset) {
    if (!msg) {
        fprintf(stderr, "Mensagem nula recebida.\n");
        return;
    }

    Command *cmd = message_get_command(msg);
    MetaInformation *info = message_get_metaInformation(msg);

    if (cmd == NULL || info == NULL) {
        fprintf(stderr, "Comando ou MetaInformation inválido.\n");
        exit(1);
    }

    char resposta[100];
    memset(resposta, 0, sizeof(resposta));

    // Executa o comando
    char *resposta_executer = executer_execute(executer, cmd, dataset);
    if (resposta_executer != NULL) {
        strncpy(resposta, resposta_executer, sizeof(resposta) - 1);
    }

    dserver_sendResponse(msg->fifo_client, resposta);
}


int main(int argc, char *argv[]) {
    char *fifo_clientToServer = "../fifos/clientToServer"; // FIFO para o servidor ler as mensagens dos clientes

    // Cria FIFO do servidor (FIFO de leitura do cliente)
    unlink(fifo_clientToServer); // Se já existir, apaga antes de criar
    if (mkfifo(fifo_clientToServer, 0666) == -1 && errno != EEXIST) {
        perror("Erro ao criar fifo_clientToServer");
        exit(1);
    }

    Executer *executer = executer_new();
    MetaInformationDataset *dataset = metaInformationDataset_new();

    while (1) {
        int fd_server = open(fifo_clientToServer, O_RDONLY);
        if (fd_server == -1) {
            perror("Erro ao abrir fifo_clientToServer");
            exit(1);
        }

        Message mensagem;
        ssize_t nbytes = bufferedRead(fd_server, &mensagem, sizeof(Message));

        if (nbytes == 0) {
            // Nenhum dado lido → cliente fechou o FIFO
            close(fd_server);
            continue;
        }

        if (nbytes < 0) {
            perror("Erro ao ler do FIFO");
            close(fd_server);
            continue;
        }

        if (nbytes != sizeof(Message)) {
            fprintf(stderr, "Mensagem inválida recebida. Tamanho esperado: %lu, tamanho lido: %lu\n", sizeof(Message), nbytes);
            close(fd_server);
            continue;
        }

        dserver_handleMessage(&mensagem, executer, dataset);

        close(fd_server);
    }

    unlink(fifo_clientToServer);  // Deleta o FIFO do servidor quando terminar (não chega aqui por causa do while(1))
    executer_free(executer);
    metaInformationDataset_free(dataset);
    
    return 0;

}


