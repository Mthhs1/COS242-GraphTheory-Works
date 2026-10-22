"""Monta as tabelas dos estudos de caso da Parte 2 (rodar_estudos_p2.sh).

uso: python3 "Study cases/gerar_tabelas_p2.py" [pasta_de_resultados]

Le <resultados>/brutos/<grafo>/<lista|matriz>/p2_q*.txt e
<resultados>/brutos/colaboracao/p2_q3_colaboracao.txt e escreve
<resultados>/tabelas.md: uma tabela por questao, com os grafos nas linhas.
Tambem confere se a questao 1 deu a mesma resposta nas duas representacoes.
"""

import re
import sys
from pathlib import Path

RES = Path(sys.argv[1] if len(sys.argv) > 1 else "resultados")
BRUTOS = RES / "brutos"
AMBIENTE = RES / "ambiente.txt"
TRACO = "—"


def ler(grafo, rep, nome):
    caminho = BRUTOS / grafo / rep / nome
    return caminho.read_text(encoding="utf-8") if caminho.exists() else None


def numero(texto, padrao, conv=float):
    if texto is None:
        return None
    achado = re.search(padrao, texto)
    return conv(achado.group(1)) if achado else None


def br(valor, casas=1):
    if valor is None:
        return TRACO
    return f"{valor:,.{casas}f}".replace(",", "X").replace(".", ",").replace("X", ".")


def inviavel(grafo, rep):
    texto = ler(grafo, rep, "inviavel.txt")
    if texto is None:
        return None
    mb = numero(texto, r"precisaria de (\d+) MB", int)
    return f"inviável ({br(mb / 1024, 1)} GB)" if mb is not None else "inviável"


def tabela(titulo, cabecalho, linhas, nota=None):
    saida = [f"## {titulo}", "", "| " + " | ".join(cabecalho) + " |",
             "|" + "|".join(["---"] + ["--:"] * (len(cabecalho) - 1)) + "|"]
    saida += ["| " + " | ".join(linha) + " |" for linha in linhas]
    if nota:
        saida += ["", nota]
    return "\n".join(saida) + "\n"


grafos = sorted((p.name for p in BRUTOS.iterdir() if p.is_dir() and p.name != "colaboracao"),
                key=lambda nome: [int(t) if t.isdigit() else t for t in re.split(r"(\d+)", nome)])
secoes = ["# Estudos de caso da Parte 2 — resultados", ""]
if AMBIENTE.exists():
    secoes += ["Ambiente de execução:", "", "```", AMBIENTE.read_text(encoding="utf-8").strip(), "```", ""]
    k_dij = numero(AMBIENTE.read_text(encoding="utf-8"), r"dijkstras por modo \(Q2\): (\d+)", int)
else:
    k_dij = None

# Questao 1: distancia e caminho minimo do vertice 10 ate 20, 30, 40, 50 e 60
def p2_q1(grafo):
    t = ler(grafo, "lista", "p2_q1_caminhos.txt")
    if t is None:
        return None
    dists, caminhos = {}, {}
    for alvo, dist, caminho in re.findall(r"^(\d+)\s+(\S+)\s+(.*)$", t, re.M):
        dists[alvo] = "inf" if dist == "infinito" else dist
        caminhos[alvo] = caminho.replace(" -> ", " → ")
    return dists, caminhos


linhas_d, linhas_c = [], []
for g in grafos:
    resultado = p2_q1(g)
    if resultado is None:
        continue
    dists, caminhos = resultado
    linhas_d.append([g] + [dists.get(a, TRACO) for a in ("20", "30", "40", "50", "60")])
    linhas_c.append([g] + [caminhos.get(a, TRACO) for a in ("20", "30", "40", "50", "60")])
