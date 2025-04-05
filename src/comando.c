#include "comando.h"
#include <stdlib.h>
#include <string.h>

struct Command {
    char *fifo_client ;   // nome do FIFO de resposa do cliente
    char *flag;
    char **args;
    int num_args;
};

Command *comando_constroi_de_linha(int numArgs, char *linha[])
{

    Command *cmd = (Command *)malloc(sizeof(Command));
    cmd->num_args = 0;

    cmd->flag = strdup(linha[1]);

    if (numArgs > 2){
        cmd->args = (char **)malloc((numArgs - 2) * sizeof(char *));

        if (!cmd->args){
            exit(1);
        }

        for (int i = 2; i < numArgs; i++){
            cmd->args[cmd->num_args] = strdup(linha[i]);
            cmd->num_args++;
        }
    }
    else{
        cmd->args = NULL;
    }

    return cmd;
}


int comando_get_num_args(Command *cmd)
{
    return cmd->num_args;
}

char *comando_get_flag(Command *cmd)
{
    return cmd->flag;
}

char *comando_get_arg_por_indice(Command *cmd, int indice)
{
    if (indice < 0 || indice >= cmd->num_args){
        return NULL;
    }
    return cmd->args[indice];
}



char *comando_get_fifo_client(Command *cmd)
{
    return cmd->fifo_client;
}