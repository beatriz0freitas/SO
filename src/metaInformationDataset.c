#include "metaInformationDataset.h"
#include <glib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define FILENAME "information.bin" // nome do ficheiro binário onde vamos guardar

struct MetaInformationDataset{
    GHashTable *MetaInformation; // hastable em que a key é o id do documento e o value é a posição em que está guardado no ficheiro binário
    GQueue *MetaInformationQueue; // Queue para guardar posições livres
    int nextindex;
};


MetaInformationDataset *metaInformationDataset_new() {
    MetaInformationDataset * dataset = g_new0(MetaInformationDataset, 1);
    dataset->MetaInformation = g_hash_table_new(g_direct_hash, g_direct_equal);
    dataset->MetaInformationQueue = g_queue_new();
    dataset->nextindex = 1;
    return dataset;
}



int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo) {

    int fd = open(FILENAME, O_CREAT | O_APPEND | O_WRONLY, 0666);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro");
        return -1;
    }

    off_t posicao_bytes = lseek(fd, 0, SEEK_END); // total de bytes do ficheiro
    int posicao_registo;


    if (g_queue_is_empty(dataset->MetaInformationQueue)) {
        posicao_registo = (posicao_bytes / metaInformation_size()); // posição em que será inserido o registo
    } else {
        // Se a queue não estiver vazia, retiramos uma posição livre
        posicao_registo = GPOINTER_TO_INT(g_queue_pop_head(dataset->MetaInformationQueue));
    }

    int key = dataset->nextindex;
    metaInformation_set_IdDocument(metaInfo, key); //atualiza o id do documento

    // Escrever a struct no ficheiro
    lseek(fd, posicao_registo * metaInformation_size(), SEEK_SET); // Saltar para a posição certa no ficheiro
    if (write(fd, metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro a escrever no ficheiro");
        close(fd);
        return -1;
    }

    close(fd);

    // Inserir na hashtable: key = idDocument, value = posição (em número de structs)
    g_hash_table_insert(dataset->MetaInformation, GINT_TO_POINTER(key), GINT_TO_POINTER(posicao_registo));
    dataset->nextindex++;
    return metaInformation_get_IdDocument(metaInfo); // dá return ao id do documento
}


gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key){

    int posicao = -1;
    posicao = GPOINTER_TO_INT(g_hash_table_lookup(dataset->MetaInformation, GINT_TO_POINTER(key)));

    if (posicao == -1) {
        return FALSE; // Não existe
    }

    int fd = open(FILENAME, O_RDWR);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro");
        return FALSE;
    }
    
    lseek(fd, posicao * metaInformation_size(), SEEK_SET);
    
    MetaInformation *metaInfo = metaInformation_new(); // Alocar memória para receber a struct
    if (read(fd, metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro ao ler do ficheiro");
        g_free(metaInfo);
        close(fd);
        return FALSE;
    }
    
    metaInformation_mark_as_deleted(metaInfo);
    
    lseek(fd, posicao * metaInformation_size(), SEEK_SET);

    if (write(fd, metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro ao escrever no ficheiro");
        g_free(metaInfo);
        close(fd);
        return FALSE;
    }
    
    close(fd);
    
    // Atualiza dataset
    g_queue_push_tail(dataset->MetaInformationQueue, GINT_TO_POINTER(posicao));

    g_hash_table_remove(dataset->MetaInformation, GINT_TO_POINTER(key));
    g_free(metaInfo);
    
        return TRUE;
    }

MetaInformation *metaInformationDataset_consult(MetaInformationDataset *dataset, int key) {

    int *value = g_hash_table_lookup(dataset->MetaInformation, GINT_TO_POINTER(key));
    if (value == NULL) {
        return NULL; // Não existe
    }

    int fd = open(FILENAME, O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro");
        return NULL;
    }

    int posicao_registo = *(int *)value;


    lseek(fd, posicao_registo * metaInformation_size(), SEEK_SET); // Saltar para a posição certa no ficheiro
    MetaInformation *metaInfo = g_malloc(metaInformation_size()); // Alocar memória para receber a struct

    if (read(fd, metaInfo,  metaInformation_size()) !=  metaInformation_size()) {
        perror("Erro a ler do ficheiro");
        g_free(metaInfo);
        close(fd);
        return NULL;
    }

    close(fd);

    if (metaInformation_is_deleted(metaInfo)) {
        g_free(metaInfo);
        return NULL; // O documento foi marcado como eliminado
    }

    return metaInfo;
}
