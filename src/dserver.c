#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "utils.h"
#include "command.h"
#include "metaInformation.h"

void handleMessage(MetaInformation *mensagem) {

}

void sendResponse(const char *fifo_cliente, const char *response) {
    int fd_client = open(fifo_cliente, O_WRONLY);
    if (fd_client == -1) {
        perror("Erro ao abrir FIFO do cliente para resposta");
        return;
    }

    if (bufferedWrite(fd_client, response, strlen(response)) == -1) {
        perror("Erro ao enviar resposta para cliente");
    }
    close(fd_client);
}

int main (int argc, char * argv[]){
    
    if (mkfifo("fifo/dserver", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Erro ao criar fifo/dserver");
            exit(1);
        }
    }
    
}


