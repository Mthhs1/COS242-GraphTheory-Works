CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I. -IDataStructures -IAlgorithms

# Parte 2 do trabalho: a MESMA biblioteca da Parte 1 (agora com pesos nas
# arestas) e, em cima dela, o que a Parte 2 acrescentou: o heap com
# decrease-key (DataStructures/heap.c), o Dijkstra com vetor e com heap
# (Algorithms/dijkstra.c), os testes da Parte 2 (test_parte2.c) e os
# programas dos estudos de caso da Parte 2 (Study cases/p2_questao*.c).
# Os alvos da Parte 1 ficam no Makefile da pasta "Trabalho 1
# (Implementação em C)"; este Makefile so tem as partes de conteudo novo.
DS  = DataStructures/node.c DataStructures/my_queue.c DataStructures/my_stack.c \
      DataStructures/heap.c
ALG = Algorithms/BFS.c Algorithms/DFS.c Algorithms/components.c \
      Algorithms/distance.c Algorithms/graph_stats.c Algorithms/dijkstra.c
LIB = $(DS) $(ALG) graph_gen.c graph_utils.c graph.c

# executaveis ficam em bin/, para nao misturar com o codigo
BIN = bin

all: $(BIN)/test_parte2 $(BIN)/heap_demo $(BIN)/dijkstra_demo

$(BIN):
	mkdir -p $(BIN)

$(BIN)/test_parte2: test_parte2.c $(LIB) | $(BIN)
	$(CC) $(CFLAGS) -o $@ test_parte2.c $(LIB) -lm

# demos dos modulos novos: o heap (decrease-key) e o dijkstra (vetor x heap)
$(BIN)/heap_demo: DataStructures/heap.c graph_utils.c | $(BIN)
	$(CC) $(CFLAGS) -DHEAP_MAIN -o $@ DataStructures/heap.c graph_utils.c

$(BIN)/dijkstra_demo: $(LIB) | $(BIN)
	$(CC) $(CFLAGS) -DDIJKSTRA_MAIN -o $@ $(LIB)

test: test_parte2

# atalhos: constroem (se precisar) e RODAM o programa
test_parte2: $(BIN)/test_parte2
	./$(BIN)/test_parte2

heap_demo: $(BIN)/heap_demo
	./$(BIN)/heap_demo

dijkstra_demo: $(BIN)/dijkstra_demo
	./$(BIN)/dijkstra_demo

# ----------------------------------------------------------------------
# estudos de caso da Parte 2 (secao 3 do enunciado) - fontes na pasta
# "Study cases"; grafos com pesos na pasta Grafos, na raiz do repositorio
# ----------------------------------------------------------------------
CASOS = Study cases

p2_questao1: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/p2_questao1 "$(CASOS)/p2_questao1_caminhos.c" $(LIB)

p2_questao2: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/p2_questao2 "$(CASOS)/p2_questao2_tempos.c" $(LIB)

p2_questao3: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/p2_questao3 "$(CASOS)/p2_questao3_colaboracao.c" $(LIB)

# o preco da "saida facil" da dica do professor: heap com decrease-key x
# reinsercao, nos mesmos vertices iniciais da questao 2
p2_saida_facil: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/p2_saida_facil "$(CASOS)/p2_preco_saida_facil.c" $(LIB)

casos_p2: p2_questao1 p2_questao2 p2_questao3 p2_saida_facil

# roda os estudos de caso da Parte 2 (so nos grafos COM PESOS de GRAFOS) e
# gera resultados/tabelas.md; a questao da rede de colaboracao roda quando
# as variaveis REDE_COLABORACAO e REDE_NOMES apontam para os arquivos
GRAFOS = ../Grafos
estudos: all casos_p2
	bash "$(CASOS)/rodar_estudos_p2.sh" "$(GRAFOS)" resultados

clean:
	rm -rf $(BIN)

.PHONY: all test clean test_parte2 heap_demo dijkstra_demo \
        p2_questao1 p2_questao2 p2_questao3 p2_saida_facil casos_p2 estudos
