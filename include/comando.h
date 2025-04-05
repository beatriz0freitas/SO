#ifndef COMANDO
#define COMANDO

typedef struct Command Command;
Command *comando_constroi_de_linha(int numArgs, char *linha[]);


int comando_get_num_args(Command *cmd);
char *comando_get_flag(Command *cmd);
char *comando_get_arg_por_indice(Command *cmd, int indice);
char *comando_get_fifo_client(Command *cmd);


#endif