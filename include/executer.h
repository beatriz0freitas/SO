#ifndef EXECUTER_H
#define EXECUTER_H

typedef struct Executer Executer;

Executer *executer_new();
char *executer(Comando *comando);

#endif