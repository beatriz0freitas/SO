#ifndef META_INFORMATION_H
#define META_INFORMATION_H

typedef struct Information Information;

Information *information_new();

int getIdDocument(Information *info);
void setIdDocument(Information *info, int id);
char *getDocumentTitle(Information *info);
void setDocumentTitle(Information *info, const char *title);
char *getAuthor(Information *info);
void setAuthor(Information *info, const char *author);
int getYear(Information *info);
void setYear(Information *info, int year);
char *getPath(Information *info);
void setPath(Information *info, const char *path);
char *getKeywords(Information *info);
void setKeywords(Information *info, const char *keywords);

void metaInformation_free(Information *info);

#endif