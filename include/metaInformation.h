#ifndef META_INFORMATION_H
#define META_INFORMATION_H

typedef struct information{
    int idDocument;         //TODO: temos de definir como vamos catalogar
    char documentTitle[200];
    char author[200];         //separados por ;
    int year;
    char path[64];
    char keywords[64];
} Information;

#endif