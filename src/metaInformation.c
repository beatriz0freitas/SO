#include "metaInformation.h"
#include <string.h>
#include <glib.h>

//caso altermos para char* na struct temos de fazer strdup()
MetaInformation *metaInformation_new(){
    MetaInformation *information = g_new0(MetaInformation, 1);
    return information;
}

int metaInformation_get_IdDocument(MetaInformation *info) {
    return info->idDocument;
}

void metaInformation_set_IdDocument(MetaInformation *info, int id) {
    info->idDocument = id;
}

char *metaInformation_get_DocumentTitle(MetaInformation *info) {
    return info->documentTitle;
}

void metaInformation_set_DocumentTitle(MetaInformation *info, const char *title) {
    memset(info->documentTitle, 0, sizeof(info->documentTitle));
    strncpy(info->documentTitle, title, sizeof(info->documentTitle)-1);
    info->documentTitle[sizeof(info->documentTitle) - 1] = '\0'; // Ensure null termination
}

char *metaInformation_get_Author(MetaInformation *info) {
    return info->author;
}

void metaInformation_set_Author(MetaInformation *info, const char *author) {
    memset(info->author, 0, sizeof(info->author));
    strncpy(info->author, author, sizeof(info->author)-1);
    info->author[sizeof(info->author) - 1] = '\0'; 
}

int metaInformation_get_Year(MetaInformation *info) {
    return info->year;
}

void metaInformation_set_Year(MetaInformation *info, int year) {
    info->year = year;
}

char *metaInformation_get_Path(MetaInformation *info) {
    return info->path;
}

void metaInformation_set_Path(MetaInformation *info, const char *path) {
    memset(info->path, 0, sizeof(info->path));
    strncpy(info->path, path, sizeof(info->path)-1);
    info->path[sizeof(info->path) - 1] = '\0'; 
}

char *metaInformation_get_Keywords(MetaInformation *info) {
    return info->keywords;
}

void metaInformation_set_Keywords(MetaInformation *info, const char *keywords) {
    memset(info->keywords, 0, sizeof(info->keywords));
    strncpy(info->keywords, keywords, sizeof(info->keywords)-1);
    info->keywords[sizeof(info->keywords) - 1] = '\0';
}

//caso altermos para char* na struct temos de fazer strdup()
void metaInformation_free(MetaInformation *info) {
    g_free(info);
}

size_t metaInformation_size() {
    return sizeof(struct MetaInformation);
}
