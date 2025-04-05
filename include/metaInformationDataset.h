#ifndef METAINFORMATIONDATASET_C
#define METAINFORMATIONDATASET_C

#include "metaInformation.h"

typedef struct MetaInformationDataset MetaInformationDataset;
MetaInformationDataset *MetaInformationDataset_new();


int metaInformationDataset_add(MetaInformationDataset *dataset, Information *metaInfo);
gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key);
Information *MetaInformationDataset_consult(MetaInformationDataset *dataset, int key);

#endif