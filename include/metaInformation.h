#ifndef META_INFORMATION_H
#define META_INFORMATION_H

#include <stddef.h>
#include <stdlib.h>
#include <glib.h>

/**
 * @file metaInformation.h
 * @brief Estrutura e funções para armazenar e manipular metadados de documentos.
 *
 * Esta estrutura representa as informações essenciais de um documento indexado,
 * incluindo título, autor, ano, caminho, e palavras-chave.
 */

/**
 * @struct MetaInformation
 * @brief Estrutura que contém os metadados de um documento.
 *
 * Essa estrutura é utilizada para indexação, busca e consulta de documentos.
 */
typedef struct MetaInformation {
    int idDocument;             ///< ID único do documento.
    char documentTitle[200];   ///< Título do documento.
    char author[200];          ///< Autor(es) do documento, separados por ponto e vírgula.
    int year;                  ///< Ano de publicação.
    char path[64];             ///< Caminho no sistema de ficheiros.
    char keywords[64];         ///< Palavras-chave associadas.
    gboolean isFree;           ///< Indica se o slot está marcado como livre (apagado).
} MetaInformation;

/**
 * @brief Cria uma nova instância de MetaInformation com valores padrão.
 *
 * O campo `isFree` será inicializado como FALSE.
 * @return Uma instância de MetaInformation inicializada.
 */
MetaInformation metaInformation_new();

/**
 * @brief Obtém o ID do documento.
 */
int metaInformation_get_IdDocument(MetaInformation *info);

/**
 * @brief Define o ID do documento.
 */
void metaInformation_set_IdDocument(MetaInformation *info, int id);

/**
 * @brief Obtém o título do documento.
 */
char *metaInformation_get_DocumentTitle(MetaInformation *info);

/**
 * @brief Define o título do documento.
 */
void metaInformation_set_DocumentTitle(MetaInformation *info, const char *title);

/**
 * @brief Obtém o autor do documento.
 */
char *metaInformation_get_Author(MetaInformation *info);

/**
 * @brief Define o(s) autor(es) do documento.
 */
void metaInformation_set_Author(MetaInformation *info, const char *author);

/**
 * @brief Obtém o ano de publicação do documento.
 */
int metaInformation_get_Year(MetaInformation *info);

/**
 * @brief Define o ano de publicação do documento.
 */
void metaInformation_set_Year(MetaInformation *info, int year);

/**
 * @brief Obtém o caminho do documento.
 */
char *metaInformation_get_Path(MetaInformation *info);

/**
 * @brief Define o caminho do documento.
 */
void metaInformation_set_Path(MetaInformation *info, const char *path);

/**
 * @brief Obtém as palavras-chave associadas ao documento.
 */
char *metaInformation_get_Keywords(MetaInformation *info);

/**
 * @brief Define as palavras-chave associadas ao documento.
 */
void metaInformation_set_Keywords(MetaInformation *info, const char *keywords);

/**
 * @brief Marca a entrada como apagada (livre).
 */
void metaInformation_mark_as_deleted(MetaInformation *info);

/**
 * @brief Verifica se a entrada está marcada como apagada.
 *
 * @return TRUE se estiver apagada, FALSE caso contrário.
 */
gboolean metaInformation_is_deleted(MetaInformation *info);

/**
 * @brief Obtém o tamanho em bytes da estrutura MetaInformation.
 *
 * Útil para operações de escrita/leitura em ficheiros binários ou partilha entre processos.
 */
size_t metaInformation_size();

/**
 * @brief Liberta memória alocada dinamicamente para uma estrutura MetaInformation.
 *
 * **Importante**: essa função só deve ser usada se a estrutura foi alocada com `g_malloc` ou equivalente.
 */
void metaInformation_free(MetaInformation *info);

#endif // META_INFORMATION_H
