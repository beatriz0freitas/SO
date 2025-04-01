#ifndef UTILS
#define UTILS

#define SERVER "fifo_server"
#define CLIENT "fifo_client"

typedef struct client{
    char* documentTitle;
    char* author;         //separados por ;
    int year;
    char* keywords;
} Client;


#endif 