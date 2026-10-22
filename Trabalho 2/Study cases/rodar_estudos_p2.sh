#!/usr/bin/env bash
# Estudos de caso da Parte 2 (secao 3 do enunciado do trabalho-P2) nos grafos
# com pesos de uma pasta, mais a rede de colaboracao.
#
# uso, a partir da pasta "Trabalho 2":
#     bash "Study cases/rodar_estudos_p2.sh" [pasta_dos_grafos] [pasta_de_resultados]
# padroes: ../Grafos e resultados   (ou: make estudos)
#
# So roda nos grafos COM PESOS (tres colunas na linha de aresta): os de duas
# colunas sao os da Parte 1 e ficam com o rodar_estudos.sh da pasta
# "Trabalho 1 (Implementação em C)". Para cada grafo com pesos e cada
# representacao, roda as questoes 1 e 2 da Parte 2 e grava a saida em
# <resultados>/brutos/<grafo>/<representacao>/. A questao da rede de
# colaboracao roda uma vez, fora do laco. No fim, gerar_tabelas_p2.py monta
# <resultados>/tabelas.md.
#
# Variaveis de ambiente:
#     K_DIJKSTRA=100           dijkstras por modo na questao 2
#     DIJKSTRA_VETOR_MAX=60000 dijkstra com vetor (O(n^2)) so em grafos com
#                              ate esse numero de vertices; maiores rodam so heap
#     REDE_COLABORACAO=...     rede de colaboracao (grafo com pesos) e
#     REDE_NOMES=...           arquivo de nomes: roda a questao da rede
#
# A matriz so e carregada quando cabe na memoria disponivel (n*n bytes de
# adjacencia, mais n*n doubles de pesos: n*n*9 no total); senao, a
# representacao fica registrada como inviavel.

set -u

GRAFOS="${1:-../Grafos}"
RES="${2:-resultados}"
K_DIJKSTRA="${K_DIJKSTRA:-100}"
DIJKSTRA_VETOR_MAX="${DIJKSTRA_VETOR_MAX:-60000}"

if [ ! -f Makefile ] || [ ! -d "Study cases" ]; then
    echo "rode a partir da pasta do Trabalho 2 (onde fica o Makefile)" >&2
    exit 1
fi
if ! ls "$GRAFOS"/grafo_*.txt >/dev/null 2>&1; then
    echo "nenhum grafo_*.txt em $GRAFOS (baixe os grafos do site da disciplina)" >&2
    exit 1
fi

make -s all casos_p2 || exit 1
mkdir -p "$RES/brutos"

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
    echo "dijkstras por modo (Q2): $K_DIJKSTRA"
} > "$RES/ambiente.txt"

mapfile -t ARQUIVOS < <(ls -1 "$GRAFOS"/grafo_*.txt | sort -V)
for arquivo in "${ARQUIVOS[@]}"; do
    colunas=$(awk 'NR==2 {print NF}' "$arquivo")
    if [ "$colunas" != "3" ]; then
        continue # grafo sem pesos: estudo da Parte 1
    fi

    nome=$(basename "$arquivo" .txt)
    n=$(head -n 1 "$arquivo" | tr -dc '0-9')
    log "$nome (n = $n, com pesos)"

    for rep in lista matriz; do
        dir="$RES/brutos/$nome/$rep"
        rm -rf "$dir"
        mkdir -p "$dir"

        if [ "$rep" = matriz ]; then
            # adjacencia (1 byte por celula) + matriz de pesos (8 bytes)
            necessario=$(( n * n * 9 + n * 8 ))
            disponivel=$(( $(awk '/MemAvailable/ {print $2}' /proc/meminfo) * 1024 ))
            if [ "$necessario" -gt $(( disponivel / 10 * 8 )) ]; then
                printf 'inviavel: a matriz precisaria de %d MB; memoria disponivel: %d MB\n' \
                    $(( necessario / 1048576 )) $(( disponivel / 1048576 )) > "$dir/inviavel.txt"
                log "  matriz: inviavel ($(( necessario / 1048576 )) MB)"
                continue
            fi
        fi

        log "  $rep: Q1 distancias e caminhos minimos do vertice 10"
        rodar "$dir/p2_q1_caminhos.txt" ./bin/p2_questao1 "$arquivo" "$rep" heap

        modo=ambos
        if [ "$n" -gt "$DIJKSTRA_VETOR_MAX" ]; then
            modo=heap
        fi
        log "  $rep: Q2 $K_DIJKSTRA dijkstras por modo ($modo)"
        rodar "$dir/p2_q2_tempos.txt" ./bin/p2_questao2 "$arquivo" "$rep" \
            "$K_DIJKSTRA" "$modo"
    done
done

# questao da rede de colaboracao: roda uma vez, fora do laco
if [ -n "${REDE_COLABORACAO:-}" ] && [ -n "${REDE_NOMES:-}" ]; then
    if [ -f "$REDE_COLABORACAO" ] && [ -f "$REDE_NOMES" ]; then
        log "rede de colaboracao"
        mkdir -p "$RES/brutos/colaboracao"
        rodar "$RES/brutos/colaboracao/p2_q3_colaboracao.txt" \
            ./bin/p2_questao3 "$REDE_COLABORACAO" "$REDE_NOMES" heap
    else
        log "REDE_COLABORACAO/REDE_NOMES apontam para arquivos ausentes: pulando"
    fi
fi

log "gerando $RES/tabelas.md"
python3 "Study cases/gerar_tabelas_p2.py" "$RES"
log "fim"
