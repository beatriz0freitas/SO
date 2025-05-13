#include "metaInformationDataset.h"
#include <glib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include "utils.h"

#define FILENAME "information.bin" // nome do ficheiro binário onde vamos guardar


MetaInformationDataset *metaInformationDataset_new(const char *document_folder) {
    MetaInformationDataset * dataset = g_new0(MetaInformationDataset, 1);
    dataset->MetaInformation = g_hash_table_new(g_direct_hash, g_direct_equal);
    dataset->MetaInformationQueue = g_queue_new();
    dataset->nextindex = 1;
    strncpy(dataset->folder, document_folder, MAX_PATH-1);
    strncpy(dataset->filename,"information.bin", MAX_PATH - 1);
    dataset->folder[MAX_PATH-1] = '\0';
    return dataset;
}

/* SERA UTIL PAARA GUARDAR EM DISCO
void metaInformationDataset_store(MetaInformationDataset *dataset) {
    int fd = open(dataset->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
    int fd = open(dataset->filename, O_RDONLY);
    if (fd < 0) {
        perror("[DEBUG]: Ficheiro de meta-informação não encontrado, a começar vazio");
        return;
    }
    
    // Verifica o tamanho do ficheiro
    off_t filesize = lseek(fd, 0, SEEK_END);
    if (filesize < sizeof(int) + sizeof(guint)) {
        // Ficheiro demasiado pequeno para conter dados válidos
        printf("[DEBUG] Ficheiro existe mas está vazio ou incompleto.\n");
        close(fd);
        return;
    }

    lseek(fd, 0, SEEK_SET);  // Voltar ao início para ler

    if (bufferedRead(fd, &dataset->nextindex, sizeof(int)) != sizeof(int)) {
        perror("[DEBUG]: Erro ao ler nextindex");
        close(fd);
        return;
    }

    guint freeCount;
    if (bufferedRead(fd, &freeCount, sizeof(guint)) != sizeof(guint)) {
        perror("[DEBUG]: Erro ao ler freeCount");
        close(fd);
        return;
    }

    for (guint i = 0; i < freeCount; i++) {
        int pos;
        if (bufferedRead(fd, &pos, sizeof(int)) != sizeof(int)) {
            perror("[DEBUG]: Erro ao ler posição livre");
            break;
        }
        g_queue_push_tail(dataset->MetaInformationQueue, GINT_TO_POINTER(pos));
    }
    close(fd);
}
*/

void metaInformationDataset_buildfull_documentpath(char *dest, size_t size, const MetaInformationDataset *dataset, const MetaInformation *info) {
    snprintf(dest, size, "%s/%s", dataset->folder, info->path);
}


// Atribui o ID reutilizado se disponível, senão usa o nextindex.
int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo) {
   
    int fd = open(dataset->filename, O_CREAT | O_RDWR, 0666); // O_APPEND removido para controlo manual do offset
    if (fd == -1) {
        perror("[DEBUG]: Erro ao abrir ficheiro");
        return -1;
    }

    // Verificar se o ficheiro do documento existe fisicamente
    char caminhoCompleto[MAX_PATH];
    metaInformationDataset_buildfull_documentpath(caminhoCompleto, sizeof(caminhoCompleto), dataset, metaInfo);

    int doc_fd = open(caminhoCompleto, O_RDONLY);
    if (doc_fd == -1) {
        perror("[DEBUG] Ficheiro do documento não encontrado");
        close(fd);
        return -1;
    }
    close(doc_fd);

    int id, posicao_registo;

    if (g_queue_is_empty(dataset->MetaInformationQueue)) {
        // Novo ID sequencial
        id = dataset->nextindex++;
        off_t posicao_bytes = lseek(fd, 0, SEEK_END) / metaInformation_size(); // última posição disponível
        posicao_registo = posicao_bytes / metaInformation_size(); // última posição disponível
    } else {
        // Reutiliza posição e ID antigos
        id = GPOINTER_TO_INT(g_queue_pop_head(dataset->MetaInformationQueue));
        posicao_registo = id; // Assumimos que ID e posição no ficheiro coincidem
    }

    metaInformation_set_IdDocument(metaInfo, id);

    lseek(fd, posicao_registo * metaInformation_size(), SEEK_SET);
    if (bufferedWrite(fd, metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("[DEBUG]: Erro a escrever no ficheiro");
        close(fd);
        return -1;
    }

    close(fd);

    // Copiar para memória independente antes de inserir na hash table
    MetaInformation *copia = g_new(MetaInformation, 1);
    memcpy(copia, metaInfo, sizeof(MetaInformation));
    g_hash_table_insert(dataset->MetaInformation, GINT_TO_POINTER(id), copia);

    //metaInformationDataset_store(dataset);

    return id;
}


gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key) {
    int posicao = GPOINTER_TO_INT(g_hash_table_lookup(dataset->MetaInformation, GINT_TO_POINTER(key)));

    if (posicao == 0 && !g_hash_table_contains(dataset->MetaInformation, GINT_TO_POINTER(key))) {
        return FALSE; // Não existe
    }

    int fd = open(dataset->filename, O_RDWR);
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

    //metaInformationDataset_store(dataset);

    return TRUE;
}

