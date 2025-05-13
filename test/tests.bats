#!/usr/bin/env bats

# --- Load bats-assert and bats-support ---
BATS_TEST_SCRIPT_DIR="$( cd "$( dirname "$BATS_TEST_FILENAME" )" >/dev/null 2>&1 && pwd )"
BATS_LIBS_DIR="${BATS_TEST_SCRIPT_DIR}/test_libs"

if [ -f "${BATS_LIBS_DIR}/bats-support/load.bash" ] && [ -f "${BATS_LIBS_DIR}/bats-assert/load.bash" ]; then
    load "${BATS_LIBS_DIR}/bats-support/load.bash"
    load "${BATS_LIBS_DIR}/bats-assert/load.bash"
else
    echo "# WARNING: bats-support or bats-assert not found in ${BATS_LIBS_DIR}. Assertions will be basic." >&2
    assert_success() { [ "$status" -eq 0 ]; }
    assert_failure() { [ "$status" -ne 0 ]; }
    assert_output() { echo "$output" | grep -qE -- "$1"; }
    assert_output_contains() { echo "$output" | grep -qF -- "$1"; }
fi

# --- Global Configuration ---
readonly SERVER_EXEC="./bin/dserver"
readonly CLIENT_EXEC="./bin/dclient"
readonly TEST_DOC_FOLDER="documentos"
readonly CACHE_SIZE="10"
readonly SERVER_LOG_BATS="dserver_bats.log"
readonly SERVER_FIFO_DIR_BATS="fifos"
readonly SERVER_DATA_FILE_BATS="information_bats.bin"

readonly DOC1_ID=1
readonly DOC2_ID=2
readonly DOC3_ID=3

create_test_document() {
    local filename="$1"; shift
    mkdir -p "$TEST_DOC_FOLDER"
    printf "%s\n" "$@" > "$TEST_DOC_FOLDER/$filename" || {
        echo "# Failed to create document: $filename" >&2
        return 1
    }
}

setup_file() {
    echo "# Setting up for Bats tests..." >&2
    make all

    #mkdir -p "$TEST_DOC_FOLDER"
    #mkdir -p "$SERVER_FIFO_DIR_BATS"
    #rm -f "$SERVER_DATA_FILE_BATS"

    #if ! pgrep -x "dserver" > /dev/null; then
    #    "$SERVER_EXEC" "$TEST_DOC_FOLDER" "$CACHE_SIZE" --data-file "$SERVER_DATA_FILE_BATS" > "$SERVER_LOG_BATS" 2>&1 &
    #    readonly BATS_SERVER_PID=$!
    #    export BATS_SERVER_PID
    #    sleep 3
    #fi

    #if ! kill -0 "$BATS_SERVER_PID" 2>/dev/null; then
    #    echo "# SERVER FAILED TO START. Check $SERVER_LOG_BATS" >&2
    #    cat "$SERVER_LOG_BATS" >&2
    #    return 1
    #fi
    #echo "# Server started (PID $BATS_SERVER_PID)" >&2

    create_test_document "doc1_bats.txt" \
        "Hello from bats document one. apple keyword." \
        "The keyword 'apple' appears once more."

    create_test_document "doc2_bats.txt" \
        "This is bats document two. banana keyword." \
        "Another line for banana here. banana"

    create_test_document "doc3_bats.txt" \
        "Bats document three is for testing."
}

teardown_file() {
    echo "# Tearing down after Bats tests..." >&2
    if [ -n "$BATS_SERVER_PID" ] && ps -p "$BATS_SERVER_PID" > /dev/null; then
        echo "# Stopping server (PID: $BATS_SERVER_PID)..." >&2
        "$CLIENT_EXEC" -f >/dev/null 2>&1
        sleep 2
        if ps -p "$BATS_SERVER_PID" > /dev/null; then
            echo "# Force kill server $BATS_SERVER_PID..." >&2
            kill -9 "$BATS_SERVER_PID"
        else
            echo "# Server stopped." >&2
        fi
    else
        echo "# Server not running or PID invalid." >&2
    fi
    #rm -rf "$TEST_DOC_FOLDER"
    rm -f "$SERVER_LOG_BATS"
    #if [ -d "$SERVER_FIFO_DIR_BATS" ]; then
        #find "$SERVER_FIFO_DIR_BATS" -type p -delete 2>/dev/null
        #rmdir "$SERVER_FIFO_DIR_BATS" 2>/dev/null || echo "# Warning: $SERVER_FIFO_DIR_BATS not empty." >&2
    #fi
    rm -f "$SERVER_DATA_FILE_BATS"
    echo "# Bats cleanup complete." >&2
}

# Adiciona trap global
trap teardown_file EXIT

@test "0. Setup Test (verifica ambiente)" {
    run ls ./bin
    assert_output --partial "dserver"
}

# --- Testes de Indexação ---
@test "1. Index Document 1 (doc1_bats.txt)" {
    run "$CLIENT_EXEC" -a "Bats Title 1" "Author A-Bats" "2023" "doc1_bats.txt"
    assert_success
    assert_output --regexp "indexado com sucesso no indice $DOC1_ID|indexed.*$DOC1_ID"
}

