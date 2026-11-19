#!/usr/bin/env bash
# Estudos de caso da Parte 3 em todos os grafos de uma pasta.
#
# uso, a partir desta pasta:
#     bash rodar_estudos_p3.sh [pasta_dos_grafos] [pasta_de_resultados]
# padroes: ../../Grafos/P3 e resultados   (ou: make estudos)
#
# Para cada grafo (*.txt ou *.txt.gz; o .gz e descompactado para um
# temporario), roda ./bin/p3_estudos com as quatro questoes e grava a saida
# em <resultados>/brutos/<grafo>.txt. No fim, gerar_tabelas_p3.py monta
# <resultados>/tabelas.md.
#
# Variaveis de ambiente (repassadas ao programa; ver p3_estudos.c):
#     RODADAS=10               rodadas para as medias de tempo (Q3 e Q4)
#     FW_MAX_N, TODOS_PARES_RODADAS, TODOS_PARES_AMOSTRA

set -u

GRAFOS="${1:-../../Grafos/P3}"
RES="${2:-resultados}"
RODADAS="${RODADAS:-10}"

make -s p3_estudos || exit 1
mkdir -p "$RES/brutos"

INICIO=$(date +%s)
log() {
    printf '[%5ds] %s\n' "$(( $(date +%s) - INICIO ))" "$*"
}

{
    echo "data: $(date '+%Y-%m-%d %H:%M')"
    echo "commit: $(git rev-parse --short HEAD 2>/dev/null || echo desconhecido)"
    echo "sistema: $(uname -srm)"
    echo "processador: $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ //')"
    echo "memoria total: $(awk '/MemTotal/ {printf "%.1f GB", $2 / 1048576}' /proc/meminfo)"
    echo "compilador: $(gcc --version | head -n 1)"
    echo "flags: $(grep -m1 '^CFLAGS' Makefile | cut -d= -f2- | sed 's/^ //')"
    echo "rodadas: $RODADAS"
} > "$RES/ambiente.txt"

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

shopt -s nullglob
ARQUIVOS=("$GRAFOS"/*.txt "$GRAFOS"/*.txt.gz)
if [ ${#ARQUIVOS[@]} -eq 0 ]; then
    echo "nenhum grafo em $GRAFOS (baixe os grafos da Parte 3 do site da disciplina)" >&2
    exit 1
fi

for arquivo in $(printf '%s\n' "${ARQUIVOS[@]}" | sort -V); do
    nome=$(basename "$arquivo")
    nome=${nome%.gz}
    nome=${nome%.txt}
    entrada="$arquivo"
    if [[ "$arquivo" == *.gz ]]; then
        log "$nome: descompactando"
        gzip -dc "$arquivo" > "$TMP/$nome.txt"
        entrada="$TMP/$nome.txt"
    fi
    log "$nome: questoes 1 a 4 ($RODADAS rodadas)"
    if ! ./bin/p3_estudos "$entrada" "$RODADAS" 1234 > "$RES/brutos/$nome.txt" 2>&1; then
        echo "FALHOU (codigo $?)" >> "$RES/brutos/$nome.txt"
        log "  falhou: $nome"
    fi
    sed -i "s#^grafo: .*#grafo: $nome#" "$RES/brutos/$nome.txt"
    rm -f "$TMP/$nome.txt"
done

log "gerando $RES/tabelas.md"
python3 gerar_tabelas_p3.py "$RES"
log "fim"