MetaInformation *metaInformationDataset_consult(MetaInformationDataset *dataset, int key) {
    MetaInformation *original = g_hash_table_lookup(dataset->MetaInformation, GINT_TO_POINTER(key));
    if (original == NULL || metaInformation_is_deleted(original)) {
        return NULL;
    }

    MetaInformation *copy = g_new(MetaInformation, 1);
    memcpy(copy, original, sizeof(MetaInformation));
    return copy;
}


int metaInformationDataset_count_keyword_lines(MetaInformationDataset *dataset, int id, const char *keyword) {
    MetaInformation *metaInfo = metaInformationDataset_consult(dataset, id);
    if (!metaInfo || metaInformation_is_deleted(metaInfo)) {
        return -1;
    }
    
    char fullpath[MAX_PATH];
    metaInformationDataset_buildfull_documentpath(fullpath, sizeof(fullpath), dataset, metaInfo);
    
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

        char *escaped_keyword = g_shell_quote(keyword);
        char *escaped_path = g_shell_quote(fullpath);
        char *comando = g_strdup_printf("grep -c %s %s", escaped_keyword, escaped_path);
        
        execl("/bin/sh", "sh", "-c", comando, (char *)NULL);

        perror("Erro ao executar grep");
        exit(1); // Se exec falhar
    }

    // Processo pai
    close(pipefd[1]); // Fecha escrita

    char buffer[64];
    ssize_t nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1);
    if (nbytes <= 0) {
        if (nbytes < 0)
            perror("Erro a ler do pipe");
        else
            fprintf(stderr, "Aviso: Nenhum dado lido do pipe.\n");
        close(pipefd[0]);
        metaInformation_free(metaInfo);
        return -1;
    }
    buffer[nbytes] = '\0';
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);
    metaInformation_free(metaInfo);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return -1;
    }

    int count = atoi(buffer); // Converte a string recebida em int
    return count;
}

char *metaInformationDataset_search_documents(MetaInformationDataset *dataset, const char *keyword) {
    GString *resultado = g_string_new("[");

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, dataset->MetaInformation);

    while (g_hash_table_iter_next(&iter, &key, &value)) {
        if(value == NULL) continue;

        MetaInformation *meta = (MetaInformation *)value;

        char fullpath[MAX_PATH];
        metaInformationDataset_buildfull_documentpath(fullpath, sizeof(fullpath), dataset, meta);
        //printf("[DEBUG]: A procurar em: %s\n", fullpath);

        // cria pipe
        int pipefd[2];
        if (pipe(pipefd) < 0) 
            continue;

        pid_t pid = fork();
        if (pid == 0) {
            // filho: fecha leitura, dup2 escrita para STDOUT, exec grep -l
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
            execlp("grep", "grep", "-l", keyword, fullpath, (char*)NULL);
            perror("[ERRO] execlp falhou");
            _exit(1);
        }

        // pai
        close(pipefd[1]);
        // lê a saída do grep — se encontrar, vai enviar pelo pipe o nome do ficheiro
        char buffer[256];
        ssize_t n = bufferedRead(pipefd[0], buffer, sizeof buffer - 1);
        close(pipefd[0]);
        waitpid(pid, NULL, 0);

        if (n > 0) {
            // grep -l escreve "fullpath\n", então basta saber que encontrou
            if (resultado->len > 1)  // testa se já há algum elemento
                g_string_append(resultado, ", ");
            g_string_append_printf(resultado, "%d", GPOINTER_TO_INT(key));
        }
    }

    g_string_append(resultado, "]");
    return g_string_free(resultado, FALSE);
}