@test "2. Index Document 2 (doc2_bats.txt)" {
    run "$CLIENT_EXEC" -a "Bats Story 2" "Author B-Bats;Author C-Bats" "2024" "doc2_bats.txt"
    assert_success
    assert_output --regexp "indexado com sucesso no indice $DOC2_ID|indexed.*$DOC2_ID"
}

@test "3. Index Document 3 (doc3_bats.txt)" {
    run "$CLIENT_EXEC" -a "Bats Tale 3" "Author D-Bats" "2025" "doc3_bats.txt"
    assert_success
    assert_output --regexp "indexado com sucesso no indice $DOC3_ID|indexed.*$DOC3_ID"
}

# --- Testes de Consulta ---
@test "4. Consult Document 1 (ID: $DOC1_ID)" {
    run "$CLIENT_EXEC" -c "$DOC1_ID"
    assert_success
    assert_output --partial "Bats Title 1"
    assert_output --partial "Author A-Bats"
}

@test "5. Consult Document 2 (ID: $DOC2_ID)" {
    run "$CLIENT_EXEC" -c "$DOC2_ID"
    assert_success
    assert_output --partial "Bats Story 2"
    assert_output --partial "Author B-Bats;Author C-Bats"
}

# --- Testes de Contagem ---
@test "6. Count lines with 'apple' in Document 1 (ID: $DOC1_ID)" {
    run "$CLIENT_EXEC" -l "$DOC1_ID" "apple"
    assert_success
    assert_output --regexp "Keyword 'apple' found 2 time(s)?"
}

@test "7. Count lines with 'banana' in Document 2 (ID: $DOC2_ID)" {
    run "$CLIENT_EXEC" -l "$DOC2_ID" "banana"
    assert_success
    assert_output --regexp "Keyword 'banana' found 2 time(s)?"
}

@test "8. Count lines with 'nonexistentkeyword' in Document 1 (ID: $DOC1_ID)" {
    run "$CLIENT_EXEC" -l "$DOC1_ID" "nonexistentkeyword"
    assert_success
    assert_output --regexp "Keyword 'nonexistentkeyword' found 0 time(s)?"
}

# --- Testes de Pesquisa ---
@test "9. Search for documents containing 'apple'" {
    run "$CLIENT_EXEC" -s "apple"
    assert_success
    assert_output --partial "[$DOC1_ID]"
}

@test "10. Search for documents containing 'banana'" {
    run "$CLIENT_EXEC" -s "banana"
    assert_success
    assert_output --partial "[$DOC2_ID]"
}

## NOTA: Não sei se é suposto contar as palavras apenas se for exatamente iguais ou se tiver minuscula ou maiuscula conta na mesma
@test "11. Search for documents containing 'Bats document'" {
    skip "Temporarily disabled"
    run "$CLIENT_EXEC" -s "Bats document"
    assert_success
    assert_output --partial "$DOC1_ID"
    assert_output --partial "$DOC2_ID"
    assert_output --partial "$DOC3_ID"
    assert_output --regexp "\[.*\]"
}

@test "11. Search for documents containing 'document'" {
    run "$CLIENT_EXEC" -s "document"
    assert_success
    assert_output --partial "$DOC1_ID"
    assert_output --partial "$DOC2_ID"
    assert_output --partial "$DOC3_ID"
    assert_output --regexp "\[.*\]"
}

@test "12. Search for documents containing 'nonexistentkeywordxyz'" {
    run "$CLIENT_EXEC" -s "nonexistentkeywordxyz"
    assert_success
    assert_output --partial "[]"
}

# --- Testes de Remoção ---
@test "13. Remove Document 1 (ID: $DOC1_ID)" {
    run "$CLIENT_EXEC" -d "$DOC1_ID"
    assert_success
    assert_output --regexp "Index entry $DOC1_ID deleted|deleted"
}

@test "14. Consult Document 1 (ID: $DOC1_ID) after removal" {
    run "$CLIENT_EXEC" -c "$DOC1_ID"
    assert_success
    assert_output --partial "Document not found"
}

@test "15. Search for 'apple' after Document 1 removal" {
    run "$CLIENT_EXEC" -s "apple"
    assert_success
    if ! echo "$output" | grep -q "\[\]"; then
        refute_output --partial "$DOC1_ID"
    else
        assert_output --partial "[]"
    fi
}

@test "16. Attempt to remove already removed/non-existent Document 1" {
    run "$CLIENT_EXEC" -d "$DOC1_ID"
    assert_success
    assert_output --partial "Entry not found"
}

# --- Teste de Novo Documento ---
@test "17. Index a new document 'doc4_bats.txt' after removal" {
    skip "Temporarily disabled"
    create_test_document "doc4_bats.txt" \
        "Fourth bats document, post removal."
    run "$CLIENT_EXEC" -a "Bats Fourth Dimension" "Author E-Bats" "2026" "doc4_bats.txt"
    assert_success
    assert_output --regexp "indexado com sucesso no indice|indexed"
}

# --- Testes de Erro ---
@test "18. Invalid command - too few arguments for -a" {
    skip "Temporarily disabled"
    run "$CLIENT_EXEC" -a "JustBatsTitle"
    assert_success
    assert_output --partial "Erro: argumentos insuficientes para ADD"
}

@test "19. Invalid command - wrong flag '-x'" {
    skip "Temporarily disabled"
    run "$CLIENT_EXEC" -x "somevalue"
    assert_failure
    assert_output --partial "Comando inválido"
}
