#ifndef METAINFORMATIONDATASET_C
#define METAINFORMATIONDATASET_C


typedef struct MetaInformationDataset MetaInformationDataset;
MetaInformationDataset *MetaInformationDataset_new();


int metaInformationDataset_add(MetaInformationDataset *dataset, Information *metaInfo);
void metaInformationDataset_remove(MetaInformationDataset *dataset, int key);
Information *MetaInformationDataset_consult(MetaInformationDataset *dataset, int key);

#endif