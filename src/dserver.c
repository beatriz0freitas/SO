#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include "executer.h"
#include "message.h"
#include "utils.h"

#define MAX_PATH 256

// envia a resposta de volta ao FIFO do cliente
void dserver_sendResponse(const char *fifo_serverToClient, const char *resposta) {
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
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <document_folder>\n", argv[0]);
        return 1;
    }

    // Apanha o caminho da pasta de documentos
    const char *document_folder = argv[1];
    printf("[DEBUG]: Servidor iniciado. Pasta dos documentos: %s\n", document_folder);

    // FIFO do servidor
    const char *fifo_clientToServer = "fifos/clientToServer";
    unlink(fifo_clientToServer);
    if (mkfifo(fifo_clientToServer, 0666) == -1 && errno != EEXIST) {
        perror("Erro ao criar fifo_clientToServer");
        return 1;
    }
    printf("[Server] FIFO '%s' criado. A esperar por mensagens...\n", fifo_clientToServer);

    // Inicializa estruturas
    Executer *executer = executer_new();
    MetaInformationDataset *dataset = metaInformationDataset_new(document_folder);


    //metaInformationDataset_load(dataset);

    // Abre FIFO para leitura
    int fd_server = open(fifo_clientToServer, O_RDONLY);
    if (fd_server == -1) {
        perror("[Server] Erro ao abrir fifo_clientToServer");
        return 1;
    }
    // Mantém um escritor aberto para não EOF
    int fd_dummy = open(fifo_clientToServer, O_WRONLY);
    if (fd_dummy == -1) {
        perror("[Server] Erro ao abrir fifo_clientToServer para escrita");
        return 1;
    }

    Message msg;
    ssize_t nbytes;
    gboolean terminar = FALSE;

    while ((nbytes = bufferedRead(fd_server, &msg, sizeof(Message))) > 0 && !terminar) {
        Command *cmd = message_get_command(&msg);
        MetaInformation *info = message_get_metaInformation(&msg);
        if (!cmd || !info) {
            fprintf(stderr, "[Server] mensagem inválida\n");
            continue;
        }

        // Executa
        char *resp = executer_execute(executer, cmd, dataset, &terminar);
        printf("[DEBUG] Enviando resposta para %s: \"%s\"\n", msg.fifo_client, resp);
        dserver_sendResponse(msg.fifo_client, resp);
        free(resp);
    }
    if (nbytes < 0) perror("[Server] erro a ler FIFO");

    printf("[Server] A encerrar...\n");
    close(fd_server);
    close(fd_dummy);
    unlink(fifo_clientToServer);

    // Guarda estado e liberta
    //metaInformationDataset_store(dataset);
    metaInformationDataset_free(dataset);
    executer_free(executer);

    return 0;
}
