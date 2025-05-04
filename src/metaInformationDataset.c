#include "metaInformationDataset.h"
#include <glib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"

#define FILENAME "information.bin" // nome do ficheiro binário onde vamos guardar


MetaInformationDataset *metaInformationDataset_new() {
    MetaInformationDataset * dataset = g_new0(MetaInformationDataset, 1);
    dataset->MetaInformation = g_hash_table_new(g_direct_hash, g_direct_equal);
    dataset->MetaInformationQueue = g_queue_new();
    dataset->nextindex = 1;
    return dataset;
}

void metaInformationDataset_store(MetaInformationDataset *dataset) {
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Erro ao abrir ficheiro para escrita");
        return;
    }

    bufferedWrite(fd, &dataset->nextindex, sizeof(int));

    // Guarda o tamanho da queue
    guint freeCount = g_queue_get_length(dataset->MetaInformationQueue);
    bufferedWrite(fd, &freeCount, sizeof(guint));

    // Guarda os elementos da queue (inteiros)
    for (GList *l = dataset->MetaInformationQueue->head; l != NULL; l = l->next) {
        int pos = GPOINTER_TO_INT(l->data);
        bufferedWrite(fd, &pos, sizeof(int));
    }

    close(fd);
}


void metaInformationDataset_load(MetaInformationDataset *dataset) {
    int fd = open(FILENAME, O_RDONLY);
    if (fd < 0) {
        perror("Ficheiro de meta-informação não encontrado, a começar vazio");
        return;
    }

    bufferedRead(fd, &dataset->nextindex, sizeof(int));

    guint freeCount;
    bufferedRead(fd, &freeCount, sizeof(guint));

    for (guint i = 0; i < freeCount; i++) {
        int pos;
        bufferedRead(fd, &pos, sizeof(int));
        g_queue_push_tail(dataset->MetaInformationQueue, GINT_TO_POINTER(pos));
    }

    close(fd);
}

       
// Atribui o ID reutilizado se disponível, senão usa o nextindex.
int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo) {
    int fd = open(FILENAME, O_CREAT | O_RDWR, 0666); // O_APPEND removido para controlo manual do offset
    if (fd == -1) {
        perror("Erro ao abrir ficheiro");
        return -1;
    }

    int id, posicao_registo;

    if (g_queue_is_empty(dataset->MetaInformationQueue)) {
        // Novo ID sequencial
        id = dataset->nextindex++;
        off_t posicao_bytes = lseek(fd, 0, SEEK_END);
        posicao_registo = posicao_bytes / metaInformation_size(); // última posição disponível
    } else {
        // Reutiliza posição e ID antigos
        id = GPOINTER_TO_INT(g_queue_pop_head(dataset->MetaInformationQueue));
        posicao_registo = id; // Assumimos que ID e posição no ficheiro coincidem
    }

    metaInformation_set_IdDocument(metaInfo, id);

    lseek(fd, posicao_registo * metaInformation_size(), SEEK_SET);
    if (bufferedWrite(fd, metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro a escrever no ficheiro");
        close(fd);
        return -1;
    }

    close(fd);

    g_hash_table_insert(dataset->MetaInformation, GINT_TO_POINTER(id), GINT_TO_POINTER(posicao_registo));

    return id;
}


gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key) {
    int posicao = GPOINTER_TO_INT(g_hash_table_lookup(dataset->MetaInformation, GINT_TO_POINTER(key)));

    if (posicao == 0 && !g_hash_table_contains(dataset->MetaInformation, GINT_TO_POINTER(key))) {
        return FALSE; // Não existe
    }

    int fd = open(FILENAME, O_RDWR);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro");
        return FALSE;
    }

    lseek(fd, posicao * metaInformation_size(), SEEK_SET);
    
    MetaInformation metaInfo;
    if (bufferedRead(fd, &metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro ao ler do ficheiro");
        close(fd);
        return FALSE;
    }

    metaInformation_mark_as_deleted(&metaInfo);

    lseek(fd, posicao * metaInformation_size(), SEEK_SET);
    if (bufferedWrite(fd, &metaInfo, metaInformation_size()) != metaInformation_size()) {
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
    lseek(fd, posicao_registo * metaInformation_size(), SEEK_SET);

    MetaInformation *metaInfo = g_malloc(metaInformation_size());
    if (bufferedRead(fd, metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro a ler do ficheiro");
        g_free(metaInfo);
        close(fd);
        return NULL;
    }

    close(fd);

    if (metaInformation_is_deleted(metaInfo)) {
        g_free(metaInfo);
        return NULL;
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