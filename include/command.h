#ifndef COMMAND
#define COMMAND

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_ARGS 7
#define MAX_ARG_SIZE 256

typedef enum {
    CMD_INVALID = -1,
    CMD_ADD = 1,
    CMD_CONSULT = 2,
    CMD_DELETE = 3,
    CMD_LIST = 4,
    CMD_SEARCH = 5,
    CMD_SEARCH_PROCESSOS = 6,
    CMD_SHUTDOWN = 7
} CommandFlag;

typedef struct Command {
    CommandFlag flag;
    char args[MAX_ARGS][MAX_ARG_SIZE]; // argumentos do comando
    int num_args;
} Command;

CommandFlag command_parse_flag(char *flagStr, int numArgs);
const char *commandFlag_to_string(CommandFlag flag);
Command command_constroi_de_linha(int numArgs, char *linha[]);

int command_get_num_args(Command *cmd);
CommandFlag command_get_flag(Command *cmd);
char *command_get_arg_por_indice(Command *cmd, int indice);

#endif