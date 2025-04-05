#include "metaInformation.h"
#include <string.h>

typedef struct Information{
    int idDocument;         //TODO: temos de definir como vamos catalogar
    char documentTitle[200];
    char author[200];         //separados por ;
    int year;
    char path[64];
    char keywords[64];
} Information;

int getIdDocument(Information *info) {
    return info->idDocument;
}

void setIdDocument(Information *info, int id) {
    info->idDocument = id;
}

char *getDocumentTitle(Information *info) {
    return info->documentTitle;
}

void setDocumentTitle(Information *info, const char *title) {
    strncpy(info->documentTitle, title, sizeof(info->documentTitle));
    info->documentTitle[sizeof(info->documentTitle) - 1] = '\0'; // Ensure null termination
}

char *getAuthor(Information *info) {
    return info->author;
}

void setAuthor(Information *info, const char *author) {
    strncpy(info->author, author, sizeof(info->author));
    info->author[sizeof(info->author) - 1] = '\0'; // Ensure null termination
}

int getYear(Information *info) {
    return info->year;
}

void setYear(Information *info, int year) {
    info->year = year;
}

char *getPath(Information *info) {
    return info->path;
}

void setPath(Information *info, const char *path) {
    strncpy(info->path, path, sizeof(info->path));
    info->path[sizeof(info->path) - 1] = '\0'; // Ensure null termination
}

char *getKeywords(Information *info) {
    return info->keywords;
}

void setKeywords(Information *info, const char *keywords) {
    strncpy(info->keywords, keywords, sizeof(info->keywords));
    info->keywords[sizeof(info->keywords) - 1] = '\0'; // Ensure null termination
}
