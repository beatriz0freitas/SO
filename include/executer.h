#ifndef EXECUTER_H
#define EXECUTER_H

#include "executer.h"
#include "command.h"
#include "metaInformationDataset.h"
#include "metaInformation.h"


typedef struct Executer Executer;

Executer *executer_new();
char *executer(Command *command);

#endif