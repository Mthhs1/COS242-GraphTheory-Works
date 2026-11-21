/*
O nucleo do Floyd-Warshall, escrito UMA vez e incluido DUAS vezes por
floyd_warshall.c - como um template: com TIPO = double (o padrao, exato) e
com TIPO = float (o modo economico, metade da memoria das distancias). SUF
da o sufixo dos nomes de cada instancia. Nao e um cabecalho de uso geral:
so floyd_warshall.c o inclui, com TIPO e SUF definidos.
*/

#define FW_JUNTA_(a, b) a##b
#define FW_JUNTA(a, b) FW_JUNTA_(a, b)

/* o FW classico restrito a: k em [kb, kb+nk), i em [ib, ib+ni), j em [jb, jb+nj) */
static void FW_JUNTA(fw_bloco, SUF)(TIPO **dist, int **pred, int kb, int nk,
                                     int ib, int ni, int jb, int nj) {
    const TIPO inf = (TIPO)INFINITO_PESO;
    for (int k = kb; k < kb + nk; k++) {
        const TIPO *dist_k = dist[k];
        const int *pred_k = pred[k];
        for (int i = ib; i < ib + ni; i++) {
            TIPO dik = dist[i][k];
            if (dik == inf) {
                continue; /* k nao serve de intermediario para i */
            }
            TIPO *dist_i = dist[i];
            int *pred_i = pred[i];
            for (int j = jb; j < jb + nj; j++) {
                TIPO alternativa = dik + dist_k[j]; /* inf + x = inf */
                if (alternativa < dist_i[j]) {
                    dist_i[j] = alternativa; /* no lugar: memoria O(n^2) */
                    /* o ultimo passo do caminho i->k->j e o mesmo do
                       caminho k->j: herda o predecessor */
                    pred_i[j] = pred_k[j];
                }
            }
        }
    }
}

/* inicializa dist/pred pelas arestas e roda as tres fases em blocos;
   devolve se ha ciclo negativo */
static bool FW_JUNTA(fw_rodar, SUF)(Graph *g, TIPO **dist, int **pred) {
    int n = g->n;
    const TIPO inf = (TIPO)INFINITO_PESO;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dist[i][j] = inf;
            pred[i][j] = -1; /* sem aresta direta */
        }
        dist[i][i] = 0; /* caminho vazio de i a i */
    }

    /* as arestas do arquivo: dist[i][j] = peso(i, j), pred[i][j] = i */
    for (int i = 0; i < n; i++) {
        int j;
        double peso;
        NeighborIter it = neighbor_iter(g, i);
        while (neighbor_next(&it, &j, &peso)) {
            if ((TIPO)peso < dist[i][j]) { /* entre duplicatas vale a menor: a
                                              leitura ja fundiu, isto e so defesa */
                dist[i][j] = (TIPO)peso;
                pred[i][j] = i;
            }
        }
    }

    /* as tres fases para cada faixa de k [kb, kb+nk) */
    for (int kb = 0; kb < n; kb += FW_BLOCO) {
        int nk = (n - kb < FW_BLOCO) ? n - kb : FW_BLOCO;

        /* (1) o bloco da diagonal */
        FW_JUNTA(fw_bloco, SUF)(dist, pred, kb, nk, kb, nk, kb, nk);

        /* (2) a linha e a coluna de blocos da faixa k */
        for (int b = 0; b < n; b += FW_BLOCO) {
            if (b == kb) {
                continue;
            }
            int nb = (n - b < FW_BLOCO) ? n - b : FW_BLOCO;
            FW_JUNTA(fw_bloco, SUF)(dist, pred, kb, nk, kb, nk, b, nb); /* linha  */
            FW_JUNTA(fw_bloco, SUF)(dist, pred, kb, nk, b, nb, kb, nk); /* coluna */
        }

        /* (3) todo o resto */
        for (int ib = 0; ib < n; ib += FW_BLOCO) {
            if (ib == kb) {
                continue;
            }
            int ni = (n - ib < FW_BLOCO) ? n - ib : FW_BLOCO;
            for (int jb = 0; jb < n; jb += FW_BLOCO) {
                if (jb == kb) {
                    continue;
                }
                int nj = (n - jb < FW_BLOCO) ? n - jb : FW_BLOCO;
                FW_JUNTA(fw_bloco, SUF)(dist, pred, kb, nk, ib, ni, jb, nj);
            }
        }

        /* PARADA ANTECIPADA: dist[i][i] < 0 e um passeio fechado negativo -
           o ciclo negativo ja esta provado, e os valores com ciclo nao sao
           confiaveis de qualquer jeito. Conferir a diagonal custa O(n) por
           faixa de k (n^2 / 64 no total), nada perto dos n^3 do algoritmo. */
        for (int i = 0; i < n; i++) {
            if (dist[i][i] < 0) {
                return true;
            }
        }
    }
    return false;
}

#undef FW_JUNTA
#undef FW_JUNTA_
