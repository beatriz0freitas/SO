#ifndef UTILS
#define UTILS

#define SERVER "fifo_server"
#define CLIENT "fifo_client"

typedef struct information{
    //char fifo_client    -  nome do FIFO de resposa do cliente
    int idDocument;         //TODO: temos de definir como vamos catalogar
    char documentTitle[200];
    char author[200];         //separados por ;
    int year;
    char path[64];
    char keywords[64];
} Information;


#endif 