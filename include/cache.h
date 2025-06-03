#ifndef CACHE_H
#define CACHE_H

#include "metaInformation.h"
#include <glib.h>

/**
 * @file cache.h
 * @brief Interface para um cache com política FIFO baseada em GQueue e GHashTable.
 *
 * Este módulo implementa um cache simples que armazena pares chave-valor onde
 * a chave é um inteiro e o valor é um ponteiro para uma estrutura MetaInformation.
 * O cache possui tamanho máximo definido e remove o elemento mais antigo (FIFO)
 * quando ultrapassa esse tamanho.
 */

/**
 * @struct Cache
 * @brief Estrutura que representa o cache.
 *
 * Contém uma fila (GQueue) para controlar a ordem de inserção dos elementos
 * e uma tabela de hash (GHashTable) para acesso rápido às entradas.
 */
typedef struct Cache {
    GQueue *queue;     /**< Fila que mantém a ordem dos elementos inseridos. */
    GHashTable *table; /**< Tabela de hash para busca eficiente. */
    int max_size;      /**< Número máximo de elementos no cache. */
} Cache;

/**
 * @brief Cria uma nova cache com tamanho máximo definido.
 *
 * @param max_size Número máximo de elementos permitidos no cache.
 * @return Ponteiro para a nova estrutura Cache.
 */
Cache *cache_new(int max_size);

/**
 * @brief Libera toda a memória associada à cache.
 *
 * @param cache Ponteiro para a estrutura Cache a ser liberada.
 */
void cache_free(Cache *cache);

/**
 * @brief Recupera um elemento da cache baseado na chave.
 *
 * @param cache Ponteiro para a estrutura Cache.
 * @param key Chave inteira do elemento a ser recuperado.
 * @return Ponteiro para o MetaInformation associado ou NULL se não existir.
 */
MetaInformation *cache_get(Cache *cache, int key);

/**
 * @brief Insere um novo elemento na cache.
 *
 * Se a chave já existir, atualiza o valor e move a chave para o fim da fila.
 * Se o cache estiver cheio, remove o elemento mais antigo (FIFO).
 *
 * @param cache Ponteiro para a estrutura Cache.
 * @param key Chave inteira a ser inserida.
 * @param value Ponteiro para a estrutura MetaInformation associada à chave.
 */
void cache_put(Cache *cache, int key, MetaInformation *value);

/**
 * @brief Remove um elemento da cache baseado na chave.
 *
 * @param cache Ponteiro para a estrutura Cache.
 * @param key Chave inteira do elemento a ser removido.
 */
void cache_remove(Cache *cache, int key);

#endif // CACHE_H
