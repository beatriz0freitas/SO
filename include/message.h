#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command.h"
#include "metaInformation.h"

typedef struct Message Message;

Message *message_new(Command *cmd, MetaInformation *info) ;
void message_free(Message *msg);
void message_set_command(Message *msg, Command *cmd);
void message_set_metaInformation(Message *msg, MetaInformation *info);
Command *message_get_command(Message *msg);
MetaInformation *message_get_metaInformation(Message *msg);


#endif