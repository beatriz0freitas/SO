#ifndef METAINFORMATIONDATASET_C
#define METAINFORMATIONDATASET_C

#include "metaInformation.h"
#include <glib.h>
#include <glib-2.0/glib/gtypes.h>

typedef struct MetaInformationDataset MetaInformationDataset;
MetaInformationDataset *metaInformationDataset_new();


int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo);
gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key);
MetaInformation *metaInformationDataset_consult(MetaInformationDataset *dataset, int key);
int metaInformationDataset_count_keyword_lines(MetaInformationDataset *dataset, int id, const char *keyword);
char *metaInformationDataset_search_documents(MetaInformationDataset *dataset, const char *keyword);
void metaInformationDataset_free(MetaInformationDataset *dataset) ;
#endif