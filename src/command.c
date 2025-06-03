#include "command.h"


CommandFlag command_parse_flag(char *flagStr, int numArgs) {
    if (strcmp(flagStr, "-a") == 0) return CMD_ADD;
    if (strcmp(flagStr, "-c") == 0) return CMD_CONSULT;
    if (strcmp(flagStr, "-d") == 0) return CMD_DELETE;
    if (strcmp(flagStr, "-l") == 0) return CMD_LIST;
    if (strcmp(flagStr, "-s") == 0) {
        if (numArgs == 1) {
            return CMD_SEARCH;
        }
        return CMD_SEARCH_PROCESSOS;
    }
    if (strcmp(flagStr, "-f") == 0) return CMD_SHUTDOWN;
    return CMD_INVALID;
}

const char *commandFlag_to_string(CommandFlag flag) {
    switch (flag) {
        case CMD_ADD: return "Indexar";
        case CMD_CONSULT: return "Consultar";
        case CMD_DELETE: return "Remover";
        case CMD_LIST: return "Pesquisa n.º de linhas";
        case CMD_SEARCH: return "Listar id's";
        case CMD_SEARCH_PROCESSOS: return "Listar id's com n processos"; // mesma flag
        case CMD_SHUTDOWN: return "Fechar";
        default: return "INVALID";
    }
}

Command command_constroi_de_linha(int numArgs, char *linha[]) {

    Command cmd;
    memset(&cmd, 0, sizeof(Command));
    cmd.flag = CMD_INVALID;

    if (numArgs >= 2) {

        for (int i = 2; i < numArgs && cmd.num_args < MAX_ARGS; i++) {
            strncpy(cmd.args[cmd.num_args], linha[i], MAX_ARG_SIZE - 1);
            cmd.args[cmd.num_args][MAX_ARG_SIZE - 1] = '\0'; // Garantir que a string está terminada
            cmd.num_args++;
        }
        cmd.flag = command_parse_flag(linha[1], cmd.num_args);
    }
    return cmd;
}

int command_get_num_args(Command *cmd) {
    return cmd->num_args;
}

CommandFlag command_get_flag(Command *cmd) {
    return cmd->flag;
}

char *command_get_arg_por_indice(Command *cmd, int indice) {
    if (indice < 0 || indice >= cmd->num_args) 
        return NULL;
    return cmd->args[indice];
}
