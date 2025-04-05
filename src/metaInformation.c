#include "metaInformation.h"
#include <string.h>
#include <glib.h>

struct Information{
    int idDocument;         //TODO: temos de definir como vamos catalogar
    char documentTitle[200];
    char author[200];         //separados por ;
    int year;
    char path[64];
    char keywords[64];
};

Information *metaInformation_new(){
    Information *information = g_new0(Information, 1);
    return information;
}

int metaInformation_get_IdDocument(Information *info) {
    return info->idDocument;
}

void metaInformation_set_IdDocument(Information *info, int id) {
    info->idDocument = id;
}

char *metaInformation_get_DocumentTitle(Information *info) {
    return info->documentTitle;
}

void metaInformation_set_DocumentTitle(Information *info, const char *title) {
    strncpy(info->documentTitle, title, sizeof(info->documentTitle));
    info->documentTitle[sizeof(info->documentTitle) - 1] = '\0'; // Ensure null termination
}

char *metaInformation_get_Author(Information *info) {
    return info->author;
}

void metaInformation_set_Author(Information *info, const char *author) {
    strncpy(info->author, author, sizeof(info->author));
    info->author[sizeof(info->author) - 1] = '\0'; 
}

int metaInformation_get_Year(Information *info) {
    return info->year;
}

void metaInformation_set_Year(Information *info, int year) {
    info->year = year;
}

char *metaInformation_get_Path(Information *info) {
    return info->path;
}

void metaInformation_set_Path(Information *info, const char *path) {
    strncpy(info->path, path, sizeof(info->path));
    info->path[sizeof(info->path) - 1] = '\0'; 
}

char *metaInformation_get_Keywords(Information *info) {
    return info->keywords;
}

void metaInformation_set_Keywords(Information *info, const char *keywords) {
    strncpy(info->keywords, keywords, sizeof(info->keywords));
    info->keywords[sizeof(info->keywords) - 1] = '\0';
}


void metaInformation_free(Information *info) {
    g_free(info);
}
