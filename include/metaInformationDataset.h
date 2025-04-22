#ifndef METAINFORMATIONDATASET_C
#define METAINFORMATIONDATASET_C

#include "metaInformation.h"
#include <glib.h>

typedef struct MetaInformationDataset MetaInformationDataset;
MetaInformationDataset *metaInformationDataset_new();


int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo);
gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key);
MetaInformation *metaInformationDataset_consult(MetaInformationDataset *dataset, int key);

void metaInformationDataset_free(MetaInformationDataset *dataset) ;
#endif