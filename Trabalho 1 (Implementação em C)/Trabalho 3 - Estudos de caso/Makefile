CC = gcc
# -O3 -march=native: deixa o compilador vetorizar o laco interno do Floyd-Warshall
# (AVX). Os binarios ficam presos a esta CPU - recompile em outra maquina.
CFLAGS = -std=c11 -Wall -Wextra -O3 -march=native

# Estudos de caso da Parte 3 (secao 3 do trabalho-P3), numa pasta EXTERNA a
# da biblioteca, como combinado com o professor. Os programas usam a
# biblioteca de "../Trabalho 3" sem copia-la.
LIBDIR = ../Trabalho 3
INC = -I"$(LIBDIR)" -I"$(LIBDIR)/DataStructures" -I"$(LIBDIR)/Algorithms"
LIB = "$(LIBDIR)"/DataStructures/node.c "$(LIBDIR)"/DataStructures/my_queue.c \
      "$(LIBDIR)"/DataStructures/my_stack.c "$(LIBDIR)"/DataStructures/heap.c \
      "$(LIBDIR)"/Algorithms/BFS.c "$(LIBDIR)"/Algorithms/DFS.c \
      "$(LIBDIR)"/Algorithms/components.c "$(LIBDIR)"/Algorithms/distance.c \
      "$(LIBDIR)"/Algorithms/graph_stats.c "$(LIBDIR)"/Algorithms/dijkstra.c \
      "$(LIBDIR)"/Algorithms/bellman_ford.c "$(LIBDIR)"/Algorithms/floyd_warshall.c \
      "$(LIBDIR)"/Algorithms/johnson.c \
      "$(LIBDIR)"/graph_gen.c "$(LIBDIR)"/graph_utils.c "$(LIBDIR)"/graph.c

BIN = bin
GRAFOS = ../../Grafos/P3

all: p3_estudos

p3_estudos:
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(INC) -o $(BIN)/p3_estudos p3_estudos.c $(LIB) -lm

# roda as quatro questoes em todos os grafos de GRAFOS e monta
# resultados/tabelas.md (ver rodar_estudos_p3.sh)
estudos: p3_estudos
	bash rodar_estudos_p3.sh "$(GRAFOS)" resultados

# grafos sinteticos (dígrafos com pesos negativos, sem ciclo negativo, e um
# com ciclo) para testar o pipeline enquanto os grafos do site nao saem
sinteticos:
	python3 gerar_digrafos_teste.py "$(GRAFOS)-sinteticos"

clean:
	rm -rf $(BIN)

.PHONY: all p3_estudos estudos sinteticos clean
