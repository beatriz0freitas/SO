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
#include "command.h"
#include <sys/wait.h>


#define MAX_PATH 256

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

    //mensagem de debug
    char cwd_buffer[MAX_PATH];
    if (getcwd(cwd_buffer, sizeof(cwd_buffer)) != NULL) {
        printf("[DEBUG]: Servidor a correr no diretório: %s\n", cwd_buffer);
    } else {
        perror("[DEBUG]: Erro ao obter o diretório atual (getcwd)");
    }

    if (argc < 3) {
        fprintf(stderr, "Uso: %s <document_folder> <cache_size>\n", argv[0]);
        exit(1);
    }

    // Apanha o caminho da pasta de documentos
    const char *document_folder = argv[1];
    printf("[DEBUG]: Servidor iniciado. Pasta dos documentos: %s\n", document_folder);

    // Apanha o tamanho da cache
    int cache_size = atoi(argv[2]);
    if (cache_size <= 0) {
        fprintf(stderr, "Erro: O tamanho da cache deve ser um número positivo.\n");
        exit(1);
    }

    // FIFO do servidor
    const char *fifo_clientToServer = "fifos/clientToServer";
    unlink(fifo_clientToServer);
    if (mkfifo(fifo_clientToServer, 0666) == -1 && errno != EEXIST) {
        perror("Erro ao criar fifo_clientToServer");
        return 1;
    }
    printf("[Server] FIFO '%s' criado. A esperar por mensagens...\n", fifo_clientToServer);

    Executer *executer = executer_new(); // Instância principal do PAI
    MetaInformationDataset *dataset = metaInformationDataset_new(document_folder, cache_size); // Instância principal do PAI

    int fd_server = open(fifo_clientToServer, O_RDONLY);
    if (fd_server == -1) {
        perror("[Server] Erro ao abrir fifo_clientToServer");
        executer_free(executer);
        metaInformationDataset_free(dataset);
        return 1;
    }

    // Mantém um escritor aberto para não EOF
    int fd_dummy = open(fifo_clientToServer, O_WRONLY);
    if (fd_dummy == -1) {
        perror("[Server] Erro ao abrir fifo_clientToServer para escrita");
        close(fd_server); 
        executer_free(executer);
        metaInformationDataset_free(dataset);
        return 1;
    }

    Message msg;
    ssize_t nbytes;
    gboolean terminar_pai = FALSE; 
    
    while (!terminar_pai && (nbytes = bufferedRead(fd_server, &msg, sizeof(Message))) > 0) {

        // Recolhe processos filhos terminados para evitar zombies
        while (waitpid(-1, NULL, WNOHANG) > 0);


        Command *cmd_ptr = message_get_command(&msg);
        MetaInformation *info_ptr = message_get_metaInformation(&msg); // Mantido como no original

        if (!cmd_ptr || !info_ptr) {
            fprintf(stderr, "[Server] mensagem inválida\n");
            continue;
        }

        CommandFlag current_command_flag = command_get_flag(cmd_ptr);

        // Se for CMD_SHUTDOWN, o PAI marca que o seu loop principal deve terminar
        // APÓS este pedido específico ser despachado (por um filho).
        if (current_command_flag == CMD_SHUTDOWN) {
            printf("[Server Parent] Comando SHUTDOWN recebido. Servidor irá terminar após este pedido.\n");
            terminar_pai = TRUE;
        }

        int is_read_like_operation = 0;
        if (current_command_flag == CMD_CONSULT ||
            current_command_flag == CMD_LIST ||
            current_command_flag == CMD_SEARCH ||
            current_command_flag == CMD_SEARCH_PROCESSOS ||
            current_command_flag == CMD_SHUTDOWN) { // CMD_SHUTDOWN envia a sua resposta através de um filho
            is_read_like_operation = 1;
        }

        if (is_read_like_operation) {
            // Não faz fork para novos pedidos de leitura se o servidor já está marcado para terminar,
            // exceto para o próprio comando CMD_SHUTDOWN que precisa enviar a sua resposta final.
            if (terminar_pai && current_command_flag != CMD_SHUTDOWN) {
                dserver_sendResponse(msg.fifo_client, "Servidor em processo de encerramento. Comando rejeitado.");
                continue;
            }

            pid_t pid = fork();

            if (pid == -1) {
                perror("[Server Parent] Erro no fork");
            } else if (pid == 0) { // Processo Filho

                close(fd_server);
                close(fd_dummy);

                gboolean flag_terminar_para_executer_do_filho = (current_command_flag == CMD_SHUTDOWN);

                char *resp = executer_execute(executer, cmd_ptr, dataset, &flag_terminar_para_executer_do_filho);

                if (resp) {
                    dserver_sendResponse(msg.fifo_client, resp);
                    free(resp);
                } else {
                    char error_msg[128];
                    sprintf(error_msg, sizeof(error_msg), "Erro ao processar o comando: %s", commandFlag_to_string(current_command_flag));
                    dserver_sendResponse(msg.fifo_client, error_msg);

                }
                exit(0);
            }

        } else { // Operações de escrita (CMD_ADD, CMD_DELETE) ou CMD_INVALID - executadas sequencialmente pelo PAI
            if (terminar_pai) { // Se o servidor está a terminar, não processa novas escritas
                dserver_sendResponse(msg.fifo_client, "Servidor em processo de encerramento. Comando rejeitado.");
            } else {
                // A flag 'terminar_pai' é para o loop do pai.
                // Para ADD/DELETE/INVALID, a flag passada a executer_execute não deve ser a 'terminar_pai',
                // pois estas operações em si não terminam o loop do servidor.
                // Se executer_execute precisasse de uma flag de output, seria uma local.
                gboolean dummy_flag_para_executer = FALSE;

                char *resp = executer_execute(executer, cmd_ptr, dataset, &dummy_flag_para_executer);
                if (resp) {
                    dserver_sendResponse(msg.fifo_client, resp);
                    free(resp);
                } else {
                    char error_msg[128];
                    sprintf(error_msg, sizeof(error_msg), "Erro ao processar o comando: %s", commandFlag_to_string(current_command_flag));
                    dserver_sendResponse(msg.fifo_client, error_msg);
                }
            }
        }
    }

    if (nbytes < 0 && errno != EINTR) {
        perror("[Server] erro a ler FIFO");
    }

    printf("[Server] A encerrar...\n");
    close(fd_server);
    close(fd_dummy);
    unlink(fifo_clientToServer);

    // AVISO: Se esta limpeza final não for suficiente e não houver reaping no loop,
    // zombies persistirão até o servidor realmente terminar.

    while(wait(NULL) > 0 || (wait(NULL) == -1 && errno == EINTR));

    metaInformationDataset_free(dataset);
    executer_free(executer);

    return 0;
}