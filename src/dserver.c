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

    //mensagem de debug
    char cwd_buffer[MAX_PATH];
    if (getcwd(cwd_buffer, sizeof(cwd_buffer)) != NULL) {
        printf("[DEBUG]: Servidor a correr no diretório: %s\n", cwd_buffer);
    } else {
        perror("[DEBUG]: Erro ao obter o diretório atual (getcwd)");
    }


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
    Executer *executer = executer_new(); // Instância principal do PAI
    MetaInformationDataset *dataset = metaInformationDataset_new(document_folder); // Instância principal do PAI

    //metaInformationDataset_load(dataset);

    // Abre FIFO para leitura
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
    gboolean terminar_pai = FALSE; // Flag do PAI para controlar o seu próprio loop principal.
                                   // A 'terminar' original era passada para executer_execute.

    // TODO: Adicionar gestão de processos filho zombie aqui (ex: SIGCHLD handler ou waitpid com WNOHANG no loop).
    // AVISO: Sem esta gestão, processos zombie SERÃO criados e acumular-se-ão!


    while (!terminar_pai && (nbytes = bufferedRead(fd_server, &msg, sizeof(Message))) > 0) {

        // Recolhe processos filhos terminados para evitar zombies
        while (waitpid(-1, NULL, WNOHANG) > 0);


        Command *cmd_ptr = message_get_command(&msg);
        MetaInformation *info_ptr = message_get_metaInformation(&msg); // Mantido como no original

        if (!cmd_ptr || !info_ptr) {
            fprintf(stderr, "[Server] mensagem inválida\n");
            // TODO: Considerar enviar uma resposta de erro específica ao cliente aqui.
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
                // TODO: Enviar uma resposta de erro ao cliente se o fork falhar.
            } else if (pid == 0) { // Processo Filho

                close(fd_server);
                close(fd_dummy);

                // O filho usa os ponteiros 'executer' e 'dataset' do pai.
                // O SO trata da cópia de memória (Copy-on-Write) se o filho tentar
                // modificar o conteúdo das estruturas apontadas (o que não deve
                // acontecer para leituras). O acesso ao ficheiro 'information.bin' é partilhado.

                gboolean flag_terminar_para_executer_do_filho = (current_command_flag == CMD_SHUTDOWN);

                // TODO ALERTA LOCKS (FILHO - LEITURA): As funções em MetaInformationDataset
                // que são chamadas por executer_execute (ex: _consult, _search) DEVEM
                // implementar LOCK_SH (shared lock) para proteger o acesso ao ficheiro 'information.bin'.
                char *resp = executer_execute(executer, cmd_ptr, dataset, &flag_terminar_para_executer_do_filho);

                if (resp) {
                    dserver_sendResponse(msg.fifo_client, resp);
                    free(resp);
                } else {
                    char error_msg[128];
                    sprintf(error_msg, sizeof(error_msg), "Erro ao processar o comando: %s", commandFlag_to_string(current_command_flag));
                    dserver_sendResponse(msg.fifo_client, error_msg);

                }

                // TODO: Numa implementação final, o filho deveria libertar quaisquer recursos
                // que sejam efetivamente copiados para ele ou que ele aloque.
                // Para "mexe o menos possível", esta libertação é omitida aqui, confiando no SO.
                // Ex: executer_free(executer_copia_filho); dataset_free(dataset_copia_filho);

                // TODO: O filho DEVE terminar explicitamente com exit() após completar a sua tarefa.
                exit(0);
            }
            // Processo Pai continua o loop IMEDIATAMENTE.
            // NÃO espera pelo filho aqui (o que levará a processos zombie).
        } else { // Operações de escrita (CMD_ADD, CMD_DELETE) ou CMD_INVALID - executadas sequencialmente pelo PAI
            if (terminar_pai) { // Se o servidor está a terminar, não processa novas escritas
                dserver_sendResponse(msg.fifo_client, "Servidor em processo de encerramento. Comando rejeitado.");
            } else {
                // A flag 'terminar_pai' é para o loop do pai.
                // Para ADD/DELETE/INVALID, a flag passada a executer_execute não deve ser a 'terminar_pai',
                // pois estas operações em si não terminam o loop do servidor.
                // Se executer_execute precisasse de uma flag de output, seria uma local.
                gboolean dummy_flag_para_executer = FALSE;

                // TODO ALERTA LOCKS (PAI - ESCRITA): As funções em MetaInformationDataset
                // que são chamadas por executer_execute (ex: _add, _remove) DEVEM
                // implementar LOCK_EX (exclusive lock) para proteger o acesso ao ficheiro 'information.bin'.
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

    // Guarda estado e liberta
    //metaInformationDataset_store(dataset);

    // TODO: Antes de libertar 'dataset' e 'executer', é crucial garantir que todos os
    // processos filho tenham terminado, especialmente se eles usam os mesmos ponteiros (mesmo
    // que o SO faça copy-on-write para memória, descritores de ficheiro dentro dessas
    // estruturas poderiam ser um problema se não geridos cuidadosamente).
    // Um loop com wait() bloqueante é uma forma de esperar por todos os filhos.

    // AVISO: Se esta limpeza final não for suficiente e não houver reaping no loop,
    // zombies persistirão até o servidor realmente terminar.

    while(wait(NULL) > 0 || (wait(NULL) == -1 && errno == EINTR));

    metaInformationDataset_free(dataset);
    executer_free(executer);

    return 0;
}