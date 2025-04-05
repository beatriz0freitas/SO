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

typedef struct Command Command;
Command *command_constroi_de_linha(int numArgs, char *linha[]);


int command_get_num_args(Command *cmd);
char *command_get_flag(Command *cmd);
char *command_get_arg_por_indice(Command *cmd, int indice);
char *command_get_fifo_client(Command *cmd);


#endif