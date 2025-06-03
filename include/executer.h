#ifndef EXECUTER_H
#define EXECUTER_H

#include "command.h"
#include "metaInformationDataset.h"
#include "metaInformation.h"
#include <string.h> 
#include <glib.h>

/**
 * @file executer.h
 * @brief Declarações para o componente responsável por executar comandos sobre o dataset.
 *
 * O módulo Executer é responsável por interpretar comandos e executar as operações apropriadas
 * sobre uma instância de `MetaInformationDataset`, retornando mensagens de resposta formatadas.
 */

/**
 * @struct Executer
 * @brief Estrutura opaca que representa um executor de comandos.
 */
typedef struct Executer Executer;

/**
 * @brief Cria e inicializa uma nova instância de Executer.
 *
 * @return Ponteiro para a nova estrutura Executer alocada.
 */
Executer *executer_new();

/**
 * @brief Liberta os recursos associados ao Executer.
 *
 * @param executer Ponteiro para a estrutura Executer a ser libertada.
 */
void executer_free(Executer *executer);

/**
 * @brief Executa um comando específico sobre o dataset fornecido.
 *
 * Esta função analisa a flag do comando e executa a operação correspondente
 * (ex: adicionar, consultar, remover documentos etc.).
 *
 * @param executer Ponteiro para a estrutura Executer.
 * @param command Ponteiro para o comando a ser executado.
 * @param dataset Ponteiro para o dataset de informações.
 * @param terminar_servidor Ponteiro para flag booleana que indica encerramento do servidor.
 * @return String com a resposta da operação. Deve ser libertada com `g_free` após uso.
 */
char *executer_execute(Executer *executer, Command *command, MetaInformationDataset *dataset, gboolean *terminar_servidor);

#endif // EXECUTER_H
