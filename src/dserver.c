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
        perror("[SERVER] Erro ao abrir FIFO do cliente para resposta");
        return;
    }

    if (bufferedWrite(fd_client, resposta, strlen(resposta) + 1) == -1) {
        perror("[SERVER] Erro ao escrever resposta");
    }
    
    close(fd_client);
}



int main(int argc, char *argv[]) {
    printf("[DEBUG] Servidor iniciado\n");
    char *fifo_clientToServer = "../fifos/clientToServer"; // FIFO para o servidor ler as mensagens dos clientes

    // Cria FIFO do servidor (FIFO de leitura do cliente)
    unlink(fifo_clientToServer); // Se já existir, apaga antes de criar
    if (mkfifo(fifo_clientToServer, 0666) == -1 && errno != EEXIST) {
        perror("Erro ao criar fifo_clientToServer");
        exit(1);
    }

    printf("[Server] FIFO 'clientToServer' criado. A esperar por mensagens...\n");

    Executer *executer = executer_new();
    MetaInformationDataset *dataset = metaInformationDataset_new();

    int fd_server = open(fifo_clientToServer, O_RDONLY);
    if (fd_server == -1) {
        perror("[Server] Erro ao abrir fifo_clientToServer");
        exit(1);
    }

    //impede que o fd_server entre em EOF - manter um FIFO aberto do lado de leitura.
    int fd_serverDUMMY = open(fifo_clientToServer, O_WRONLY);
    if (fd_serverDUMMY == -1) {
        perror("Erro ao abrir FIFO do servidor para escrita");
        return 1;
    }

    Message mensagem;
    ssize_t nbytes;

    gboolean terminar = FALSE;
    while (bufferedRead(fd_server, &mensagem, sizeof(Message)) > 0 && !terminar) {
    
        Command *cmd = message_get_command(&mensagem);
        MetaInformation *info = message_get_metaInformation(&mensagem);
    
        if (cmd == NULL || info == NULL) {
            fprintf(stderr, "[Server] Comando ou MetaInformation inválido.\n");
            continue;
        }
    
        char resposta[100];
        memset(resposta, 0, sizeof(resposta));
    
        // Executa o comando
        char *resposta_executer = executer_execute(executer, cmd, dataset, &terminar);
        if (resposta_executer != NULL) {
            strncpy(resposta, resposta_executer, sizeof(resposta) - 1);
            free(resposta_executer);
        }

        printf("[DEBUG] Enviando resposta para %s: \"%s\"\n", mensagem.fifo_client, resposta);

        dserver_sendResponse(mensagem.fifo_client, resposta);
    }
    
    if (nbytes == -1) {
        perror("[Server] Erro ao ler do FIFO");
    }
  
    printf("[Server] A encerrar...\n");
    //SHUTDOWN SERVER
    close(fd_server);
    close(fd_serverDUMMY);

    unlink(fifo_clientToServer);
    executer_free(executer);
    metaInformationDataset_free(dataset);

    return 0;

}
