"""Digrafo em grade L x L (vizinhos nas 4 direcoes), pesos com potencial: caminhos longos
(diametro ~2L), pesos negativos, sem ciclo negativo. uso: grade.py L saida"""
import random, sys
L = int(sys.argv[1]); rnd = random.Random(7)
n = L * L
p = [0.0] + [rnd.uniform(0, 8) for _ in range(n)]
with open(sys.argv[2], "w") as f:
    f.write(f"{n}\n")
    for r in range(L):
        for c in range(L):
            u = r * L + c + 1
            for dr, dc in ((0, 1), (1, 0), (0, -1), (-1, 0)):
                rr, cc = r + dr, c + dc
                if 0 <= rr < L and 0 <= cc < L:
                    v = rr * L + cc + 1
                    f.write(f"{u} {v} {rnd.uniform(1, 10) + p[u] - p[v]:.3f}\n")
