CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I. -IDataStructures -IAlgorithms

# biblioteca: mesmos modulos do Python, agora separados por pasta
DS  = DataStructures/node.c DataStructures/my_queue.c DataStructures/my_stack.c
ALG = Algorithms/BFS.c Algorithms/DFS.c Algorithms/components.c \
      Algorithms/distance.c Algorithms/graph_stats.c
LIB = $(DS) $(ALG) graph_gen.c graph_utils.c graph.c

# executaveis ficam em bin/, para nao misturar com o codigo
BIN = bin

all: $(BIN)/test_parte1 $(BIN)/graph

$(BIN):
	mkdir -p $(BIN)

$(BIN)/test_parte1: test_parte1.c $(LIB) | $(BIN)
	$(CC) $(CFLAGS) -o $@ test_parte1.c $(LIB) -lm

# demo do graph.py (equivale a rodar `python graph.py`)
$(BIN)/graph: $(LIB) | $(BIN)
	$(CC) $(CFLAGS) -DGRAPH_MAIN -o $@ $(LIB)

# demos dos `if __name__ == "__main__"` de cada modulo
$(BIN)/my_queue_demo: DataStructures/my_queue.c DataStructures/node.c graph_utils.c | $(BIN)
	$(CC) $(CFLAGS) -DMY_QUEUE_MAIN -o $@ DataStructures/my_queue.c \
	    DataStructures/node.c graph_utils.c

$(BIN)/my_stack_demo: DataStructures/my_stack.c graph_utils.c | $(BIN)
	$(CC) $(CFLAGS) -DMY_STACK_MAIN -o $@ DataStructures/my_stack.c graph_utils.c

$(BIN)/components_demo: $(LIB) | $(BIN)
	$(CC) $(CFLAGS) -DCOMPONENTS_MAIN -o $@ $(LIB)

$(BIN)/distance_demo: $(LIB) | $(BIN)
	$(CC) $(CFLAGS) -DDISTANCE_MAIN -o $@ $(LIB)

$(BIN)/graph_stats_demo: $(LIB) | $(BIN)
	$(CC) $(CFLAGS) -DGRAPH_STATS_MAIN -o $@ $(LIB)

test: test_parte1

# atalhos: `make <nome>` constroi (se precisar) e RODA o programa,
# com o mesmo comportamento do `make test`
test_parte1: $(BIN)/test_parte1
	./$(BIN)/test_parte1

graph: $(BIN)/graph
	./$(BIN)/graph

my_queue_demo: $(BIN)/my_queue_demo
	./$(BIN)/my_queue_demo

my_stack_demo: $(BIN)/my_stack_demo
	./$(BIN)/my_stack_demo

components_demo: $(BIN)/components_demo
	./$(BIN)/components_demo

distance_demo: $(BIN)/distance_demo
	./$(BIN)/distance_demo

graph_stats_demo: $(BIN)/graph_stats_demo
	./$(BIN)/graph_stats_demo

# ----------------------------------------------------------------------
# estudos de caso (secao 4 do enunciado) - fontes na pasta "Study cases"
# ----------------------------------------------------------------------
CASOS = Study cases

questao1: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/questao1 "$(CASOS)/questao1_memoria.c" $(LIB)

questao2_3: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/questao2_3 "$(CASOS)/questao2_3_buscas.c" $(LIB)

questao4: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/questao4 "$(CASOS)/questao4_pais.c" $(LIB)

questao5: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/questao5 "$(CASOS)/questao5_distancias.c" $(LIB)

questao6: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/questao6 "$(CASOS)/questao6_componentes.c" $(LIB)

questao7: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/questao7 "$(CASOS)/questao7_diametro.c" $(LIB)

# requisitos 2 e 4 nos grafos do estudo de caso: estatisticas e arquivos de saida
saida: | $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/saida "$(CASOS)/saida_arquivos.c" $(LIB)

casos: questao1 questao2_3 questao4 questao5 questao6 questao7 saida

# roda todos os estudos de caso nos grafos de GRAFOS e gera resultados/tabelas.md
GRAFOS = ../Grafos
estudos: all casos
	bash "$(CASOS)/rodar_estudos.sh" "$(GRAFOS)" resultados

clean:
	rm -rf $(BIN)

.PHONY: all test clean test_parte1 graph my_queue_demo my_stack_demo \
        components_demo distance_demo graph_stats_demo \
        questao1 questao2_3 questao4 questao5 questao6 questao7 saida casos estudos
