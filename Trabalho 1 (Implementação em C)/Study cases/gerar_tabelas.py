"""Monta as tabelas dos estudos de caso a partir das saidas de rodar_estudos.sh.

uso: python3 "Study cases/gerar_tabelas.py" [pasta_de_resultados]

Le <resultados>/brutos/<grafo>/<lista|matriz>/*.txt e escreve
<resultados>/tabelas.md: uma tabela por questao, com os grafos nas linhas e as
caracteristicas nas colunas, como pede o enunciado. Tambem confere se as
questoes 4 a 7 deram a mesma resposta nas duas representacoes.
"""

import re
import sys
from pathlib import Path

RES = Path(sys.argv[1] if len(sys.argv) > 1 else "resultados")
BRUTOS = RES / "brutos"
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


def inteiro(valor):
    return TRACO if valor is None else f"{valor:,d}".replace(",", ".")


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


def pais(texto):
    """{('BFS', 1): '2042 / 8382 / 2394', ...}"""
    resultado = {}
    for busca, raiz, p10, p20, p30 in re.findall(r"^(BFS|DFS)\s+(\d+)\s+(\S+)\s+(\S+)\s+(\S+)\s*$",
                                                 texto or "", re.M):
        resultado[(busca, int(raiz))] = f"{p10} / {p20} / {p30}"
    return resultado


def respostas(grafo, rep):
    """Saidas das questoes 4 a 7 sem cabecalhos nem tempos, para comparar representacoes."""
    partes = []
    for nome in ("q4_pais.txt", "q5_distancias.txt", "q6_componentes.txt", "q7_diametro.txt"):
        texto = ler(grafo, rep, nome)
        if texto is None:
            return None
        linhas = [re.sub(r"\s*\(.*?ms\).*$", "", l) for l in texto.splitlines()
                  if not l.startswith("grafo:") and "exato" not in l]
        partes.append("\n".join(linhas))
    return partes


grafos = sorted((p.name for p in BRUTOS.iterdir() if p.is_dir()),
                key=lambda nome: [int(t) if t.isdigit() else t for t in re.split(r"(\d+)", nome)])
secoes = ["# Estudos de caso — resultados", ""]
ambiente = RES / "ambiente.txt"
if ambiente.exists():
    secoes += ["Ambiente de execução:", "", "```", ambiente.read_text(encoding="utf-8").strip(), "```", ""]

# grafos
linhas = []
for g in grafos:
    t = ler(g, "lista", "saida.txt")
    linhas.append([g, inteiro(numero(t, r"Numero de vertices: (\d+)", int)),
                   inteiro(numero(t, r"Numero de arestas: (\d+)", int)),
                   inteiro(numero(t, r"Grau minimo: (\d+)", int)),
                   inteiro(numero(t, r"Grau maximo: (\d+)", int)),
                   br(numero(t, r"Grau medio: ([\d.]+)"), 2),
                   br(numero(t, r"Mediana de grau: ([\d.]+)"), 1)])
secoes.append(tabela("Grafos (requisito 2)",
                     ["Grafo", "Vértices", "Arestas", "Grau mín.", "Grau máx.", "Grau médio", "Mediana"],
                     linhas, "Laços e arestas repetidas dos arquivos são ignorados (grafo simples)."))

# Q1
linhas = []
for g in grafos:
    lista = numero(ler(g, "lista", "q1_memoria.txt"), r"memoria do grafo \(diferenca\): ([\d.]+) MB")
    mtx = ler(g, "matriz", "q1_memoria.txt")
    linhas.append([g, br(lista),
                   inviavel(g, "matriz") or br(numero(mtx, r"memoria do grafo \(diferenca\): ([\d.]+) MB")),
                   inviavel(g, "matriz") or br(numero(mtx, r"alocado para a matriz[^:]*: ([\d.]+) MB"))])
secoes.append(tabela("Questão 1 — memória (MB)",
                     ["Grafo", "Lista", "Matriz (residente)", "Matriz (alocada)"], linhas,
                     "Memória residente do processo (VmRSS) depois de carregar menos a de antes. Na matriz, "
                     "páginas que nunca recebem uma aresta não chegam a ocupar memória física, por isso a "
                     "coluna \"alocada\" mostra o tamanho reservado (n² células de 1 byte). \"Inviável\" = a "
                     "matriz não cabe na memória disponível da máquina."))

