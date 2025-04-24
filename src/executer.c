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
char *executer_execute(Executer *executer, Command *command, MetaInformationDataset *dataset) {
    CommandFlag flag = command_get_flag(command);
    char *resposta = g_new(char, 100);

    if (flag == CMD_INVALID) {
        return "Comando inválido";
    }

    switch(flag) {
        case CMD_ADD: {
            MetaInformation *metaInfo = metaInformation_new();
            printf("%s ", command_get_arg_por_indice(command, 0));
            printf("%s ", command_get_arg_por_indice(command, 1));
            printf("%s ", command_get_arg_por_indice(command, 2));
            printf("%s ", command_get_arg_por_indice(command, 3));

            metaInformation_set_DocumentTitle(metaInfo, command_get_arg_por_indice(command, 0));
            metaInformation_set_Author(metaInfo, command_get_arg_por_indice(command, 1));
            metaInformation_set_Year(metaInfo, atoi(command_get_arg_por_indice(command, 2)));
            metaInformation_set_Path(metaInfo, command_get_arg_por_indice(command, 3));

            int index = -1;

            index = metaInformationDataset_add(dataset, metaInfo);

            if (index == -1) {
                metaInformation_free(metaInfo);
                sprintf(resposta, "\nNão foi possível introduzir o ficheiro no índice");
            }else{
            sprintf(resposta, "\nficheiro foi indexado com sucesso no indice %d", index);
        }
            break;
        }

        case CMD_CONSULT: {
            int id = atoi(command_get_arg_por_indice(command, 0));
            MetaInformation *metaInfo = metaInformationDataset_consult(dataset, id);
            
            if (metaInfo != NULL) {
                sprintf(resposta, "Title: %s\nAuthors: %s\nYear: %d\nPath: %s",
                        metaInformation_get_DocumentTitle(metaInfo),
                        metaInformation_get_Author(metaInfo),
                        metaInformation_get_Year(metaInfo),
                        metaInformation_get_Path(metaInfo));
            } else {
                strcpy(resposta, "Document not found"); //confirmar se posso
            }
            break;
        }

        case CMD_DELETE: {
            int id = atoi(command_get_arg_por_indice(command, 0));
            if (metaInformationDataset_remove(dataset, id)) {
                sprintf(resposta, "Index entry %d deleted", id);
            } else {
                strcpy(resposta, "Entry not found");
            }
            break;
        }

        case CMD_LIST: {
            int id = atoi(command_get_arg_por_indice(command, 0));
            char *keyword = command_get_arg_por_indice(command, 1);
            int count = metaInformationDataset_count_keyword_lines(dataset, id, keyword);
            if (count != -1) {
                sprintf(resposta, "Keyword '%s' found %d times in document with ID %d", keyword, count, id);
            } else {
                strcpy(resposta, "Error counting keyword occurrences");
            }
            break;
        }

        case CMD_SEARCH: {
            char *keyword = command_get_arg_por_indice(command, 0);
            //TODO
            break;
        }

        case CMD_SEARCH_PROCESSOS: {
            char *keyword = command_get_arg_por_indice(command, 0);
            int num_processos = atoi(command_get_arg_por_indice(command, 1));
            //TODO
            break;
        }

        case CMD_SHUTDOWN: {
            //TODO: fechar todos os pipes, mensagem para o filho fechar
            break;
        }

        default:
            return "Comando inválido";
    }
    executer->num_executions++;
    return resposta;
}



