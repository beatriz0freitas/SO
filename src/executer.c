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


//NOTA: Falta adaptar isto para executar com varios utilizadores ao mesmo tempo
char *executer(Executer *executer, Command *command, MetaInformationDataset *dataset) {
    char *flag = command_get_flag(command);
    char *resposta = malloc(100 * sizeof(char));

    if (flag == NULL) {
        return NULL;
    }

    if (strcmp(flag, "-a") == 0){
        MetaInformation *metaInfo = metaInformation_new();
        printf("%s ", command_get_arg_por_indice(command, 0));
        printf("%s ", command_get_arg_por_indice(command, 1));
        printf("%s ", command_get_arg_por_indice(command, 2));
        printf("%s ", command_get_arg_por_indice(command, 3));
        metaInformation_set_DocumentTitle(metaInfo, command_get_arg_por_indice(command, 0));
        metaInformation_set_Author(metaInfo, command_get_arg_por_indice(command, 1));
        metaInformation_set_Year(metaInfo, atoi(command_get_arg_por_indice(command, 2)));
        metaInformation_set_Path(metaInfo, command_get_arg_por_indice(command, 3));

        int index = metaInformationDataset_add(dataset, metaInfo);

        sprintf(resposta, "\nficheiro foi indexado com sucesso no indice %d", index); //DUVIDA NAO SEI SE PODEMOS USAR
    }
    if (strcmp(flag, "-c") == 0){
        resposta = "ficheiro foi consultado com sucesso";
        //
    }
    if (strcmp(flag, "-d") == 0){
        resposta = "ficheiro foi apagado com sucesso";
        //
    }
    
    executer->num_executions++;
    return resposta;
}



