#include "metaInformationDataset.h"
#include <glib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"

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
    
    MetaInformation metaInfo = metaInformation_new(); // Alocar memória para receber a struct
    if (read(fd, &metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro ao ler do ficheiro");
        close(fd);
        return FALSE;
    }
    
    metaInformation_mark_as_deleted(&metaInfo);
    
    lseek(fd, posicao * metaInformation_size(), SEEK_SET);

    if (write(fd, &metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro ao escrever no ficheiro");
        close(fd);
        return FALSE;
    }
    
    close(fd);
    
    // Atualiza dataset
    g_queue_push_tail(dataset->MetaInformationQueue, GINT_TO_POINTER(posicao));

    g_hash_table_remove(dataset->MetaInformation, GINT_TO_POINTER(key));
    
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

    int posicao_registo = GPOINTER_TO_INT(value);


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

int metaInformationDataset_count_keyword_lines(MetaInformationDataset *dataset, int id, const char *keyword) {
    MetaInformation *metaInfo = metaInformationDataset_consult(dataset, id);

    if (!metaInfo || metaInformation_is_deleted(metaInfo)) {
        return -1;
    }

    const char *path = metaInformation_get_Path(metaInfo);
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("Erro ao criar pipe");
        metaInformation_free(metaInfo);
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Erro ao criar processo filho");
        close(pipefd[0]);
        close(pipefd[1]);
        metaInformation_free(metaInfo);
        return -1;
    }

    if (pid == 0) {  // Processo filho
       
        close(pipefd[0]);               // Fecha leitura
        dup2(pipefd[1], STDOUT_FILENO); // Redireciona stdout para o pipe
        close(pipefd[1]);

        
        const char *comando = g_strdup_printf("grep -c %s %s", keyword, path); // Executar grep -c "keyword" path
        mysystem(comando);
        perror("Erro ao executar grep");
        exit(1); // Se exec falhar
    }

    // Processo pai
    close(pipefd[1]); // Fecha escrita

    char buffer[64];
    ssize_t nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1);
    if (nbytes < 0) {
        perror("Erro ao ler do pipe");
        close(pipefd[0]);
        metaInformation_free(metaInfo);
        return -1;
    }
    buffer[nbytes] = '\0';

    close(pipefd[0]);
    metaInformation_free(metaInfo);

    int count = atoi(buffer); // Converte a string recebida em int
    return count;
}

char *metaInformationDataset_search_documents(MetaInformationDataset *dataset, const char *keyword) {
    GString *resultado = g_string_new("[");
    gboolean primeiro = TRUE;

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, dataset->MetaInformation);
    printf("[DEBUG] Nº de elementos no dataset: %u\n", g_hash_table_size(dataset->MetaInformation));

    while (g_hash_table_iter_next(&iter, &key, &value)) {
        int id = GPOINTER_TO_INT(key);
        MetaInformation *meta = metaInformationDataset_consult(dataset, id);
        if (!meta) continue;

        const char *path = metaInformation_get_Path(meta);
        printf("[DEBUG] A procurar no ficheiro: %s (ID %d)\n", path, id);

        FILE *fp = fopen(path, "r");
        if (!fp) {
            perror("Erro ao abrir ficheiro");
            metaInformation_free(meta);
            continue;
        }

        char *line = NULL;
        size_t len = 0;
        gboolean found = FALSE;

        while (getline(&line, &len, fp) != -1) {
            if (strstr(line, keyword)) {
                found = TRUE;
                break;
            }
        }

        free(line);
        fclose(fp);
        metaInformation_free(meta);

        if (found) {
            printf("[DEBUG] Palavra '%s' encontrada no ficheiro ID %d\n", keyword, id);
            if (!primeiro) {
                g_string_append(resultado, ", ");
            }
            g_string_append_printf(resultado, "%d", id);
            primeiro = FALSE;
        } else {
            printf("[DEBUG] Palavra '%s' **NÃO** encontrada no ficheiro ID %d\n", keyword, id);
        }
    }

    g_string_append(resultado, "]");
    return g_string_free(resultado, FALSE);
}


void metaInformationDataset_free(MetaInformationDataset *dataset) {
    if (dataset) {
        g_hash_table_destroy(dataset->MetaInformation);
        g_free(dataset);
    }
}