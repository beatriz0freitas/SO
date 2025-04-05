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
    char *resposta = NULL;

    if (flag == NULL) {
        return NULL;
    }

    if (flag == "-a"){
        Information *metaInfo = information_new();
        setDocumentTitle(metaInfo, command_get_arg_por_indice(command, 0));
        setAuthor(metaInfo, command_get_arg_por_indice(command, 1));
        setYear(metaInfo, atoi(command_get_arg_por_indice(command, 2)));
        setPath(metaInfo, command_get_arg_por_indice(command, 3));

        int index = metaInformationDataset_add(dataset, metaInfo);

        sprintf(resposta, "ficheiro foi indexado com sucesso no indice %d", index); //DUVIDA NAO SEI SE PODEMOS USAR
    }
    if (flag == "-c"){
        resposta = "ficheiro foi consultado com sucesso";
        //
    }
    if (flag == "-d"){
        resposta = "ficheiro foi apagado com sucesso";
        //
    }
    
    executer->num_executions++;
    return resposta;
}



