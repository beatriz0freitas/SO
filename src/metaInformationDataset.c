#include "metaInformationDataset.h"
#include <glib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include "utils.h"
#include "cache.h"

#define FILENAME "information.bin" // nome do ficheiro binário onde vamos guardar


MetaInformationDataset *metaInformationDataset_new(const char *document_folder, int cache_size) {
    MetaInformationDataset * dataset = g_new0(MetaInformationDataset, 1);
    dataset->MetaInformationQueue = g_queue_new();
    dataset->nextindex = 1;
    strncpy(dataset->folder, document_folder, MAX_PATH-1);
    strncpy(dataset->filename,"information.bin", MAX_PATH - 1);
    dataset->folder[MAX_PATH-1] = '\0';
    dataset->cache = cache_new(cache_size); // Initialize cache with the provided size
    return dataset;
}

void metaInformationDataset_buildfull_documentpath(char *dest, size_t size, const MetaInformationDataset *dataset, const MetaInformation *info) {
    snprintf(dest, size, "%s/%s", dataset->folder, info->path);
}

int metaInformationDataset_add(MetaInformationDataset *dataset, MetaInformation *metaInfo) {
   
    int fd = open_with_lock(dataset->filename, O_CREAT | O_RDWR, 0666, LOCK_EX); // O_APPEND removido para controlo manual do offset
    if (fd == -1) {
        perror("[DEBUG]: Erro ao abrir ficheiro");
        return -1;
    }

    // Verificar se o ficheiro do documento existe fisicamente
    char caminhoCompleto[MAX_PATH];
    metaInformationDataset_buildfull_documentpath(caminhoCompleto, sizeof(caminhoCompleto), dataset, metaInfo);

    int doc_fd = open_with_lock(caminhoCompleto, O_RDONLY, 0, LOCK_SH);
    if (doc_fd == -1) {
        perror("[DEBUG] Ficheiro do documento não encontrado");
        close_with_unlock(fd);
        return -1;
    }
    close_with_unlock(doc_fd);

    //  Verificar duplicação: percorre metainformações já existentes 
    MetaInformation existente;
    int pos = 0;
    while (bufferedRead(fd, &existente, metaInformation_size()) == metaInformation_size()) {
        if (metaInformation_is_deleted(&existente)) {
            pos++;
            continue;
        }

        char caminhoCompletoExistente[MAX_PATH];
        metaInformationDataset_buildfull_documentpath(caminhoCompletoExistente, sizeof(caminhoCompletoExistente), dataset, &existente);

        if (strcmp(caminhoCompletoExistente, caminhoCompleto) == 0) {
            fprintf(stderr, "[DEBUG] Documento já indexado: %s\n", caminhoCompleto);
            close_with_unlock(fd);
            return pos - CODIGOJAINDEXADO; // Código de erro para duplicado, mas possivel de transmitir a posição atual NOTA: rever isto
        }
        pos++;
    }

    int id;
    if (g_queue_is_empty(dataset->MetaInformationQueue)) {
        id = dataset->nextindex++;  // Novo ID sequencial
    } else {
        id = GPOINTER_TO_INT(g_queue_pop_head(dataset->MetaInformationQueue));  // Reutiliza posição e ID antigos de uma metainformação apagada
    }

    int posicao_registo = id; // Assumimos que ID e posição no ficheiro coincidem
    metaInformation_set_IdDocument(metaInfo, id);

    lseek(fd, posicao_registo * metaInformation_size(), SEEK_SET);
    if (bufferedWrite(fd, metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("[DEBUG]: Erro a escrever no ficheiro");
        close_with_unlock(fd);
        return -1;
    }

    close_with_unlock(fd);

    return id;
}

int metaInformationDataset_add_with_cache_write_through(MetaInformationDataset *dataset, MetaInformation *metaInfo) {
    int id = metaInformationDataset_add(dataset, metaInfo);
    if (id >= 0) {
        cache_put(dataset->cache, id, metaInfo);
    }
    return id;
}

gboolean metaInformationDataset_remove(MetaInformationDataset *dataset, int key) {

    int posicao_registo = key; 

    int fd = open_with_lock(dataset->filename, O_CREAT | O_RDWR, 0666, LOCK_EX);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro");
        return FALSE;
    }

    //procura o registo no ficheiro
    lseek(fd, posicao_registo * metaInformation_size(), SEEK_SET);
    
    MetaInformation metaInfo;
    if (bufferedRead(fd, &metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro ao ler do ficheiro");
        close_with_unlock(fd);
        return FALSE;
    }

    if (metaInformation_is_deleted(&metaInfo)){
        close_with_unlock(fd);
        return FALSE;
    }

    //marca como apagado (tecnica das tombstones)
    metaInformation_mark_as_deleted(&metaInfo); 

    lseek(fd, posicao_registo * metaInformation_size(), SEEK_SET);
    if (bufferedWrite(fd, &metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro ao escrever no ficheiro");
        close_with_unlock(fd);
        return FALSE;
    }

    close_with_unlock(fd);

    // Atualiza dataset (adiciona o id á stack de ids de metainformação apagados, para ser utilizada posteriormente)
    g_queue_push_tail(dataset->MetaInformationQueue, GINT_TO_POINTER(posicao_registo));

    cache_remove(dataset->cache, key); 


    return TRUE;
}


MetaInformation *metaInformationDataset_consult(MetaInformationDataset *dataset, int key) {
    MetaInformation *cached_info = cache_get(dataset->cache, key);
    if (cached_info) {
        return cached_info;
    }

    int fd = open_with_lock(dataset->filename, O_RDONLY, 0,  LOCK_SH);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro");
        return NULL;
    }

    int posicao_registo = key;
    lseek(fd, posicao_registo * metaInformation_size(), SEEK_SET);

    MetaInformation *metaInfo = g_malloc(metaInformation_size());
    if (bufferedRead(fd, metaInfo, metaInformation_size()) != metaInformation_size()) {
        perror("Erro a ler do ficheiro");
        g_free(metaInfo);
        close_with_unlock(fd);
        return NULL;
    }

    close_with_unlock(fd);

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

    if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0 && WEXITSTATUS(status) != 1)) { //aceita 0 (encontrou com sucesso) ou 1 (se não encontrou nenhuma palavra igual mas leu o ficheiro)
        return -1;
    }

    int count = atoi(buffer); // Converte a string recebida em int
    return count;

}

