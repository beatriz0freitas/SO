#ifndef METAINFORMATIONDATASET_C
#define METAINFORMATIONDATASET_C

#include "metaInformation.h"
#include <glib/gtypes.h>

typedef struct MetaInformationDataset MetaInformationDataset;
MetaInformationDataset *MetaInformationDataset_new();


int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo);
gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key);
MetaInformation *MetaInformationDataset_consult(MetaInformationDataset *dataset, int key);

#endif