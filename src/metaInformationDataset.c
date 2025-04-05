#include "metaInformationDataset.h"
#include <glib.h>

struct MetaInformationDataset{
    GHashTable *MetaInformation;
    int nextindex;
};

// NOTA: Falta adicionar a implementação de escrever, apagar e consultar também pra o ficheiro binário em cada uma das funções

MetaInformationDataset *metaInformationDataset_new() {
    MetaInformationDataset * dataset = g_new0(MetaInformationDataset, 1);
    dataset->MetaInformation = g_hash_table_new(g_direct_hash, g_direct_equal);
    dataset->nextindex = 1;
    return dataset;
}

int metaInformationDataset_add(MetaInformationDataset *dataset, Information *metaInfo) {
    int key = dataset->nextindex;
    dataset->nextindex++;
  
    g_hash_table_insert(dataset->MetaInformation, (gpointer)key, (gpointer)metaInfo);
    return key;
}

gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key){
    Information *metaInfo = g_hash_table_lookup(dataset->MetaInformation, (gpointer)key);
    if (metaInfo != NULL) {
        g_hash_table_remove(dataset->MetaInformation, (gpointer)key);
        metaInformation_free(metaInfo);
        return TRUE;
    }

    return FALSE;
}

Information *metaInformationDataset_consult(MetaInformationDataset *dataset, int key) {
    Information *metaInfo = g_hash_table_lookup(dataset->MetaInformation, (gpointer)key);
    if (metaInfo != NULL) {
        return metaInfo;
    } 

    return NULL;
}

