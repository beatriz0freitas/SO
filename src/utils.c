#include "utils.h"
#include <unistd.h>

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

