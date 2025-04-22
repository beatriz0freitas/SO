#ifndef EXECUTER_H
#define EXECUTER_H

#include "executer.h"
#include "command.h"
#include "metaInformationDataset.h"
#include "metaInformation.h"
#include <string.h> 



typedef struct Executer Executer;

Executer *executer_new();
char *executer_execute(Executer *executer, Command *command, MetaInformationDataset *dataset);

#endif