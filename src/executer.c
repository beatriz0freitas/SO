
#include "executer.h"
#include "comando.h"
#include "metaInformationDataset.h"
#include "metaInformation.h"
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
char *executer(Executer *executer, Command *comando, MetaInformationDataset *dataset) {
    char *flag = comando_get_flag(comando);

    if (flag == NULL) {
        return NULL;
    }

    if (flag == "-a"){
        Information *metaInfo = information_new();
        setDocumentTitle(metaInfo, comando_get_arg_por_indice(comando, 0));
        setAuthor(metaInfo, comando_get_arg_por_indice(comando, 1));
        setYear(metaInfo, atoi(comando_get_arg_por_indice(comando, 0)));
        setPath(metaInfo, comando_get_arg_por_indice(comando, 3));

        int index = metaInformationDataset_add(dataset, metaInfo)

        char resposta[100]; // ou maior se precisar
        sprintf(resposta, "ficheiro foi indexado com sucesso no indice %d", index); //DUVIDA NAO SEI SE PODEMOS USAR
    }
    if (flag == "-c"){
        char *resposta = "ficheiro foi consultado com sucesso";
        //
    }
    if (flag == "-d"){
        char *resposta = "ficheiro foi apagado com sucesso";
        //
    }
    
    executer->num_executions++;
    return resposta;
}