char *metaInformationDataset_search_documents_sequential(MetaInformationDataset *dataset, const char *keyword) {
    return metaInformationDataset_search_documents_parallel(dataset, keyword, 1);
}

char *metaInformationDataset_search_documents_parallel(MetaInformationDataset *dataset, const char *keyword, int max_procs) {
    GString *resultado = g_string_new("[");
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, dataset->MetaInformation);

    GArray *matching_ids = g_array_new(FALSE, FALSE, sizeof(int));
    int active_children = 0;

    // Para guardar pids e respetivos doc_ids
    typedef struct {
        pid_t pid;
        int doc_id;
    } ChildInfo;

    GArray *children = g_array_new(FALSE, FALSE, sizeof(ChildInfo));

    while (g_hash_table_iter_next(&iter, &key, &value)) {
        if (value == NULL) continue;

        MetaInformation *meta = (MetaInformation *)value;
        int doc_id = GPOINTER_TO_INT(key);

        char fullpath[MAX_PATH];
        metaInformationDataset_buildfull_documentpath(fullpath, sizeof(fullpath), dataset, meta);

        pid_t pid = fork();
        if (pid == 0) {
            // Filho: apenas executa grep
            execlp("grep", "grep", "-q", keyword, fullpath, (char*)NULL);
            _exit(1); // grep não encontrou ou erro
        } else if (pid > 0) {
            ChildInfo info = {pid, doc_id};
            g_array_append_val(children, info);
            active_children++;
        }

        // Se atingimos o limite, espera por algum
        if (active_children >= max_procs) {
            int status;
            pid_t ended = wait(&status);
            active_children--;

            // Verifica se o grep encontrou a keyword
            for (guint i = 0; i < children->len; i++) {
                ChildInfo info = g_array_index(children, ChildInfo, i);
                if (info.pid == ended) {
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                        g_array_append_val(matching_ids, info.doc_id);
                    }
                    // Remove este elemento da lista
                    g_array_remove_index(children, i);
                    break;  // Só pode haver um com este pid
                }
            }
            
        }
    }

    // Espera por os restantes filhos
    while (active_children > 0) {
        int status;
        pid_t ended = wait(&status);
        active_children--;

        for (guint i = 0; i < children->len; i++) {
            ChildInfo info = g_array_index(children, ChildInfo, i);
            if (info.pid == ended && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                g_array_append_val(matching_ids, info.doc_id);
                break;
            }
        }
    }

    // Constrói o resultado
    for (guint i = 0; i < matching_ids->len; i++) {
        if (i > 0) g_string_append(resultado, ", ");
        g_string_append_printf(resultado, "%d", g_array_index(matching_ids, int, i));
    }

    g_array_free(matching_ids, TRUE);
    g_array_free(children, TRUE);
    g_string_append(resultado, "]");
    return g_string_free(resultado, FALSE);
}



void metaInformationDataset_free(MetaInformationDataset *dataset) {
    if (dataset) {
        if (dataset->MetaInformation) {
            GHashTableIter iter;
            gpointer key, value;
            g_hash_table_iter_init(&iter, dataset->MetaInformation);
            while (g_hash_table_iter_next(&iter, &key, &value)) {
                g_free(value); // liberta MetaInformation*
            }
            g_hash_table_destroy(dataset->MetaInformation);
        }

        if (dataset->MetaInformationQueue) {
            g_queue_free(dataset->MetaInformationQueue);
        }

        g_free(dataset);
    }
}
