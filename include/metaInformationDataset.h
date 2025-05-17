#ifndef METAINFORMATIONDATASET_C
#define METAINFORMATIONDATASET_C

#include "metaInformation.h"
#include <glib.h>
#include <glib-2.0/glib/gtypes.h>
#include "cache.h"

#define MAX_PATH 256
#define CODIGOJAINDEXADO 20000

typedef struct MetaInformationDataset{
    GQueue *MetaInformationQueue; // Queue para guardar posições livres
    int nextindex;
    char folder[MAX_PATH];        // caminho da pasta de documentos
    char filename[MAX_PATH];
    Cache *cache; // Cache for MetaInformation
} MetaInformationDataset;


MetaInformationDataset *metaInformationDataset_new(const char *document_folder, int cache_size);

int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo);
gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key);
MetaInformation *metaInformationDataset_consult(MetaInformationDataset *dataset, int key);

int metaInformationDataset_count_keyword_lines(MetaInformationDataset *dataset, int id, const char *keyword);
char *metaInformationDataset_search_documents(MetaInformationDataset *dataset, const char *keyword);
char *metaInformationDataset_search_documents_sequential(MetaInformationDataset *dataset, const char *keyword);
char *metaInformationDataset_search_documents_parallel(MetaInformationDataset *dataset, const char *keyword, int max_procs);
void metaInformationDataset_free(MetaInformationDataset *dataset);

#endif