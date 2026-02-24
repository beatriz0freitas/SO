# SO - Sistema de Indexação e Pesquisa de Documentos

Serviço cliente-servidor em C que permite indexar e pesquisar documentos de texto guardados localmente. A comunicação entre cliente e servidor é feita através de FIFOs (named pipes), com suporte a concorrência, cache e pesquisa paralela.

---

## Compilação e Execução

### Pré-requisitos

* GCC
* GLib 2.0 (`pkg-config --libs glib-2.0`)
* Bats (para testes)

### Compilar

```bash
make
```

### Iniciar o servidor

```bash
bin/dserver <pasta_documentos> <tamanho_cache>
# Exemplo:
bin/dserver data/Gdataset 10
```

### Usar o cliente

```bash
bin/dclient <comando> [argumentos...]
```

### Compilar e iniciar tudo automaticamente

```bash
make run
```

---

## Comandos Disponíveis

| Flag   | Comando              | Argumentos                            | Descrição                                           |
| ------ | -------------------- | ------------------------------------- | ----------------------------------------------------- |
| `-a` | Indexar              | `<título> <autor> <ano> <caminho>` | Adiciona um documento ao índice                      |
| `-c` | Consultar            | `<id>`                              | Consulta os metadados de um documento                 |
| `-d` | Remover              | `<id>`                              | Remove logicamente um documento do índice            |
| `-l` | Contar linhas        | `<id> <keyword>`                    | Conta as linhas de um documento que contêm a keyword |
| `-s` | Pesquisar            | `<keyword>`                         | Lista IDs de documentos que contêm a keyword         |
| `-s` | Pesquisar (paralelo) | `<keyword> <n_processos>`           | Idem, com N processos paralelos                       |
| `-f` | Encerrar             | —                                    | Encerra o servidor                                    |

### Exemplos

```bash
# Indexar um documento
bin/dclient -a "Título do Livro" "Autor Nome" 2023 "docs/livro.txt"

# Consultar documento com ID 3
bin/dclient -c 3

# Remover documento com ID 3
bin/dclient -d 3

# Contar ocorrências da palavra "linux" no documento 2
bin/dclient -l 2 linux

# Pesquisar documentos que contenham "kernel"
bin/dclient -s kernel

# Pesquisar com 4 processos paralelos
bin/dclient -s kernel 4

# Encerrar o servidor
bin/dclient -f
```

---

## Testes

Os testes utilizam o framework [Bats](https://github.com/bats-core/bats-core).

```bash
# Instalar bibliotecas de suporte (só necessário uma vez)
make test/test_libs

# Testes de arranque/paragem do servidor
make test_start_stop

# Testes de funcionalidades
make test_features

# Testes de persistência
make test_persistence

# Testes de pesquisa paralela
make test_paralell_search

# Testes de concorrência
make test_concurrency

# Testes de desempenho da cache
make test_cache_performance
```
