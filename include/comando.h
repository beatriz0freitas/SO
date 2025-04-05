#ifndef COMANDO
#define COMANDO

typedef struct Comando Comando;
Comando *comando_constroi_de_linha(int numArgs, char *linha[]);

int comando_get_num_args(Comando *cmd);
char *comando_get_flag(Comando *cmd);
char *comando_get_arg_por_indice(Comando *cmd, int indice);
char *comando_get_fifo_client(Comando *cmd);

#endif