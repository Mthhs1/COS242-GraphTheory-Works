#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "graph.h"

/*
Requisito 6 do enunciado: componentes conexas.

Descobre o numero de componentes, o tamanho de cada uma e a lista de
vertices que a compoe, devolvidas em ordem DECRESCENTE de tamanho.

Custo: O(n + m). Cada vertice entra na fila uma unica vez (e marcado na
descoberta) e cada aresta e examinada duas vezes, uma na lista de cada
extremidade.

No Python a funcao devolvia uma lista de listas; aqui usamos o struct
Components (vetor de vetores + tamanhos).
*/

typedef struct Components {
    int count;        /* len(components) no Python */
    int **components; /* componentes[i][0..sizes[i]-1]: rotulos 1-based */
    int *sizes;       /* len de cada componente */
} Components;

Components connected_components(Graph *g);
void component_of(Graph *g, Components *result, int **component_id);
char *summary(Graph *g);
void Components_free(Components *components);

#endif
