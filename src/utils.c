#include "utils.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <stdio.h>

ssize_t bufferedRead(int fd, void *buf, size_t count) {
    ssize_t bytesRead = 0;
    ssize_t testRead;

    while (bytesRead < count) {
        testRead = read(fd, buf + bytesRead, count - bytesRead);
        if (testRead < 0) {
            perror("Erro ao ler do ficheiro");
            return -1;
        }
        if (testRead == 0) {
            break;  // EOF (Fim do ficheiro)
        }
        bytesRead += testRead;
    }
    return bytesRead;
}

ssize_t bufferedWrite (int fd, const void *buf, size_t count) {
    ssize_t bytesWritten = 0;
    ssize_t testWrite;

    while (bytesWritten < count) {
        testWrite = write(fd, buf + bytesWritten, count - bytesWritten);
        if (testWrite < 0) {
            perror("Erro ao escrever no ficheiro");
            return -1;
        }
        bytesWritten += testWrite;
    }
    return bytesWritten;
}



int open_with_lock(const char *path, int flags, mode_t mode, int lock_type) {
    int fd = open(path, flags, mode);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro com lock");
        return -1;
    }
    if (flock(fd, lock_type) != 0) {
        perror("Erro ao aplicar lock no ficheiro");
        close(fd);
        return -1;
    }
    return fd;
}

int close_with_unlock(int fd) {
    if (flock(fd, LOCK_UN) != 0) {
        perror("Erro ao remover lock");
        return -1;
    }
    return close(fd);
}





//TODO: Confirmar esta Mysystem

int mysystem(const char *command_line) {
    if (command_line == NULL) return -1;

    // Copiar a linha de comando porque strtok a vai modificar
    char *cmd_copy = strdup(command_line);
    if (!cmd_copy) return -1;

    // Contar argumentos
    int argc = 0;
    char *token = strtok(cmd_copy, " ");
    while (token) {
        argc++;
        token = strtok(NULL, " ");
    }

    // Criar array de strings para execvp
    char **args = malloc((argc + 1) * sizeof(char *));
    if (!args) {
        free(cmd_copy);
        return -1;
    }

    // Preencher o array de args
    strcpy(cmd_copy, command_line);
    int i = 0;
    token = strtok(cmd_copy, " ");
    while (token) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        free(args);
        free(cmd_copy);
        return -1;
    }

    if (pid == 0) {
        // Processo filho
        execvp(args[0], args);
        perror("execvp"); // Só chega aqui se exec falhar
        exit(EXIT_FAILURE);
    } else {
        // Processo pai
        int status;
        waitpid(pid, &status, 0);
        free(args);
        free(cmd_copy);
        return status;
    }
}