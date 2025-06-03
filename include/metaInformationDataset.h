#ifndef METAINFORMATIONDATASET_C
#define METAINFORMATIONDATASET_C

#include "metaInformation.h"
#include <glib.h>
#include "cache.h"

#define MAX_PATH 256
#define CODIGOJAINDEXADO 20000

/**
 * Estrutura que representa um dataset de metainformação.
 * Contém uma fila de posições livres, índice sequencial, caminhos para os ficheiros,
 * e uma cache de metainformação.
 */
typedef struct MetaInformationDataset {
    GQueue *MetaInformationQueue;  ///< Fila para guardar IDs reutilizáveis de documentos apagados
    int nextindex;                 ///< Próximo índice disponível para novo documento
    char folder[MAX_PATH];         ///< Caminho da pasta onde os documentos estão localizados
    char filename[MAX_PATH];       ///< Nome do ficheiro binário que guarda as MetaInformation
    Cache *cache;                  ///< Cache para acelerar acessos a MetaInformation
} MetaInformationDataset;

/**
 * Cria um novo MetaInformationDataset.
 * 
 * @param document_folder Caminho para a pasta dos documentos.
 * @param cache_size Tamanho da cache de metainformação.
 * @return Ponteiro para a nova estrutura MetaInformationDataset.
 */
MetaInformationDataset *metaInformationDataset_new(const char *document_folder, int cache_size);

/**
 * Adiciona uma nova MetaInformation ao dataset.
 * 
 * @param dataset Ponteiro para a estrutura MetaInformationDataset.
 * @param metaInfo MetaInformation a ser adicionada.
 * @return ID atribuído ao documento, ou um valor negativo em caso de erro.
 */
int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo);

int metaInformationDataset_add_with_cache_write_through(MetaInformationDataset *dataset, MetaInformation *metaInfo);

/**
 * Remove logicamente uma MetaInformation do dataset (tombstone).
 * 
 * @param dataset Ponteiro para o dataset.
 * @param key ID do documento a remover.
 * @return TRUE se a remoção foi bem-sucedida, FALSE caso contrário.
 */
gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key);

/**
 * Consulta uma MetaInformation por ID.
 * 
 * @param dataset Ponteiro para o dataset.
 * @param key ID do documento.
 * @return Ponteiro para MetaInformation, ou NULL se não existir ou estiver apagada.
 */
MetaInformation *metaInformationDataset_consult(MetaInformationDataset *dataset, int key);

/**
 * Conta quantas linhas de um documento contêm uma determinada palavra-chave.
 * 
 * @param dataset Ponteiro para o dataset.
 * @param id ID do documento.
 * @param keyword Palavra-chave a procurar.
 * @return Número de linhas que contêm a palavra, ou -1 em caso de erro.
 */
int metaInformationDataset_count_keyword_lines(MetaInformationDataset *dataset, int id, const char *keyword);

/**
 * Procura documentos que contenham uma determinada palavra-chave.
 * Esta é a implementação padrão (possivelmente sequencial).
 * 
 * @param dataset Ponteiro para o dataset.
 * @param keyword Palavra-chave a procurar.
 * @return String com IDs dos documentos encontrados, no formato JSON (ex: "[1, 2, 5]").
 */
char *metaInformationDataset_search_documents(MetaInformationDataset *dataset, const char *keyword);

/**
 * Procura sequencialmente documentos que contenham uma palavra-chave.
 * 
 * @param dataset Ponteiro para o dataset.
 * @param keyword Palavra-chave a procurar.
 * @return String com IDs dos documentos encontrados, no formato JSON.
 */
char *metaInformationDataset_search_documents_sequential(MetaInformationDataset *dataset, const char *keyword);

/**
 * Procura paralelamente documentos que contenham uma palavra-chave.
 * 
 * @param dataset Ponteiro para o dataset.
 * @param keyword Palavra-chave a procurar.
 * @param max_procs Número máximo de processos paralelos a usar.
 * @return String com IDs dos documentos encontrados, no formato JSON.
 */
char *metaInformationDataset_search_documents_parallel(MetaInformationDataset *dataset, const char *keyword, int max_procs);

/**
 * Liberta todos os recursos associados ao dataset.
 * 
 * @param dataset Ponteiro para o dataset a ser destruído.
 */
void metaInformationDataset_free(MetaInformationDataset *dataset);

#endif // METAINFORMATIONDATASET_C
