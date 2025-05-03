#ifndef EXECUTER_H
#define EXECUTER_H

#include "executer.h"
#include "command.h"
#include "metaInformationDataset.h"
#include "metaInformation.h"
#include <string.h> 



typedef struct Executer Executer;

Executer *executer_new();
void executer_free(Executer *executer);
char *executer_execute(Executer *executer, Command *command, MetaInformationDataset *dataset, gboolean *terminar_servidor);

#endif