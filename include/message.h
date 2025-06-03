#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command.h"
#include "metaInformation.h"

/**
 * @file message.h
 * @brief Estrutura e funções auxiliares para comunicação entre cliente e servidor via FIFO.
 *
 * A estrutura Message encapsula um comando, seus dados associados e o nome do FIFO
 * de retorno do cliente. Essa estrutura é usada para troca de mensagens entre processos.
 */

#define MAX_FIFO_NAME 256 ///< Tamanho máximo para o nome do FIFO do cliente.

/**
 * @struct Message
 * @brief Representa uma mensagem trocada entre cliente e servidor.
 *
 * Contém um comando (`Command`), as informações de metadados (`MetaInformation`),
 * e o nome do FIFO do cliente para resposta.
 */
typedef struct Message {
    char fifo_client[MAX_FIFO_NAME]; ///< Nome do FIFO do cliente.
    Command cmd;                     ///< Comando a ser executado.
    MetaInformation info;           ///< Dados associados ao comando.
} Message;

/**
 * @brief Inicializa uma estrutura Message com um comando e metainformação.
 *
 * @param msg Ponteiro para a estrutura Message a ser inicializada.
 * @param cmd Ponteiro para o comando.
 * @param info Ponteiro para os dados do documento.
 */
void message_init(Message *msg, Command *cmd, MetaInformation *info);

/**
 * @brief Define o comando da mensagem.
 *
 * @param msg Ponteiro para a estrutura Message.
 * @param cmd Ponteiro para o comando.
 */
void message_set_command(Message *msg, Command *cmd);

/**
 * @brief Define as metainformações da mensagem.
 *
 * @param msg Ponteiro para a estrutura Message.
 * @param info Ponteiro para os metadados.
 */
void message_set_metaInformation(Message *msg, MetaInformation *info);

/**
 * @brief Define o nome do FIFO do cliente.
 *
 * @param msg Ponteiro para a estrutura Message.
 * @param fifo Nome do FIFO do cliente.
 */
void message_set_fifoClient(Message *msg, char *fifo);

/**
 * @brief Obtém um ponteiro para o comando da mensagem.
 *
 * @param msg Ponteiro para a estrutura Message.
 * @return Ponteiro para o comando.
 */
Command *message_get_command(Message *msg);

/**
 * @brief Obtém um ponteiro para a metainformação da mensagem.
 *
 * @param msg Ponteiro para a estrutura Message.
 * @return Ponteiro para os metadados.
 */
MetaInformation *message_get_metaInformation(Message *msg);

/**
 * @brief Retorna o nome do FIFO do cliente associado à mensagem.
 *
 * @param msg Ponteiro para a estrutura Message.
 * @return Ponteiro para o nome do FIFO.
 */
char *message_get_fifoClient(Message *msg);

/**
 * @brief Retorna o tamanho em bytes da estrutura Message.
 *
 * Útil para chamadas como `read()` e `write()` com FIFOs.
 *
 * @return Tamanho da estrutura Message.
 */
size_t message_get_size();

#endif // MESSAGE_H
