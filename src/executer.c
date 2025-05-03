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

//NOTA: Falta adaptar isto para executar com varios utilizadores ao mesmo tempo
char *executer_execute(Executer *executer, Command *command, MetaInformationDataset *dataset, gboolean *terminar_servidor) {
    if (!executer || !command || !dataset || !terminar_servidor) {
        char *resposta_erro = g_strdup("Erro interno: ponteiro NULL recebido.");
        return resposta_erro;
    }
    
    CommandFlag flag = command_get_flag(command);
    char *resposta = g_new(char, 100);

    if (flag == CMD_INVALID) {
        sprintf(resposta, "Comando inválido");
        return resposta;
    }

    switch(flag) {
        case CMD_ADD: {
            if (command_get_num_args(command) < 4) {
                snprintf(resposta, 100, "Erro: argumentos insuficientes para ADD");
                break;
            }
        
            MetaInformation metaInfo = metaInformation_new();
        
            const char *titulo = command_get_arg_por_indice(command, 0);
            const char *autor = command_get_arg_por_indice(command, 1);
            const char *ano_str = command_get_arg_por_indice(command, 2);
            const char *path = command_get_arg_por_indice(command, 3);
        
            if (!titulo || !autor || !ano_str || !path) {
                snprintf(resposta, 100, "Erro: argumento NULL");
                break;
            }

            metaInformation_set_DocumentTitle(&metaInfo, titulo);
            metaInformation_set_Author(&metaInfo, autor);
            metaInformation_set_Year(&metaInfo, atoi(ano_str));
            metaInformation_set_Path(&metaInfo, path);

            int index = metaInformationDataset_add(dataset, &metaInfo);

            if(index == -1) {
                sprintf(resposta, "Erro ao indexar o ficheiro");
            } else {
                sprintf(resposta, "Ficheiro indexado com sucesso no indice %d", index);
            }
            printf("[DEBUG] ADD: ficheiro %s indexado no ID %d\n", command_get_arg_por_indice(command, 3), index);

            break;
        }

        case CMD_CONSULT: {
            if (command_get_num_args(command) < 1) {
                snprintf(resposta, 100, "Erro: ID não fornecido");
                break;
            }

            const char *id_str = command_get_arg_por_indice(command, 0);
            if (!id_str) {
                strcpy(resposta, "Erro: ID inválido.");
                break;
            }
            int id = atoi(id_str);
            MetaInformation *metaInfo = metaInformationDataset_consult(dataset, id);
            
            if (metaInfo != NULL) {
                sprintf(resposta, "Title: %s\nAuthors: %s\nYear: %d\nPath: %s",
                        metaInformation_get_DocumentTitle(metaInfo),
                        metaInformation_get_Author(metaInfo),
                        metaInformation_get_Year(metaInfo),
                        metaInformation_get_Path(metaInfo));
            } else {
                sprintf(resposta, "Document not found"); //confirmar se posso
            }
            break;
            
        }

        case CMD_DELETE: {
            if (command_get_num_args(command) < 1) {
                snprintf(resposta, 100, "Erro: ID não fornecido");
                break;
            }

            const char *id_str = command_get_arg_por_indice(command, 0);
            if (!id_str) {
                strcpy(resposta, "Erro: ID inválido.");
                break;
            }
            int id = atoi(id_str);
            if (metaInformationDataset_remove(dataset, id)) {
                sprintf(resposta, "Index entry %d deleted", id);
            } else {
                strcpy(resposta, "Entry not found");
            }
            break;
        }

        case CMD_LIST: {
            if (command_get_num_args(command) < 2) {
                snprintf(resposta, 100, "Erro: numero de linhas ou keyword não fornecida");
                break;
            }

            const char *id_str = command_get_arg_por_indice(command, 0);
            if (!id_str) {
                strcpy(resposta, "Erro: ID inválido.");
                break;
            }

            int id = atoi(id_str);
            char *keyword = command_get_arg_por_indice(command, 1);
            if (!keyword) {
                strcpy(resposta, "Erro: keyword inválida.");
                break;
            }

            int count = metaInformationDataset_count_keyword_lines(dataset, id, keyword);
            if (count != -1) {
                sprintf(resposta, "Keyword '%s' found %d times in document with ID %d", keyword, count, id);
            } else {
                strcpy(resposta, "Error counting keyword occurrences");
            }
            break;
        }

        case CMD_SEARCH: {
            if (command_get_num_args(command) < 1) {
                snprintf(resposta, 100, "Erro: keyword não fornecida");
                break;
            }
            char *keyword = command_get_arg_por_indice(command, 0);
            if (!keyword) {
                strcpy(resposta, "Erro: keyword inválida.");
                break;
            }
            
            char *result = metaInformationDataset_search_documents(dataset, keyword);
            if (result != NULL) {
                sprintf(resposta, "%s", result);
                g_free(result);
            } else {
                strcpy(resposta, "[]");
            }
            
            break;
        }

        case CMD_SEARCH_PROCESSOS: {
            char *keyword = command_get_arg_por_indice(command, 0);
            int num_processos = atoi(command_get_arg_por_indice(command, 1));
            //TODO
            break;
        }

        case CMD_SHUTDOWN: {
            *terminar_servidor = TRUE;
            strcpy(resposta, "Servidor encerrado com sucesso.");
            break;
        }
    }
    executer->num_executions++;
    return resposta;
}



