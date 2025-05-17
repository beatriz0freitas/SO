#include "executer.h"
#include <glib.h>

struct Executer {
    int num_executions;
};

Executer *executer_new() {
    Executer *executer = g_new(Executer, 1);
    executer->num_executions = 0;
    return executer;
}

void executer_free(Executer *executer) {
    if (executer != NULL) {
        g_free(executer);
    }
}

static char *handle_add(Command *command, MetaInformationDataset *dataset) {
    if (command_get_num_args(command) < 4) {
        return g_strdup("Erro: argumentos insuficientes para ADD");
    }

    const char *titulo = command_get_arg_por_indice(command, 0);
    const char *autor = command_get_arg_por_indice(command, 1);
    const char *ano_str = command_get_arg_por_indice(command, 2);
    const char *path = command_get_arg_por_indice(command, 3);

    if (!titulo || !autor || !ano_str || !path) {
        return g_strdup("Erro: argumento NULL");
    }

    MetaInformation metaInfo = metaInformation_new();
    metaInformation_set_DocumentTitle(&metaInfo, titulo);
    metaInformation_set_Author(&metaInfo, autor);
    metaInformation_set_Year(&metaInfo, atoi(ano_str));
    metaInformation_set_Path(&metaInfo, path);

    int id = metaInformationDataset_add(dataset, &metaInfo);
    char *resposta = g_malloc(100);

    if (id == -1) {
        sprintf(resposta, "Erro ao indexar o ficheiro");
    } else if (id < -1) {
        sprintf(resposta, "Ficheiro já indexado no indice %d", id + CODIGOJAINDEXADO);
    } else {
        sprintf(resposta, "Ficheiro indexado com sucesso no indice %d", id);
    }

    printf("[DEBUG] ADD: ficheiro %s indexado no ID %d\n", path, id);
    return resposta;
}

static char *handle_consult(Command *command, MetaInformationDataset *dataset) {
    if (command_get_num_args(command) < 1) {
        return g_strdup("Erro: ID não fornecido");
    }

    const char *id_str = command_get_arg_por_indice(command, 0);
    if (!id_str) return g_strdup("Erro: ID inválido.");

    int id = atoi(id_str);
    MetaInformation *metaInfo = metaInformationDataset_consult(dataset, id);
    char *resposta = g_malloc(100);

    if (metaInfo) {
        sprintf(resposta, "Title: %s\nAuthors: %s\nYear: %d\nPath: %s",
                metaInformation_get_DocumentTitle(metaInfo),
                metaInformation_get_Author(metaInfo),
                metaInformation_get_Year(metaInfo),
                metaInformation_get_Path(metaInfo));
    } else {
        sprintf(resposta, "Document not found");
    }
    return resposta;
}

static char *handle_delete(Command *command, MetaInformationDataset *dataset) {
    if (command_get_num_args(command) < 1) {
        return g_strdup("Erro: ID não fornecido");
    }

    const char *id_str = command_get_arg_por_indice(command, 0);
    if (!id_str) return g_strdup("Erro: ID inválido.");

    int id = atoi(id_str);
    char *resposta = g_malloc(100);

    if (metaInformationDataset_remove(dataset, id)) {
        sprintf(resposta, "Index entry %d deleted", id);
    } else {
        sprintf(resposta, "Entry not found");
    }
    return resposta;
}

static char *handle_list(Command *command, MetaInformationDataset *dataset) {
    if (command_get_num_args(command) < 2) {
        return g_strdup("Erro: numero de linhas ou keyword não fornecida");
    }

    const char *id_str = command_get_arg_por_indice(command, 0);
    char *keyword = command_get_arg_por_indice(command, 1);

    if (!id_str || !keyword) return g_strdup("Erro: argumento inválido.");

    int id = atoi(id_str);
    int count = metaInformationDataset_count_keyword_lines(dataset, id, keyword);
    char *resposta = g_malloc(100);

    if (count != -1) {
        sprintf(resposta, "Keyword '%s' found %d times in document with ID %d", keyword, count, id);
    } else {
        strcpy(resposta, "Error counting keyword occurrences");
    }
    return resposta;
}

static char *handle_search(Command *command, MetaInformationDataset *dataset) {
    if (command_get_num_args(command) < 1) {
        return g_strdup("Erro: keyword não fornecida");
    }

    char *keyword = command_get_arg_por_indice(command, 0);
    if (!keyword) return g_strdup("Erro: keyword inválida.");

    char *result = metaInformationDataset_search_documents(dataset, keyword);
    if (result) {
        char *resposta = g_strdup(result);
        g_free(result);
        return resposta;
    }
    return g_strdup("[]");
}

static char *handle_search_processos(Command *command, MetaInformationDataset *dataset) {
    if (command_get_num_args(command) < 2) {
        return g_strdup("Erro: keyword ou número de processos não fornecido");
    }

    char *keyword = command_get_arg_por_indice(command, 0);
    int num_processos = atoi(command_get_arg_por_indice(command, 1));

    if (!keyword || num_processos <= 0) {
        return g_strdup("Erro:  Número de processos inválido.");
    }

    char *result = metaInformationDataset_search_documents_parallel(dataset, keyword, num_processos);
    if (result) {
        char *resposta = g_strdup(result);
        g_free(result);
        return resposta;
    }
    return g_strdup("[]");
}

static char *handle_shutdown(gboolean *terminar_servidor) {
    *terminar_servidor = TRUE;
    return g_strdup("Servidor encerrado com sucesso.");
}

char *executer_execute(Executer *executer, Command *command, MetaInformationDataset *dataset, gboolean *terminar_servidor) {
    if (!executer || !command || !dataset || !terminar_servidor) {
        return g_strdup("Erro interno: ponteiro NULL recebido.");
    }

    CommandFlag flag = command_get_flag(command);
    char *resposta = NULL;

    if (flag == CMD_INVALID) {
        return g_strdup("Comando inválido");
    }

    switch (flag) {
        case CMD_ADD:
            resposta = handle_add(command, dataset);
            break;
        case CMD_CONSULT:
            resposta = handle_consult(command, dataset);
            break;
        case CMD_DELETE:
            resposta = handle_delete(command, dataset);
            break;
        case CMD_LIST:
            resposta = handle_list(command, dataset);
            break;
        case CMD_SEARCH:
            resposta = handle_search(command, dataset);
            break;
        case CMD_SEARCH_PROCESSOS:
            resposta = handle_search_processos(command, dataset);
            break;
        case CMD_SHUTDOWN:
            resposta = handle_shutdown(terminar_servidor);
            break;
        default:
            resposta = g_strdup("Comando não suportado.");
            break;
    }

    executer->num_executions++;
    return resposta;
}