char *metaInformationDataset_search_documents(MetaInformationDataset *dataset, const char *keyword) {

    int fd = open_with_lock(dataset->filename, O_RDONLY, 0,  LOCK_SH);
    if (fd == -1) {
        perror("[ERRO] Não foi possível abrir o ficheiro de metainformação");
        return g_strdup("[]");
    }
    
    GString *resultado = g_string_new("[");

    int id = 0;
    MetaInformation meta;

    // Lê registos de metainformação até não haver mais dados
    while (bufferedRead(fd, &meta, metaInformation_size()) == metaInformation_size()) {
        if (metaInformation_is_deleted(&meta)) {
                id++;
                continue;
        }

        char fullpath[MAX_PATH];
        metaInformationDataset_buildfull_documentpath(fullpath, sizeof(fullpath), dataset, &meta);
        //printf("[DEBUG]: A procurar em: %s\n", fullpath);

        // cria pipe
        int pipefd[2];
        if (pipe(pipefd) < 0) {
            id++;
            continue;
        }
           

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
            g_string_append_printf(resultado, "%d", meta.idDocument);
        }

        id++;
    }
    g_string_append(resultado, "]");
    return g_string_free(resultado, FALSE);

}

char *metaInformationDataset_search_documents_sequential(MetaInformationDataset *dataset, const char *keyword) {
    return metaInformationDataset_search_documents_parallel(dataset, keyword, 1);
}

char *metaInformationDataset_search_documents_parallel(MetaInformationDataset *dataset, const char *keyword, int max_procs) {
    GString *resultado = g_string_new("[");
    GArray *matching_ids = g_array_new(FALSE, FALSE, sizeof(int));
    // Para guardar pids e respetivos doc_ids
    typedef struct {
        pid_t pid;
        int doc_id;
    } ChildInfo;

    GArray *children = g_array_new(FALSE, FALSE, sizeof(ChildInfo));

    int fd = open_with_lock(dataset->filename, O_RDONLY, 0,  LOCK_SH);
    if (fd == -1) {
        perror("[DEBUG] Erro ao abrir ficheiro de metainformação");
        return NULL;
    }

    int active_children = 0;
    int doc_id = 0;
    MetaInformation meta;

    while (read(fd, &meta, sizeof(MetaInformation)) == sizeof(MetaInformation)) {

        if (metaInformation_is_deleted(&meta)) {
            doc_id++;
            continue;
        }

        char fullpath[MAX_PATH];
        metaInformationDataset_buildfull_documentpath(fullpath, sizeof(fullpath), dataset, &meta);

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

        // Se atingimos o limite de processos, espera por algum
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
        doc_id++;
    }

    // Espera pelos  restantes filhos
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

        if (dataset->MetaInformationQueue) {
            g_queue_free(dataset->MetaInformationQueue);
        }

        if (dataset->cache) {
            cache_free(dataset->cache);
        }

        g_free(dataset);
    }
}
