#ifndef COMMAND
#define COMMAND

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_ARGS 7          /**< Número máximo de argumentos permitidos. */
#define MAX_ARG_SIZE 256    /**< Tamanho máximo de cada argumento. */

/**
 * @enum CommandFlag
 * @brief Enumeração dos tipos de comandos suportados.
 */
typedef enum {
    CMD_INVALID = -1,          /**< Comando inválido ou não reconhecido. */
    CMD_ADD = 1,               /**< Comando de indexação (-a). */
    CMD_CONSULT = 2,           /**< Comando de consulta (-c). */
    CMD_DELETE = 3,            /**< Comando de remoção (-d). */
    CMD_LIST = 4,              /**< Comando de listagem de linhas (-l). */
    CMD_SEARCH = 5,            /**< Comando de busca (-s) com 1 argumento. */
    CMD_SEARCH_PROCESSOS = 6,  /**< Comando de busca com processos (-s) com mais de 1 argumento. */
    CMD_SHUTDOWN = 7           /**< Comando para encerrar o sistema (-f). */
} CommandFlag;

/**
 * @struct Command
 * @brief Estrutura que representa um comando completo com seus argumentos.
 */
typedef struct Command {
    CommandFlag flag;                        /**< Tipo do comando. */
    char args[MAX_ARGS][MAX_ARG_SIZE];       /**< Lista de argumentos do comando. */
    int num_args;                            /**< Número real de argumentos armazenados. */
} Command;

/**
 * @brief Converte uma string de flag em um valor do tipo CommandFlag.
 *
 * @param flagStr String que representa o comando (ex: "-a").
 * @param numArgs Número de argumentos fornecidos para o comando.
 * @return O valor correspondente do enum CommandFlag ou CMD_INVALID se não reconhecido.
 */
CommandFlag command_parse_flag(char *flagStr, int numArgs);

/**
 * @brief Retorna uma string legível que representa o nome do comando.
 *
 * @param flag Valor do tipo CommandFlag.
 * @return Nome do comando como string.
 */
const char *commandFlag_to_string(CommandFlag flag);

/**
 * @brief Constrói uma estrutura Command a partir de uma linha de entrada.
 *
 * @param numArgs Número total de argumentos (incluindo o nome do programa).
 * @param linha Vetor de strings contendo os argumentos da linha de comando.
 * @return Estrutura Command preenchida.
 */
Command command_constroi_de_linha(int numArgs, char *linha[]);

/**
 * @brief Retorna o número de argumentos associados ao comando.
 *
 * @param cmd Ponteiro para a estrutura Command.
 * @return Número de argumentos.
 */
int command_get_num_args(Command *cmd);

/**
 * @brief Retorna o tipo (flag) do comando.
 *
 * @param cmd Ponteiro para a estrutura Command.
 * @return Valor do enum CommandFlag correspondente.
 */
CommandFlag command_get_flag(Command *cmd);

/**
 * @brief Retorna o argumento do comando na posição especificada.
 *
 * @param cmd Ponteiro para a estrutura Command.
 * @param indice Índice do argumento desejado (0 baseado).
 * @return Ponteiro para o argumento ou NULL se o índice for inválido.
 */
char *command_get_arg_por_indice(Command *cmd, int indice);

#endif // COMMAND
