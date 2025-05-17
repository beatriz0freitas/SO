#!/usr/bin/env bats

# --- Teste com múltiplos cenários ---


@test "Cenário 4: Cache muito grande" {
    export CACHE_SIZE=500
    export TOTAL_DOCS_TO_INDEX=2000
    export NUM_CONSULT_QUERIES=1000
    run bats test/test_cache_performance.bats
    [ "$status" -eq 0 ]
}

@test "Cenário 3: Cache grande" {
    export CACHE_SIZE=100
    export TOTAL_DOCS_TO_INDEX=1000
    export NUM_CONSULT_QUERIES=500
    run bats test/test_cache_performance.bats
    [ "$status" -eq 0 ]
}


@test "Cenário 2: Cache média" {
    export CACHE_SIZE=50
    export TOTAL_DOCS_TO_INDEX=500
    export NUM_CONSULT_QUERIES=200
    run bats test/test_cache_performance.bats
    [ "$status" -eq 0 ]
}

@test "Cenário 1: Cache pequena" {
    export CACHE_SIZE=5
    export TOTAL_DOCS_TO_INDEX=100
    export NUM_CONSULT_QUERIES=50
    run bats test/test_cache_performance.bats
    [ "$status" -eq 0 ]
}

