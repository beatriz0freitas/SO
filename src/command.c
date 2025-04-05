#include "command.h"

struct Command {
    char *fifo_client ;   // nome do FIFO de resposa do cliente
    char *flag;
    char **args;
    int num_args;
};

Command *command_constroi_de_linha(int numArgs, char *linha[]) {

    Command *cmd = (Command *)malloc(sizeof(Command));
    if (!cmd) {
        perror("Erro ao alocar memória para Command");
        exit(1);
    }   
    cmd->fifo_client = NULL;
    cmd->flag = NULL;
    cmd->args = NULL;
    cmd->num_args = 0;

    if (numArgs > 2){
        cmd->args = (char **)malloc((numArgs - 2) * sizeof(char *));
        if (!cmd->args){
            printf("Erro ao alocar memória\n");
            exit(1);
        }

        cmd->flag = strdup(linha[1]);

        for (int i = 2; i < numArgs; i++){
            cmd->args[cmd->num_args] = strdup(linha[i]);
            cmd->num_args++;
        }

    } else{
        cmd->args = NULL;
    }

    // Criação do FIFO único por cliente
    char fifo_name[256];
    snprintf(fifo_name, sizeof(fifo_name), "fifo/client_%d", getpid());
    cmd->fifo_client = strdup(fifo_name);

    return cmd;
}


int command_get_num_args(Command *cmd)
{
    return cmd->num_args;
}

char *command_get_flag(Command *cmd)
{
    return cmd->flag;
}

char *command_get_arg_por_indice(Command *cmd, int indice)
{
    if (indice < 0 || indice >= cmd->num_args){
        return NULL;
    }
    return cmd->args[indice];
}


char *command_get_fifo_client(Command *cmd) {
    return cmd->fifo_client;
}

void command_set_fifo_client(Command *cmd, char *fifo_client) {
    if (cmd->fifo_client) {
        free(cmd->fifo_client);
    }
    if (!fifo_client) {
        fprintf(stderr, "FIFO do cliente inválido.\n");
        return;
    }

    cmd->fifo_client = strdup(fifo_client);
    if (!cmd->fifo_client) {
        perror("Erro ao duplicar string FIFO");
        exit(1);
    }   
}

void command_free(Command *cmd) {
    if (!cmd) return;
    free(cmd->flag);
    for (int i = 0; i < cmd->num_args; i++) {
        free(cmd->args[i]);
    }
    free(cmd->args);
    free(cmd->fifo_client);
    free(cmd);
}

