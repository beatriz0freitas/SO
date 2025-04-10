#ifndef META_INFORMATION_H
#define META_INFORMATION_H


#include <stdlib.h>

typedef struct MetaInformation MetaInformation;


MetaInformation *metaInformation_new();

int metaInformation_get_IdDocument(MetaInformation *info);
void metaInformation_set_IdDocument(MetaInformation *info, int id);
char *metaInformation_get_DocumentTitle(MetaInformation *info);
void metaInformation_set_DocumentTitle(MetaInformation *info, const char *title);
char *metaInformation_get_Author(MetaInformation *info);
void metaInformation_set_Author(MetaInformation *info, const char *author);
int metaInformation_get_Year(MetaInformation *info);
void metaInformation_set_Year(MetaInformation *info, int year);
char *metaInformation_get_Path(MetaInformation *info);
void metaInformation_set_Path(MetaInformation *info, const char *path);
char *metaInformation_get_Keywords(MetaInformation *info);
void metaInformation_set_Keywords(MetaInformation *info, const char *keywords);


size_t metaInformation_size();

void metaInformation_free(MetaInformation *info);


#endif