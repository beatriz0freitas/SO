#ifndef METAINFORMATIONDATASET_C
#define METAINFORMATIONDATASET_C

#include "metaInformation.h"
#include <glib.h>
#include <glib-2.0/glib/gtypes.h>


typedef struct MetaInformationDataset{
    GHashTable *MetaInformation; // hastable em que a key é o id do documento e o value é a posição em que está guardado no ficheiro binário
    GQueue *MetaInformationQueue; // Queue para guardar posições livres
    int nextindex;
} MetaInformationDataset;


MetaInformationDataset *metaInformationDataset_new();
void metaInformationDataset_store(MetaInformationDataset *dataset);
void metaInformationDataset_load(MetaInformationDataset *dataset);

int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo);
gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key);
MetaInformation *metaInformationDataset_consult(MetaInformationDataset *dataset, int key);
int metaInformationDataset_count_keyword_lines(MetaInformationDataset *dataset, int id, const char *keyword);
char *metaInformationDataset_search_documents(MetaInformationDataset *dataset, const char *keyword);
void metaInformationDataset_free(MetaInformationDataset *dataset) ;
#endif