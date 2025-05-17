#ifndef UTILS
#define UTILS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/**
 * @file utils.h
 * @brief Funções utilitárias para operações de ficheiros, leitura/escrita com buffer,
 *        manipulação de locks e execução de comandos externos.
 *
 * Este módulo fornece funções para abrir ficheiros com locks (flock), 
 * realizar leituras e escritas completas (buffered), e executar comandos do sistema
 * de forma segura e controlada.
 */

/** 
 * @def FIFO_CLIENT_TO_SERVER
 * @brief Nome do FIFO para comunicação do cliente para o servidor.
 */

/**
 * @def FIFO_TEMPLATE_SERVER_TO_CLIENT
 * @brief Template do nome do FIFO para comunicação do servidor para o cliente.
 *        O %d deve ser substituído pelo PID do cliente.
 */


/**
 * @brief Abre um ficheiro com o tipo de lock especificado.
 * 
 * @param path Caminho do ficheiro a abrir.
 * @param flags Flags para a função open (ex: O_RDONLY, O_RDWR, O_CREAT, etc).
 * @param mode Permissões para o ficheiro, caso seja criado (ex: 0666).
 * @param lock_type Tipo de lock a aplicar (ex: LOCK_EX, LOCK_SH).
 * 
 * @return O descritor do ficheiro aberto com sucesso, ou -1 em caso de erro.
 *         Em caso de erro, a mensagem é impressa em stderr.
 * 
 * @note Utiliza flock para aplicar o lock no ficheiro.
 */
int open_with_lock(const char *path, int flags, mode_t mode, int lock_type);

/**
 * @brief Fecha um ficheiro e remove o lock aplicado.
 * 
 * @param fd Descritor do ficheiro a fechar.
 * 
 * @return 0 em sucesso, -1 em caso de erro (mensagem em stderr).
 */
int close_with_unlock(int fd);

/**
 * @brief Realiza uma leitura completa, garantindo que todos os bytes solicitados são lidos.
 * 
 * @param fd Descritor do ficheiro.
 * @param buf Buffer onde os dados lidos serão armazenados.
 * @param count Número de bytes a ler.
 * 
 * @return Número total de bytes lidos (pode ser menor que count se for EOF),
 *         ou -1 em caso de erro.
 */
ssize_t bufferedRead(int fd, void *buf, size_t count);

/**
 * @brief Realiza uma escrita completa, garantindo que todos os bytes solicitados são escritos.
 * 
 * @param fd Descritor do ficheiro.
 * @param buf Buffer com os dados a escrever.
 * @param count Número de bytes a escrever.
 * 
 * @return Número total de bytes escritos, ou -1 em caso de erro.
 */
ssize_t bufferedWrite (int fd, const void *buf, size_t count);

/**
 * @brief Executa um comando do sistema, similar à função system(), mas usando fork e execvp.
 * 
 * @param command_line Linha de comando a executar (ex: "ls -l /tmp").
 * 
 * @return O status do processo filho (retornado por waitpid),
 *         ou -1 em caso de erro.
 * 
 * @note Esta função divide a linha de comando em argumentos e executa com execvp.
 *       O processo pai espera o filho terminar e retorna o seu status.
 */
int mysystem(const char *command_line);

#endif
