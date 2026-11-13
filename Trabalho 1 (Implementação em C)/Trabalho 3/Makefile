CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I. -IDataStructures -IAlgorithms

# Parte 3 do trabalho: a MESMA biblioteca da Parte 2 (agora com DIGRAFOS:
# o parametro `direcionado` na leitura e o grafo transposto) e, em cima
# dela, o que a Parte 3 acrescentou: o Bellman-Ford com as duas
# otimizacoes de aula (vetor unico de estimativas + terminacao precoce)
# e o Floyd-Warshall com a otimizacao de memoria (uma matriz so, in
# place) - Algorithms/bellman_ford.c e Algorithms/floyd_warshall.c - os
# testes da Parte 3 (test_parte3.c) e os demos novos.
# Os alvos da Parte 1 ficam no Makefile da pasta "Trabalho 1
# (Implementação em C)"; os da Parte 2, no da pasta "Trabalho 2"; este
# Makefile so tem as partes de conteudo novo.
#
# Os programas dos ESTUDOS DE CASO da Parte 3 (secao 3 do enunciado)
# ficam em uma pasta EXTERNA a esta, como combinado com o professor - os
# alvos ainda serao acrescentados quando os estudos forem feitos.
DS  = DataStructures/node.c DataStructures/my_queue.c DataStructures/my_stack.c \
      DataStructures/heap.c
ALG = Algorithms/BFS.c Algorithms/DFS.c Algorithms/components.c \
      Algorithms/distance.c Algorithms/graph_stats.c Algorithms/dijkstra.c \
      Algorithms/bellman_ford.c Algorithms/floyd_warshall.c
LIB = $(DS) $(ALG) graph_gen.c graph_utils.c graph.c

# executaveis ficam em bin/, para nao misturar com o codigo
BIN = bin

all: $(BIN)/test_parte3 $(BIN)/bellman_ford_demo $(BIN)/floyd_warshall_demo

$(BIN):
	mkdir -p $(BIN)

$(BIN)/test_parte3: test_parte3.c $(LIB) | $(BIN)
	$(CC) $(CFLAGS) -o $@ test_parte3.c $(LIB) -lm

# demos dos modulos novos: o bellman-ford (fonte x alvo, ciclo negativo)
# e o floyd-warshall (todos os pares, com caminho reconstruido)
$(BIN)/bellman_ford_demo: $(LIB) | $(BIN)
	$(CC) $(CFLAGS) -DBELLMAN_FORD_MAIN -o $@ $(LIB) -lm

$(BIN)/floyd_warshall_demo: $(LIB) | $(BIN)
	$(CC) $(CFLAGS) -DFLOYD_WARSHALL_MAIN -o $@ $(LIB) -lm

test: test_parte3

# atalhos: constroem (se precisar) e RODAM o programa
test_parte3: $(BIN)/test_parte3
	./$(BIN)/test_parte3

bellman_ford_demo: $(BIN)/bellman_ford_demo
	./$(BIN)/bellman_ford_demo

floyd_warshall_demo: $(BIN)/floyd_warshall_demo
	./$(BIN)/floyd_warshall_demo

# ----------------------------------------------------------------------
# estudos de caso da Parte 3 (secao 3 do enunciado): pasta EXTERNA
# (a fazer, quando os estudos de caso forem implementados)
# ----------------------------------------------------------------------

clean:
	rm -rf $(BIN)

.PHONY: all test clean test_parte3 bellman_ford_demo floyd_warshall_demo
