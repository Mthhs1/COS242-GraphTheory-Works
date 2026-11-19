/* Bancada do Floyd-Warshall: original x contigua x em blocos x em blocos com
   OpenMP, num digrafo aleatorio de n vertices (pesos com potencial: negativos,
   sem ciclo negativo). Confere distancias e os caminhos do pred da versao em
   blocos. uso: bancada_floyd_warshall <n> */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>

static double agora(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static unsigned long long st=88172645463325252ULL;
static unsigned long long rnd(void){st^=st<<13;st^=st>>7;st^=st<<17;return st;}

/* (a) atual: linhas separadas, testes de INF */
static void fw_a(int n,double **d,int **p){
  for(int k=0;k<n;k++)for(int i=0;i<n;i++){ if(d[i][k]==HUGE_VAL)continue; double dik=d[i][k]; double*dk=d[k]; int*pk=p[k];
    for(int j=0;j<n;j++){ if(dk[j]==HUGE_VAL)continue; double a=dik+dk[j]; if(a<d[i][j]){d[i][j]=a;p[i][j]=pk[j];}}}
}
/* (b) contigua, sem teste de INF no laco interno */
static void fw_b(int n,double *D,int *P){
  for(int k=0;k<n;k++){ const double *Dk=D+(size_t)k*n; const int *Pk=P+(size_t)k*n;
    for(int i=0;i<n;i++){ double *Di=D+(size_t)i*n; double dik=Di[k]; if(dik==HUGE_VAL)continue; int *Pi=P+(size_t)i*n;
      for(int j=0;j<n;j++){ double a=dik+Dk[j]; if(a<Di[j]){Di[j]=a;Pi[j]=Pk[j];}}}}
}
/* (c) em blocos de B: as tres fases do FW em blocos (dependente, linha/coluna, resto) */
#define B 64
static inline void bloco(int n,double *D,int *P,int kb,int ib,int jb,int nk,int ni,int nj){
  for(int k=kb;k<kb+nk;k++){ const double *Dk=D+(size_t)k*n; const int *Pk=P+(size_t)k*n;
    for(int i=ib;i<ib+ni;i++){ double *Di=D+(size_t)i*n; double dik=Di[k]; if(dik==HUGE_VAL)continue; int *Pi=P+(size_t)i*n;
      for(int j=jb;j<jb+nj;j++){ double a=dik+Dk[j]; if(a<Di[j]){Di[j]=a;Pi[j]=Pk[j];}}}}
}
static void fw_c(int n,double *D,int *P){
  for(int kb=0;kb<n;kb+=B){ int nk=n-kb<B?n-kb:B;
    bloco(n,D,P,kb,kb,kb,nk,nk,nk);
    for(int jb=0;jb<n;jb+=B){ if(jb==kb)continue; int nj=n-jb<B?n-jb:B; bloco(n,D,P,kb,kb,jb,nk,nk,nj);}
    for(int ib=0;ib<n;ib+=B){ if(ib==kb)continue; int ni=n-ib<B?n-ib:B; bloco(n,D,P,kb,ib,kb,nk,ni,nk);}
    for(int ib=0;ib<n;ib+=B){ if(ib==kb)continue; int ni=n-ib<B?n-ib:B;
      for(int jb=0;jb<n;jb+=B){ if(jb==kb)continue; int nj=n-jb<B?n-jb:B; bloco(n,D,P,kb,ib,jb,nk,ni,nj);}}
  }
}
static void fw_d(int n,double *D,int *P){
  for(int kb=0;kb<n;kb+=B){ int nk=n-kb<B?n-kb:B;
    bloco(n,D,P,kb,kb,kb,nk,nk,nk);
    #pragma omp parallel for schedule(dynamic)
    for(int jb=0;jb<n;jb+=B){ if(jb==kb)continue; int nj=n-jb<B?n-jb:B; bloco(n,D,P,kb,kb,jb,nk,nk,nj); bloco(n,D,P,kb,jb,kb,nk,nj,nk);}
    #pragma omp parallel for schedule(dynamic)
    for(int ib=0;ib<n;ib+=B){ if(ib==kb)continue; int ni=n-ib<B?n-ib:B;
      for(int jb=0;jb<n;jb+=B){ if(jb==kb)continue; int nj=n-jb<B?n-jb:B; bloco(n,D,P,kb,ib,jb,nk,ni,nj);}}
  }
}
int main(int argc,char**argv){
  int n=argc>1?atoi(argv[1]):2000; int grau=5;
  double *h=malloc(sizeof(double)*n); for(int v=0;v<n;v++)h[v]=(rnd()%8000)/1000.0;
  double *D0=malloc(sizeof(double)*n*n); int *P0=malloc(sizeof(int)*n*n);
  for(size_t x=0;x<(size_t)n*n;x++){D0[x]=HUGE_VAL;P0[x]=-1;}
  for(int i=0;i<n;i++){D0[(size_t)i*n+i]=0; for(int e=0;e<grau;e++){int j=rnd()%n; if(j==i)continue; double w=1+(rnd()%9000)/1000.0+h[i]-h[j]; if(w<D0[(size_t)i*n+j]){D0[(size_t)i*n+j]=w;P0[(size_t)i*n+j]=i;}}
    int j=(i+1)%n; double w=5+h[i]-h[j]; if(w<D0[(size_t)i*n+j]){D0[(size_t)i*n+j]=w;P0[(size_t)i*n+j]=i;}}
  double **da=malloc(sizeof(double*)*n); int **pa=malloc(sizeof(int*)*n);
  for(int i=0;i<n;i++){da[i]=malloc(sizeof(double)*n);pa[i]=malloc(sizeof(int)*n);memcpy(da[i],D0+(size_t)i*n,sizeof(double)*n);memcpy(pa[i],P0+(size_t)i*n,sizeof(int)*n);}
  double *Db=malloc(sizeof(double)*n*n); int *Pb=malloc(sizeof(int)*n*n); memcpy(Db,D0,sizeof(double)*n*n); memcpy(Pb,P0,sizeof(int)*n*n);
  double *Dc=malloc(sizeof(double)*n*n); int *Pc=malloc(sizeof(int)*n*n); memcpy(Dc,D0,sizeof(double)*n*n); memcpy(Pc,P0,sizeof(int)*n*n);
  double t=agora(); fw_a(n,da,pa); double ta=agora()-t;
  double *Dd=malloc(sizeof(double)*n*n); int *Pd=malloc(sizeof(int)*n*n); memcpy(Dd,D0,sizeof(double)*n*n); memcpy(Pd,P0,sizeof(int)*n*n);
  t=agora(); fw_d(n,Dd,Pd); double td=agora()-t; long difd=0; for(size_t x=0;x<(size_t)n*n;x++) if(fabs(Dd[x]-D0[x]*0-Dd[x])>0) difd++;
  t=agora(); fw_b(n,Db,Pb); double tb=agora()-t;
  t=agora(); fw_c(n,Dc,Pc); double tc=agora()-t;
  long difb=0,difc=0,pdc=0; for(int i=0;i<n;i++)for(int j=0;j<n;j++){ if(da[i][j]!=Db[(size_t)i*n+j])difb++; if(fabs(da[i][j]-Dc[(size_t)i*n+j])>1e-9)difc++; if(pa[i][j]!=Pc[(size_t)i*n+j])pdc++; }
  long ruins=0; for(int i=0;i<n;i++)for(int j=0;j<n;j++){ if(i==j||Dc[(size_t)i*n+j]==HUGE_VAL)continue; double soma=0; int v=j,passos=0; while(v!=i&&passos<=n){int u=Pc[(size_t)i*n+v]; double w=D0[(size_t)u*n+v]; soma+=w; v=u; passos++;} if(v!=i||fabs(soma-Dc[(size_t)i*n+j])>1e-9)ruins++; }
  printf("caminhos do pred em blocos invalidos: %ld\n",ruins);
  printf("n=%d  atual %.3fs | contigua %.3fs (%.2fx, dif %ld) | blocos %.3fs (%.2fx, dif dist %ld, dif pred %ld)\n",n,ta,tb,ta/tb,difb,tc,ta/tc,difc,pdc);
  long dd=0; for(int i=0;i<n;i++)for(int j=0;j<n;j++) if(fabs(da[i][j]-Dd[(size_t)i*n+j])>1e-9) dd++;
  printf("blocos + %d threads: %.3fs (%.2fx sobre o atual, %.2fx sobre blocos sequencial, dif dist %ld)\n",omp_get_max_threads(),td,ta/td,tc/td,dd);
  return 0;
}
