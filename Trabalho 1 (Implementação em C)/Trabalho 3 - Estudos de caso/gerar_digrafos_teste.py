"""Digrafos sinteticos para testar os estudos de caso da Parte 3.

uso: python3 gerar_digrafos_teste.py <pasta> [n1,n2,...]

Pesos negativos SEM ciclo negativo, pelo truque do potencial: cada vertice
ganha um p(v) aleatorio e a aresta u->v recebe base(u, v) + p(u) - p(v), com
base > 0. Num ciclo os p se cancelam e sobra a soma das bases, positiva -
nenhum ciclo negativo - mas muitas arestas individuais ficam negativas.
Cada grafo garante um caminho 10 -> 20 -> ... (em anel), para as questoes
terem distancias finitas. O ultimo arquivo tem um ciclo negativo de
proposito (para testar a deteccao).
"""
import random
import sys
from pathlib import Path


def gerar(caminho, n, grau, semente, ciclo_negativo=False):
    rnd = random.Random(semente)
    p = [rnd.uniform(0, 8) for _ in range(n + 1)]
    arestas = set()
    # um anel 1 -> 2 -> ... -> n -> 1 garante conexidade forte
    for v in range(1, n + 1):
        arestas.add((v, v % n + 1))
    alvo = n * grau
    while len(arestas) < alvo:
        u, v = rnd.randint(1, n), rnd.randint(1, n)
        if u != v:
            arestas.add((u, v))
    with open(caminho, "w") as f:
        f.write(f"{n}\n")
        for u, v in sorted(arestas):
            w = rnd.uniform(1, 10) + p[u] - p[v]
            f.write(f"{u} {v} {w:.3f}\n")
        if ciclo_negativo:
            # um triangulo 40 -> 41 -> 42 -> 40 com soma -3 (arestas novas)
            f.write("40 41 -1\n41 42 -1\n42 40 -1\n")


def main():
    pasta = Path(sys.argv[1] if len(sys.argv) > 1 else "sinteticos")
    pasta.mkdir(parents=True, exist_ok=True)
    tamanhos = [int(x) for x in sys.argv[2].split(",")] if len(sys.argv) > 2 else [200, 1000, 3000]
    for i, n in enumerate(tamanhos, 1):
        gerar(pasta / f"sintetico_{i}.txt", n, 5, 100 + i)
        print(pasta / f"sintetico_{i}.txt", n)
    gerar(pasta / "sintetico_ciclo_negativo.txt", 200, 5, 999, ciclo_negativo=True)
    print(pasta / "sintetico_ciclo_negativo.txt", 200)


if __name__ == "__main__":
    main()
