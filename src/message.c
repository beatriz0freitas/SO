#include "message.h"

struct Message{
    Command *cmd;
    MetaInformation *info;
};

Message *message_new(Command *cmd, MetaInformation *info) {
    if (!cmd || !info) {
        fprintf(stderr, "Command ou MetaInformation inválido.\n");
        return NULL;
    }
    
    Message *msg = (Message *)malloc(sizeof(Message));
    if (!msg) {
        perror("Erro ao alocar memória para a mensagem");
        exit(1);
    }
    msg->cmd = cmd;
    msg->info = info;
    return msg;
}

void message_free(Message *msg) {
    if (msg) {
        command_free(msg->cmd);
        metaInformation_free(msg->info);
        free(msg);
    }
}

void message_set_command(Message *msg, Command *cmd) {
    if (msg) {
        msg->cmd = cmd;
    }
}
void message_set_metaInformation(Message *msg, MetaInformation *info) {
    if (msg) {
        msg->info = info;
    }
}
Command *message_get_command(Message *msg) {
    if (msg) {
        return msg->cmd;
    }
    return NULL;
}
MetaInformation *message_get_metaInformation(Message *msg) {
    if (msg) {
        return msg->info;
    }
    return NULL;
}