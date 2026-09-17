#!/usr/bin/env bash
# Estudos de caso (secao 4 do enunciado) em todos os grafos de uma pasta.
#
# uso, a partir da pasta "Trabalho 1 (Implementação em C)":
#     bash "Study cases/rodar_estudos.sh" [pasta_dos_grafos] [pasta_de_resultados]
# padroes: ../Grafos e resultados   (ou: make estudos)
#
# Para cada grafo_*.txt e cada representacao, roda as questoes 1 a 7 e grava
# a saida de cada programa em <resultados>/brutos/<grafo>/<representacao>/.
# O programa de saida (requisitos 2 e 4) roda uma vez por grafo, na lista, e
# grava os arquivos em <resultados>/saidas/ (fora do git: chegam a dezenas de MB).
# No fim, gerar_tabelas.py monta <resultados>/tabelas.md.
#
# Variaveis de ambiente:
#     BUSCAS=100               buscas por tipo nas questoes 2 e 3
#     DIAMETRO_EXATO_MAX=60000 diametro exato (BFS de cada vertice) so na lista
#                              e em grafos com ate esse numero de vertices
#
# A matriz so e carregada quando cabe na memoria disponivel (n*n inteiros);
# senao, a representacao fica registrada como inviavel.

set -u

GRAFOS="${1:-../Grafos}"
RES="${2:-resultados}"
BUSCAS="${BUSCAS:-100}"
DIAMETRO_EXATO_MAX="${DIAMETRO_EXATO_MAX:-60000}"

if [ ! -f Makefile ] || [ ! -d "Study cases" ]; then
    echo "rode a partir da pasta da implementacao em C (onde fica o Makefile)" >&2
    exit 1
fi
if ! ls "$GRAFOS"/grafo_*.txt >/dev/null 2>&1; then
    echo "nenhum grafo_*.txt em $GRAFOS (baixe os grafos do site da disciplina)" >&2
    exit 1
fi

make -s all casos || exit 1
mkdir -p "$RES/brutos" "$RES/saidas"

INICIO=$(date +%s)
log() {
    printf '[%5ds] %s\n' "$(( $(date +%s) - INICIO ))" "$*"
}

# executa um programa gravando a saida; registra falhas no proprio arquivo
rodar() {
    local destino="$1"
    shift
    if ! "$@" > "$destino" 2>&1; then
        echo "FALHOU (codigo $?): $*" >> "$destino"
        log "  falhou: $(basename "$destino")"
    fi
}

{
    echo "data: $(date '+%Y-%m-%d %H:%M')"
    echo "commit: ${COMMIT:-$(git rev-parse --short HEAD 2>/dev/null || echo desconhecido)}"
    echo "sistema: $(uname -srm)"
    echo "processador: $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ //')"
    echo "nucleos: $(nproc)"
    echo "memoria total: $(awk '/MemTotal/ {printf "%.1f GB", $2 / 1048576}' /proc/meminfo)"
    echo "compilador: $(gcc --version | head -n 1)"
    echo "flags: $(grep -m1 '^CFLAGS' Makefile | cut -d= -f2- | sed 's/^ //')"
    echo "buscas por tipo (Q2/Q3): $BUSCAS"
} > "$RES/ambiente.txt"

mapfile -t ARQUIVOS < <(ls -1 "$GRAFOS"/grafo_*.txt | sort -V)
for arquivo in "${ARQUIVOS[@]}"; do
    nome=$(basename "$arquivo" .txt)
    n=$(head -n 1 "$arquivo" | tr -dc '0-9')
    log "$nome (n = $n)"

    for rep in lista matriz; do
        dir="$RES/brutos/$nome/$rep"
        rm -rf "$dir"
        mkdir -p "$dir"

        if [ "$rep" = matriz ]; then
            necessario=$(( n * n * 4 + n * 8 ))
            disponivel=$(( $(awk '/MemAvailable/ {print $2}' /proc/meminfo) * 1024 ))
            if [ "$necessario" -gt $(( disponivel / 10 * 8 )) ]; then
                printf 'inviavel: a matriz precisaria de %d MB; memoria disponivel: %d MB\n' \
                    $(( necessario / 1048576 )) $(( disponivel / 1048576 )) > "$dir/inviavel.txt"
                log "  matriz: inviavel ($(( necessario / 1048576 )) MB)"
                continue
            fi
        fi

        log "  $rep: Q1 memoria"
        rodar "$dir/q1_memoria.txt" ./bin/questao1 "$arquivo" "$rep" auto
        log "  $rep: Q2/Q3 $BUSCAS BFS e $BUSCAS DFS"
        rodar "$dir/q2_3_buscas.txt" ./bin/questao2_3 "$arquivo" "$rep" "$BUSCAS"
        log "  $rep: Q4 a Q6"
        rodar "$dir/q4_pais.txt" ./bin/questao4 "$arquivo" "$rep"
        rodar "$dir/q5_distancias.txt" ./bin/questao5 "$arquivo" "$rep"
        rodar "$dir/q6_componentes.txt" ./bin/questao6 "$arquivo" "$rep"

        modo=aproximado
        if [ "$rep" = lista ] && [ "$n" -le "$DIAMETRO_EXATO_MAX" ]; then
            modo=ambos
        fi
        log "  $rep: Q7 diametro ($modo)"
        rodar "$dir/q7_diametro.txt" ./bin/questao7 "$arquivo" "$rep" "$modo"

        if [ "$rep" = lista ]; then
            log "  $rep: arquivos de saida (requisitos 2 e 4)"
            rodar "$dir/saida.txt" ./bin/saida "$arquivo" lista "$RES/saidas"
        fi
    done
done

log "gerando $RES/tabelas.md"
python3 "Study cases/gerar_tabelas.py" "$RES"
log "fim"
