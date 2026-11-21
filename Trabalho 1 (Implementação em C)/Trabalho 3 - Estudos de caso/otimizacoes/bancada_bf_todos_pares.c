/* Bancada do Bellman-Ford (atual, varredura alternada, SPFA) e de todos os
   pares em paralelo (n x BF e Johnson com OpenMP), fora da biblioteca.
   uso: bancada_bf_todos_pares <grafo.txt> [amostra_de_fontes] */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include "graph.h"
#include "graph_utils.h"
#include "bellman_ford.h"
#include "floyd_warshall.h"
#include "johnson.h"

static double agora(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static long relax;

/* variante 1: atual (Gauss-Seidel crescente, so ativos) */
static int bf_atual(Graph *g,int s,double *d){
  int n=g->n; char *at=calloc(n,1); for(int v=0;v<n;v++)d[v]=HUGE_VAL; d[s]=0; at[s]=1; int na=1,p=0;
  for(int i=1;i<=n-1&&na>0;i++){p=i; for(int u=0;u<n;u++){ if(!at[u])continue; at[u]=0;na--; int v;double w; NeighborIter it=neighbor_iter(g,u);
      while(neighbor_next(&it,&v,&w)){relax++; double a=d[u]+w; if(a<d[v]){d[v]=a; if(!at[v]){at[v]=1;na++;}}}}}
  free(at); return p;
}
/* variante 2: varredura alternada (crescente / decrescente) */
static int bf_alt(Graph *g,int s,double *d){
  int n=g->n; char *at=calloc(n,1); for(int v=0;v<n;v++)d[v]=HUGE_VAL; d[s]=0; at[s]=1; int na=1,p=0;
  for(int i=1;i<=n-1&&na>0;i++){p=i; int ini=(i&1)?0:n-1, fim=(i&1)?n:-1, passo=(i&1)?1:-1;
    for(int u=ini;u!=fim;u+=passo){ if(!at[u])continue; at[u]=0;na--; int v;double w; NeighborIter it=neighbor_iter(g,u);
      while(neighbor_next(&it,&v,&w)){relax++; double a=d[u]+w; if(a<d[v]){d[v]=a; if(!at[v]){at[v]=1;na++;}}}}}
  free(at); return p;
}
/* variante 3: SPFA (fila FIFO de ativos) */
static int bf_spfa(Graph *g,int s,double *d){
  int n=g->n; char *na_fila=calloc(n,1); int *q=malloc(sizeof(int)*(n+1)); int h=0,t=0,cnt=0;
  for(int v=0;v<n;v++)d[v]=HUGE_VAL; d[s]=0; q[t++]=s; na_fila[s]=1; cnt=1;
  while(cnt>0){ int u=q[h]; h=(h+1)%(n+1); cnt--; na_fila[u]=0; int v;double w; NeighborIter it=neighbor_iter(g,u);
    while(neighbor_next(&it,&v,&w)){relax++; double a=d[u]+w; if(a<d[v]){d[v]=a; if(!na_fila[v]){na_fila[v]=1;q[t]=v;t=(t+1)%(n+1);cnt++;}}}}
  free(na_fila);free(q); return 0;
}

int main(int argc,char**argv){
  Graph *g=Graph_create(argv[1],true,true); int n=g->n;
  int am=argc>2?atoi(argv[2]):n; if(am>n)am=n;
  double *d=malloc(sizeof(double)*n), *ref=malloc(sizeof(double)*n);
  printf("grafo %s n=%d fontes=%d threads=%d\n",argv[1],n,am,omp_get_max_threads());
  /* --- Bellman-Ford sequencial, 3 variantes, mesmas fontes --- */
  const char *nomes[3]={"atual (crescente)","alternada","SPFA (fila)"};
  int (*f[3])(Graph*,int,double*)={bf_atual,bf_alt,bf_spfa};
  for(int k=0;k<3;k++){ relax=0; long passos=0; double maxdif=0; double t0=agora();
    for(int i=0;i<am;i++){ int s=(int)((long)i*n/am); passos+=f[k](g,s,d);
      if(k==0) continue; bf_atual(g,s,ref); for(int v=0;v<n;v++){ if(isinf(ref[v])!=isinf(d[v])) maxdif=1e300; else if(!isinf(ref[v])) maxdif=fmax(maxdif,fabs(ref[v]-d[v]));} }
    double dt=agora()-t0; if(k>0){ /* o tempo inclui a conferencia: refaz sem ela */ relax=0; passos=0; t0=agora(); for(int i=0;i<am;i++){int s=(int)((long)i*n/am); passos+=f[k](g,s,d);} dt=agora()-t0; }
    printf("BF %-18s %8.3f s (extrap. n: %8.3f s) | relaxamentos/fonte %10.0f | passadas/fonte %6.1f | dif %.1e\n",
           nomes[k],dt,dt*n/am,(double)relax/am,(double)passos/am,maxdif);
  }
  /* --- todos os pares em paralelo: n x BF e Johnson --- */
  int T=omp_get_max_threads();
  double t0=agora();
  #pragma omp parallel
  { double *dd=malloc(sizeof(double)*n);
    #pragma omp for schedule(dynamic,16)
    for(int i=0;i<am;i++){ int s=(int)((long)i*n/am); BellmanFordResult *r=bellman_ford(g,s); BellmanFordResult_free(r);} free(dd);}
  double tbp=agora()-t0;
  t0=agora(); for(int i=0;i<am;i++){int s=(int)((long)i*n/am); BellmanFordResult *r=bellman_ford(g,s); BellmanFordResult_free(r);} double tbs=agora()-t0;
  t0=agora(); Johnson *j=johnson_preparar(g); double tpot=agora()-t0;
  t0=agora(); for(int i=0;i<am;i++){int s=(int)((long)i*n/am); DijkstraResult *r=johnson_fonte(j,g,s); DijkstraResult_free(r);} double tjs=agora()-t0;
  t0=agora();
  #pragma omp parallel for schedule(dynamic,16)
  for(int i=0;i<am;i++){int s=(int)((long)i*n/am); DijkstraResult *r=johnson_fonte(j,g,s); DijkstraResult_free(r);}
  double tjp=agora()-t0; Johnson_free(j);
  printf("todos os pares (extrap. n) | n x BF: seq %.3f s, %d threads %.3f s (%.1fx) | Johnson: seq %.3f s, par %.3f s (%.1fx)\n",
         tbs*n/am,T,tbp*n/am,tbs/tbp,tpot+tjs*n/am,tpot+tjp*n/am,(tpot+tjs*n/am)/(tpot+tjp*n/am));
  free(d);free(ref); Graph_destroy(g); return 0;
}
