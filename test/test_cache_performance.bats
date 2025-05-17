#!/usr/bin/env bats

# --- Configurações ---

readonly DCLIENT_CMD="./bin/dclient" 
readonly DSERVER_CMD="./bin/dserver" 
readonly TEST_DOC_FOLDER="data/Gdataset"
readonly GCATALOG="Gcatalog.tsv"
readonly CSV_RESULTS_FILE="test/test_cache_performance.csv"


readonly CACHE_SIZE="10"
readonly TOTAL_DOCS_TO_INDEX=200  
readonly NUM_CONSULT_QUERIES=1000  
readonly PROBABILITY_OF_REPEATING_QUERY=50 
readonly HISTORY_SIZE_FOR_REPEATS=50 


DSERVER_PID=""

# --- Funções Auxiliares ---

start_the_server() {
    "$DSERVER_CMD" "$TEST_DOC_FOLDER" "$CACHE_SIZE" > /dev/null 2>&1 &
    DSERVER_PID=$!
    sleep 2
}

stop_the_server() {
    if [ -n "$DSERVER_PID" ]; then
        echo "A parar o servidor dserver (PID $DSERVER_PID)..."
        # Tentar um encerramento gracioso primeiro, se disponível
        # $DCLIENT_CMD -f # Exemplo se -f for o comando de shutdown
        # sleep 1
        # Se não houver comando de shutdown ou como fallback:
        if ! kill -0 "$DSERVER_PID" 2>/dev/null; then
            echo "Servidor dserver (PID $DSERVER_PID) já não estava a correr."
        else
            # Enviar SIGTERM primeiro
            kill "$DSERVER_PID"
            # Esperar um pouco
            sleep 1
            # Verificar se ainda está a correr e forçar com SIGKILL se necessário
            if kill -0 "$DSERVER_PID" 2>/dev/null; then
                echo "Servidor dserver (PID $DSERVER_PID) não parou com SIGTERM, a enviar SIGKILL."
                kill -9 "$DSERVER_PID"
            fi
        fi
        wait "$DSERVER_PID" 2>/dev/null || true
        DSERVER_PID=""
        echo "Servidor dserver parado."
    fi
}

setup_initial_documents() {
    echo "A indexar documentos usando $GCATALOG..."
    # Limpar dados de indexação anteriores, se necessário
    # rm -f information.bin # Exemplo, ajuste conforme o nome do seu ficheiro de índice

    run ./bin/addGdatasetMetadata.sh "$GCATALOG"
    if [ "$status" -ne 0 ]; then
        echo "Falha ao indexar documentos com addGdatasetMetadata.sh: $output"
        echo "Status: $status"
        return 1 # Falha a configuração se a indexação falhar
    fi
    echo "Indexação de documentos via $GCATALOG concluída."
    # NOTA: Se o número de documentos indexados por addGdatasetMetadata.sh
    # ou os seus IDs não corresponderem a TOTAL_DOCS_TO_INDEX (atualmente ${TOTAL_DOCS_TO_INDEX}),
    # a lógica de consulta aleatória no teste principal pode precisar de ajustes.
}

# --- Hooks Globais (Setup/Teardown para todo o ficheiro de teste) ---
setup_file() {
    echo "Setup global do ficheiro de teste..."
    # Limpar execuções anteriores do servidor, se necessário
    pkill -f "$DSERVER_CMD" || true
    sleep 1
    # Criar diretório para resultados CSV, se não existir
    mkdir -p "$(dirname "$CSV_RESULTS_FILE")"
    # Não limpar o ficheiro CSV antigo para permitir append
}

teardown_file() {
    echo "Teardown global do ficheiro de teste..."
    # Garantir que o servidor é parado no final de todos os testes no ficheiro
    # (Se não for parado em cada @test com stop_the_server)
    # stop_the_server # Descomente se o servidor for partilhado entre testes e iniciado em setup_file
    pkill -f "$DSERVER_CMD" || true # Limpeza final
}


# --- Testes ---

@test "Consulta de documentos com padrão aleatório e repetições" {
    echo "A iniciar teste: Consulta de documentos com padrão aleatório e repetições"

    local test_start_time=$(date +%s%N) # Start time for the test

    # Iniciar o servidor para este teste
    start_the_server
    if [ -z "$DSERVER_PID" ] || ! kill -0 "$DSERVER_PID" 2>/dev/null; then
        echo "Falha ao iniciar o servidor dserver. A abortar teste."
        return 1
    fi

    # Indexar documentos
    setup_initial_documents
    if [ $? -ne 0 ]; then
        echo "Falha na configuração da indexação. A abortar teste."
        stop_the_server
        return 1
    fi

    echo "A realizar $NUM_CONSULT_QUERIES consultas com padrão aleatório e repetições..."

    for i in $(seq 1 "$NUM_CONSULT_QUERIES"); do
        local doc_id_to_consult
        if [ $((RANDOM % 100)) -lt "$PROBABILITY_OF_REPEATING_QUERY" ]; then
            doc_id_to_consult=$(( (RANDOM % TOTAL_DOCS_TO_INDEX) + 1 ))
        else
            doc_id_to_consult=$(( (RANDOM % TOTAL_DOCS_TO_INDEX) + 1 ))
        fi

        run $DCLIENT_CMD CONSULT "$doc_id_to_consult"
    done

    echo "Consultas concluídas."

    local test_end_time=$(date +%s%N) # End time for the test
    local total_time_ms=$(( (test_end_time - test_start_time) / 1000000 ))

    # Append the total parameters and time to the CSV file
    if [ ! -f "$CSV_RESULTS_FILE" ]; then
        echo "CACHE_SIZE,TOTAL_DOCS_TO_INDEX,NUM_CONSULT_QUERIES,PROBABILITY_OF_REPEATING_QUERY,HISTORY_SIZE_FOR_REPEATS,TOTAL_TIME_MS" > "$CSV_RESULTS_FILE"
    fi
    echo "$CACHE_SIZE,$TOTAL_DOCS_TO_INDEX,$NUM_CONSULT_QUERIES,$PROBABILITY_OF_REPEATING_QUERY,$HISTORY_SIZE_FOR_REPEATS,$total_time_ms" >> "$CSV_RESULTS_FILE"

    echo "Resultados gravados em $CSV_RESULTS_FILE"
    cat "$CSV_RESULTS_FILE"

    stop_the_server
}

# Pode adicionar mais casos de teste aqui.
# Se quiser testar diferentes tamanhos de cache, você precisaria de testes separados
# ou uma forma de passar o tamanho da cache para start_the_server e registá-lo no CSV.