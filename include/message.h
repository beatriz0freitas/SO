#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command.h"
#include "metaInformation.h"

#define MAX_FIFO_NAME 256

typedef struct Message{
    char fifo_client[MAX_FIFO_NAME]; // nome do FIFO de resposta do cliente
    Command cmd;
    MetaInformation info;
} Message;

void message_init(Message *msg, Command *cmd, MetaInformation *info);
Message *message_new(Command *cmd, MetaInformation *info);

void message_set_command(Message *msg, Command *cmd);
void message_set_metaInformation(Message *msg, MetaInformation *info);
void message_set_fifoClient(Message *msg, char *fifo);
Command *message_get_command(Message *msg);
MetaInformation *message_get_metaInformation(Message *msg);
char *message_get_fifoClient(Message *msg);
size_t message_get_size();

void message_free(Message *msg);

#endif