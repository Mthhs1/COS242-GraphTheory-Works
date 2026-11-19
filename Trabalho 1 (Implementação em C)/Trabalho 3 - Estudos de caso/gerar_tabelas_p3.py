"""Monta as tabelas dos estudos de caso da Parte 3 (rodar_estudos_p3.sh).

uso: python3 gerar_tabelas_p3.py [pasta_de_resultados]

Le <resultados>/brutos/<grafo>.txt (saida do p3_estudos) e escreve
<resultados>/tabelas.md: uma tabela por questao, com os grafos nas linhas.
"""
import re
import sys
from pathlib import Path

RES = Path(sys.argv[1] if len(sys.argv) > 1 else "resultados")
BRUTOS = RES / "brutos"
TRACO = "—"


def br(valor, casas=3):
    if valor is None:
        return TRACO
    return f"{valor:,.{casas}f}".replace(",", "X").replace(".", ",").replace("X", ".")


def dist(texto):
    return "∞" if texto == "inf" else br(float(texto), 3)


def tempo(segundos):
    if segundos is None:
        return TRACO
    if segundos < 0.01:
        return br(segundos * 1000, 3) + " ms"
    return br(segundos, 3) + " s"


def ler(arquivo):
    t = arquivo.read_text(encoding="utf-8")
    r = {"nome": arquivo.stem, "texto": t}
    m = re.search(r"^n (\d+) m (\d+) pesos_negativos (\S+)", t, re.M)
    if m:
        r["n"], r["m"], r["neg"] = int(m.group(1)), int(m.group(2)), m.group(3)
    r["q1"] = {o: (d, a) for o, d, a in re.findall(r"^q1 (\d+) (\S+) \| (\d+) arestas", t, re.M)}
    r["q1_ciclo"] = "SIM" in (re.search(r"ciclo negativo que chega.*", t) or [""])[0]
    r["q2"] = {(i, j): d for i, j, d in re.findall(r"^q2 (\d+) (\d+) (\S+)", t, re.M)}
    r["q2_ciclo"] = bool(re.search(r"ciclo negativo no grafo: SIM", t))
    r["fw_inviavel"] = "floyd-warshall inviavel" in t
    for chave, padrao in (("t_bf", r"tempo bellman_ford_to ([\d.]+)"),
                          ("t_fw", r"tempo floyd_warshall ([\d.]+)"),
                          ("t_tp_bf", r"tempo todos_pares_bellman_ford ([\d.]+)"),
                          ("t_tp_jo", r"tempo todos_pares_johnson ([\d.]+)")):
        m = re.search(padrao, t)
        r[chave] = float(m.group(1)) if m else None
    r["estimado"] = "ESTIMADO" in t
    m = re.search(r"^conferencia: (.*)$", t, re.M)
    r["conferencia"] = m.group(1) if m else TRACO
    return r


def tabela(titulo, cabecalho, linhas, nota=None):
    out = [f"## {titulo}", "", "| " + " | ".join(cabecalho) + " |",
           "|" + "|".join(["---"] + ["--:"] * (len(cabecalho) - 1)) + "|"]
    out += ["| " + " | ".join(l) + " |" for l in linhas]
    if nota:
        out += ["", nota]
    return "\n".join(out) + "\n"


grafos = sorted((ler(a) for a in BRUTOS.glob("*.txt")),
                key=lambda r: [int(x) if x.isdigit() else x for x in re.split(r"(\d+)", r["nome"])])

secoes = ["# Estudos de caso da Parte 3 — resultados", ""]
amb = RES / "ambiente.txt"
if amb.exists():
    secoes += ["Ambiente de execução:", "", "```", amb.read_text(encoding="utf-8").strip(), "```", ""]

secoes.append(tabela("Grafos", ["Grafo", "Vértices", "Arestas", "Pesos negativos"],
                     [[g["nome"], br(g.get("n"), 0), br(g.get("m"), 0), g.get("neg", TRACO)] for g in grafos]))

secoes.append(tabela("Questão 1 — Bellman-Ford: distância até o vértice 100",
                     ["Grafo", "d(10, 100)", "d(20, 100)", "d(30, 100)"],
                     [[g["nome"]] + (["indefinida (ciclo negativo)"] * 3 if g["q1_ciclo"] else
                                     [f"{dist(g['q1'][o][0])} ({g['q1'][o][1]} arestas)" if o in g["q1"] else TRACO
                                      for o in ("10", "20", "30")]) for g in grafos],
                     "Entre parênteses, o número de arestas do caminho mínimo. "
                     "Com ciclo negativo que chega ao 100, as distâncias não são definidas."))

V = ["10", "20", "30", "40", "50"]
for g in grafos:
    if g["fw_inviavel"]:
        secoes.append(f"## Questão 2 — {g['nome']}\n\nFloyd-Warshall inviável (memória: 12 n² bytes).\n")
        continue
    if g["q2_ciclo"]:
        secoes.append(f"## Questão 2 — {g['nome']}\n\nO grafo tem ciclo negativo (dist[i][i] < 0 para algum i): "
                      "as distâncias não são definidas.\n")
        continue
    linhas = [[i] + [dist(g["q2"].get((i, j), "inf")) for j in V] for i in V]
    secoes.append(tabela(f"Questão 2 — Floyd-Warshall, {g['nome']}" + (" (CICLO NEGATIVO)" if g["q2_ciclo"] else ""),
                         ["de \\ para"] + V, linhas))

rodadas = re.search(r"rodadas: (\d+)", amb.read_text(encoding="utf-8")).group(1) if amb.exists() else "10"
secoes.append(tabela(f"Questão 3 — tempo médio ({rodadas} rodadas)",
                     ["Grafo", "Bellman-Ford (todos → 100)", "Floyd-Warshall"],
                     [[g["nome"], tempo(g["t_bf"]), tempo(g["t_fw"])] for g in grafos],
                     "O cronômetro cobre só o algoritmo (sem leitura nem escrita em disco). "
                     "O Bellman-Ford \"de todos para um\" inclui a construção do grafo transposto."))

secoes.append(tabela("Questão 4 — todos os pares",
                     ["Grafo", "n × Bellman-Ford", "Floyd-Warshall", "Johnson", "Conferência"],
                     [[g["nome"], tempo(g["t_tp_bf"]) + (" (est.)" if g["estimado"] else ""), tempo(g["t_fw"]),
                       tempo(g["t_tp_jo"]) + (" (est.)" if g["estimado"] else ""), g["conferencia"]] for g in grafos],
                     "n × Bellman-Ford: uma execução por fonte. Johnson: um Bellman-Ford para o potencial "
                     "e um Dijkstra com heap por fonte (otimização, além do pedido)."))

(RES / "tabelas.md").write_text("\n".join(secoes), encoding="utf-8")
print("escrito", RES / "tabelas.md")