if linhas_d:
    secoes.append(tabela("Questão 1 — distância mínima do vértice 10",
                         ["Grafo", "d(10, 20)", "d(10, 30)", "d(10, 40)", "d(10, 50)", "d(10, 60)"],
                         linhas_d, '"inf" = vértice em outra componente. Dijkstra na lista de adjacência.'))
    secoes.append(tabela("Questão 1 — caminho mínimo do vértice 10",
                         ["Grafo", "→ 20", "→ 30", "→ 40", "→ 50", "→ 60"], linhas_c))

# Questao 2: tempo medio do Dijkstra com vetor e com heap
linhas = []
for g in grafos:
    t = ler(g, "lista", "p2_q2_tempos.txt")
    if t is None:
        continue
    vetor = numero(t, r"vetor media: ([\d.]+) ms")
    heap = numero(t, r"heap\s+media: ([\d.]+) ms")
    linhas.append([g,
                   TRACO if vetor is None else (br(vetor, 2) if vetor < 1000 else br(vetor / 1000, 1) + " s"),
                   TRACO if heap is None else br(heap, 2)])
if linhas:
    secoes.append(tabela("Questão 2 — tempo médio de um Dijkstra",
                         ["Grafo", "Vetor (ms)", "Heap (ms)"], linhas,
                         f"Média de {k_dij or 100} Dijkstras por modo, partindo de k vértices aleatórios "
                         "distintos (semente fixa), na lista. O cronômetro cobre apenas o algoritmo. O modo "
                         "vetor é O(n²) e só roda nos grafos menores (limite configurável em "
                         "rodar_estudos_p2.sh: DIJKSTRA_VETOR_MAX)."))

# Rede de colaboracao: roda uma vez, fora do laco por grafo
colaboracao = BRUTOS / "colaboracao" / "p2_q3_colaboracao.txt"
if colaboracao.exists():
    linhas, bloco = [], {}
    for linha in colaboracao.read_text(encoding="utf-8").splitlines():
        if linha.startswith("  distancia: "):
            bloco["dist"] = linha[len("  distancia: "):]
        elif linha.startswith("  caminho: "):
            bloco["caminho"] = linha[len("  caminho: "):].replace(" -> ", " → ")
        elif linha and not linha.startswith("  "):
            achado = re.match(r"^(.*) \(vertice (\d+)\)$", linha)
            if achado:
                if bloco:
                    linhas.append(bloco)
                bloco = {"nome": achado.group(1), "vertice": achado.group(2)}
    if bloco:
        linhas.append(bloco)
    secoes.append(tabela(
        "Rede de colaboração — a partir de Edsger W. Dijkstra",
        ["Pesquisador", "Vértice", "Distância", "Caminho mínimo"],
        [[b["nome"], b["vertice"], b.get("dist", TRACO), b.get("caminho", TRACO)] for b in linhas],
        "Peso da aresta = proximidade entre pesquisadores (inversamente proporcional ao número de "
        "artigos em coautoria); a distância é a soma dos pesos do caminho mínimo."))

# consistencia entre representacoes (a questao 1 nao depende da representacao)
iguais, diferentes = [], []
for g in grafos:
    lista, matriz = ler(g, "lista", "p2_q1_caminhos.txt"), ler(g, "matriz", "p2_q1_caminhos.txt")
    if lista is None or matriz is None:
        continue
    linhas_l = [l for l in lista.splitlines() if not l.startswith("grafo:")]
    linhas_m = [l for l in matriz.splitlines() if not l.startswith("grafo:")]
    (iguais if linhas_l == linhas_m else diferentes).append(g)
nota = []
if iguais:
    nota.append("Questão 1 com respostas idênticas na lista e na matriz: " + ", ".join(iguais) + ".")
if diferentes:
    nota.append("ATENÇÃO — respostas diferentes entre lista e matriz: " + ", ".join(diferentes) + ".")
if nota:
    secoes += ["## Conferência entre representações", "", *nota, ""]

(RES / "tabelas.md").write_text("\n".join(secoes), encoding="utf-8")
print(f"escrito {RES / 'tabelas.md'}")