# Q2 e Q3
linhas = []
for g in grafos:
    celulas = [g]
    for tipo in ("BFS", "DFS"):
        for rep in ("lista", "matriz"):
            t = ler(g, rep, "q2_3_buscas.txt")
            celulas.append(inviavel(g, rep) or br(numero(t, tipo + r"\s+media: ([\d.]+) ms"), 2))
    linhas.append(celulas)
buscas = numero(ambiente.read_text(encoding="utf-8") if ambiente.exists() else None,
                r"buscas por tipo \(Q2/Q3\): (\d+)", int) or 100
secoes.append(tabela("Questões 2 e 3 — tempo médio de uma busca (ms)",
                     ["Grafo", "BFS lista", "BFS matriz", "DFS lista", "DFS matriz"], linhas,
                     f"Média de {buscas} buscas de cada tipo, partindo de vértices distintos espalhados "
                     "por [1, n]. O cronômetro cobre apenas o algoritmo (sem leitura nem escrita)."))

# Q4
linhas = []
for g in grafos:
    p = pais(ler(g, "lista", "q4_pais.txt"))
    linhas.append([g] + [p.get((busca, raiz), TRACO) for busca in ("BFS", "DFS") for raiz in (1, 2, 3)])
secoes.append(tabela("Questão 4 — pai dos vértices 10 / 20 / 30",
                     ["Grafo", "BFS raiz 1", "BFS raiz 2", "BFS raiz 3", "DFS raiz 1", "DFS raiz 2", "DFS raiz 3"],
                     linhas, "\"-\" = vértice fora da componente da raiz. Vizinhos visitados em ordem "
                             "crescente de rótulo."))

# Q5
linhas = []
for g in grafos:
    t = ler(g, "lista", "q5_distancias.txt") or ""
    celulas = [g]
    for par in ("10,20", "10,30", "20,30"):
        d = numero(t, r"d\(" + par + r"\) = (-?\d+)", int)
        celulas.append(TRACO if d is None else ("sem caminho" if d == -1 else str(d)))
    linhas.append(celulas)
secoes.append(tabela("Questão 5 — distâncias", ["Grafo", "d(10, 20)", "d(10, 30)", "d(20, 30)"], linhas))

# Q6
linhas = []
for g in grafos:
    t = ler(g, "lista", "q6_componentes.txt")
    linhas.append([g, inteiro(numero(t, r"Numero de componentes conexas: (\d+)", int)),
                   inteiro(numero(t, r"Maior componente: (\d+)", int)),
                   inteiro(numero(t, r"Menor componente: (\d+)", int))])
secoes.append(tabela("Questão 6 — componentes conexas",
                     ["Grafo", "Componentes", "Maior (vértices)", "Menor (vértices)"], linhas))

# Q7
linhas = []
for g in grafos:
    t = ler(g, "lista", "q7_diametro.txt")
    exato = numero(t, r"diametro exato\s+= (\d+)", int)
    linhas.append([g, inteiro(numero(t, r"diametro aproximado = (\d+)", int)),
                   br(numero(t, r"diametro aproximado = \d+\s+\(([\d.]+) ms\)") / 1000, 2)
                   if numero(t, r"diametro aproximado = \d+\s+\(([\d.]+) ms\)") is not None else TRACO,
                   "não calculado" if exato is None else inteiro(exato),
                   br(numero(t, r"diametro exato\s+= \d+\s+\(([\d.]+) ms\)") / 1000, 1)
                   if exato is not None else TRACO])
secoes.append(tabela("Questão 7 — diâmetro",
                     ["Grafo", "Aproximado", "Tempo aprox. (s)", "Exato", "Tempo exato (s)"], linhas,
                     "Aproximado: dupla varredura (2 BFS por componente), uma cota inferior do diâmetro. "
                     "Exato: BFS a partir de cada vértice, calculado só nos grafos menores. Medido na lista."))

# consistencia entre representacoes
iguais, diferentes = [], []
for g in grafos:
    r_lista, r_matriz = respostas(g, "lista"), respostas(g, "matriz")
    if r_lista is None or r_matriz is None:
        continue
    (iguais if r_lista == r_matriz else diferentes).append(g)
nota = []
if iguais:
    nota.append("Questões 4 a 7 com respostas idênticas na lista e na matriz: " + ", ".join(iguais) + ".")
if diferentes:
    nota.append("ATENÇÃO — respostas diferentes entre lista e matriz: " + ", ".join(diferentes) + ".")
if nota:
    secoes += ["## Conferência entre representações", "", *nota, ""]

(RES / "tabelas.md").write_text("\n".join(secoes), encoding="utf-8")
print(f"escrito {RES / 'tabelas.md'}")
