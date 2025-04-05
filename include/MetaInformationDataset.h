#ifndef METAINFORMATIONDATASET_C
#define METAINFORMATIONDATASET_C

#include "MetaInformation.h"
#include <glib.h>

typedef struct MetaInformationDataset MetaInformationDataset;
MetaInformationDataset *MetaInformationDataset_new();


int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo);
void metaInformationDataset_remove(MetaInformationDataset *dataset, int key);
MetaInformation *MetaInformationDataset_consult(MetaInformationDataset *dataset, int key);

#endif