#include "comando.h"
#include <stdlib.h>

struct Comando {
    char fifo_client ;   // nome do FIFO de resposa do cliente
    char *flag;
    char **args;
    int num_args;
};

Comando *comando_constroi_de_linha(int numArgs, char *linha[])
{

    Comando *cmd = (Comando *)malloc(sizeof(Comando));
    cmd->num_args = 0;

    cmd->flag = strdup(linha[1]);

    if (numArgs > 2){
        cmd->args = (char **)malloc((numArgs - 2) * sizeof(char *));

        if (!cmd->args){
            printf("Erro ao alocar memória\n");
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


int comando_get_num_args(Comando *cmd)
{
    return cmd->num_args;
}

char *comando_get_flag(Comando *cmd)
{
    return cmd->flag;
}

char *comando_get_arg_por_indice(Comando *cmd, int indice)
{
    if (indice < 0 || indice >= cmd->num_args){
        return NULL;
    }
    return cmd->args[indice];
}

