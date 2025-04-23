#include "message.h"


void message_init(Message *msg, Command *cmd, MetaInformation *info) {
    if (!cmd || !info) {
        fprintf(stderr, "Command ou MetaInformation inválido ao iniciar mensagem.\n");
        return;
    }
    if (!msg) {
        fprintf(stderr, "Mensagem inválida.\n");
        return;
    }
    msg->cmd = *cmd;
    msg->info = *info;
    memset(msg->fifo_client, 0, MAX_FIFO_NAME);  // Inicializar o nome do FIFO
}

Message *message_new(Command *cmd, MetaInformation *info) {
    Message msg;
    message_init(&msg, cmd, info);
    return &msg;
}

void message_set_command(Message *msg, Command *cmd) {
    if (msg && cmd) 
        msg->cmd = *cmd;
}
void message_set_metaInformation(Message *msg, MetaInformation *info) {
    if (msg && info) 
        msg->info = *info;
}

Command *message_get_command(Message *msg) {
    if (msg) {
        return &msg->cmd;
    }
    return NULL;
}

MetaInformation *message_get_metaInformation(Message *msg) {
    if (msg) {
        return &msg->info;
    }
    return NULL;
}
