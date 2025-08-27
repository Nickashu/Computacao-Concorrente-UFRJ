/* Disciplina: Computação Concorrente */
/* Profa.: Silvana Rossetto */
/* Laboratório 3 Exercício 1 */
/* Nome: Nícolas da Mota Arruda */
/* DRE: 122153341 */

/*
Programa concorrente que recebe como entrada o número de threads t e um nome de arquivo, que conterá uma dimensão n (long int), dois
vetores com floats e o resultado do produto interno entre esses vetores. O programa executa o cálculo do produto interno dos dois 
vetores dividindo a tarefa entre as t threads de forma balanceada, e ao final compara o valor calculado com o valor registrado no arquivo de entrada. 
Também será calculada a variação relativa considerando como valor de referência o resultado do cálculo sequencial.
OBS: os arquivos de entrada serão gerados pelo programa sequencial "prod_interno_seq.c"
Este código foi feito com ajuda do código "soma_vetor_conc.c" disponibilizado para o laboratório 3 
*/

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h> 
#include "timer.h"

#define VERSOES    //Descomentar o define para comparar os resultados sequenciais e concorrentes

//Vetores de elementos
float *vet1, *vet2;

//Estrutura de dados para passar argumentos para a thread
typedef struct {
    long int n;   //Tamanho dos vetores
    short int nthreads;   //Número de threads
    short int id;    //Identificador da thread
} t_args;  


//Função executada pelas threads (estratégia de divisão de tarefas: blocos de n/nthreads elementos)
void *ProdIntVetores (void *args) {
    t_args *arg = (t_args*) args;  //Argumentos da thread
    int ini, fim, fatia;  //Auxiliares para divisao do vetor em blocos
    double prod_int_local=0, *ret;  //Produto interno local

    fatia = arg->n / arg->nthreads;  //Tamanho do bloco de dados de cada thread
    ini = arg->id * fatia;  //Posicao inicial do vetor
    fim = ini + fatia;  //Posicao final do vetor
    if (arg->id == (arg->nthreads-1)) fim = arg->n;  //A ultima thread trata os elementos restantes no caso de divisao nao exata

    //Calculando o produto interno local
    for(int i=ini; i<fim; i++) {
        prod_int_local += vet1[i] * vet2[i];
    }

    //Retornando o resultado do produto interno
    ret = (double*) malloc(sizeof(double));
    if (ret!=NULL) *ret = prod_int_local;
    else printf("ERRO: malloc() dentro da thread\n");

    free(arg);  //Liberando a memória dos argumentos
    pthread_exit((void*) ret);
}

int main(int argc, char *argv[]) {
    long int n;  //Tamanho do vetor
    short int nthreads;  //Numero de threads 
    FILE *arq;  //Arquivo de entrada
    size_t ret;  //Retorno da funcao de leitura no arquivo de entrada
    double prod_int_arq;  //Produto interno registrado no arquivo
    #ifdef VERSOES
    double prod_int_seq; //Resultado do produto interno feito de forma sequencial
    #endif
    double prod_int_conc_global;   //Resultado do produto interno concorrente
    double *soma_retorno_threads;  //Auxiliar para retorno das threads
    double variacao_relativa;  //Variação relativa entre o resultado sequencial e concorrente
    double start, end, delta;   //Variáveis para controle de tempo

    pthread_t *tid_sistema;  //Vetor de identificadores das threads no sistema

    //Recebendo os valores de entrada
    if(argc < 3) { printf("Use: %s <arquivo de entrada> <numero de threads> \n", argv[0]); exit(-1); }

    nthreads = atoi(argv[2]);   //Lendo o numero de threads da entrada do usuario
    if (nthreads < 1){
        printf("Numero de threads invalido. Usando 1 thread.\n");
        nthreads = 1;  //Garantindo que o numero de threads seja pelo menos 1
    }

    //Abrindo o arquivo de entrada com os valores para serem somados
    arq = fopen(argv[1], "rb");
    if(arq==NULL) { 
        printf("--ERRO: fopen()\n"); 
        exit(-1); 
    }

    //Lendo o tamanho dos vetores
    ret = fread(&n, sizeof(long int), 1, arq);
    if(!ret) {
        fprintf(stderr, "Erro de leitura das dimensoes dos vetores no arquivo \n");
        fclose(arq);
        return 3;
    }

    if(nthreads > n) nthreads = n;  //Limitando o numero de threads ao tamanho do vetor

    //Alocando espaco de memoria e carregando os vetores de entrada
    vet1 = (float*) malloc (sizeof(float) * n);
    if(vet1==NULL) { 
        printf("--ERRO: malloc() para o vetor 1\n"); 
        fclose(arq);
        exit(-1); 
    }
    ret = fread(vet1, sizeof(float), n, arq);
    if(ret < n) {
        fprintf(stderr, "Erro de leitura dos elementos do vetor 1\n");
        free(vet1);
        fclose(arq);
        return 4;
    }
    vet2 = (float*) malloc (sizeof(float) * n);
    if(vet2==NULL) { 
        printf("--ERRO: malloc() para o vetor 2\n"); 
        free(vet1);
        fclose(arq);
        exit(-1); 
    }
    ret = fread(vet2, sizeof(float), n, arq);
    if(ret < n) {
        fprintf(stderr, "Erro de leitura dos elementos do vetor 2\n");
        free(vet1);
        free(vet2);
        fclose(arq);
        return 4;
    }

    //Alocando espaco para o vetor de identificadores das threads no sistema
    tid_sistema = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
    if(tid_sistema==NULL) { 
        printf("--ERRO: malloc()\n"); 
        free(vet1);
        free(vet2);
        fclose(arq);
        exit(-1); 
    }

    #ifdef VERSOES
    GET_TIME(start);  //Começando a contagem de tempo para o cálculo sequencial
    //Primeiramente, calculando o produto interno de forma sequencial
    prod_int_seq = 0;
    for(int t=0; t<n; t++) {
        prod_int_seq += vet1[t] * vet2[t];
    }

    GET_TIME(end);  //Terminando a contagem de tempo para o cálculo sequencial
    delta = end-start;

    printf("\n");
    printf("Prod interno sequencial = %.26lf\n", prod_int_seq);
    printf("Tempo (sequencial): %.6lf\n", delta);
    #endif

    GET_TIME(start);  //Começando a contagem de tempo para o cálculo concorrente
    //Criando as threads
    for(long int i=0; i<nthreads; i++) {
        t_args *args;
        args = (t_args*) malloc(sizeof(t_args));
        if(args==NULL) {    
            printf("--ERRO: malloc para argumentos das threads\n"); exit(-1);
        }
        args->n = n;
        args->nthreads = nthreads;
        args->id = i;
        if (pthread_create(&tid_sistema[i], NULL, ProdIntVetores, (void*) args)) {
            printf("--ERRO: pthread_create()\n"); 
            free(vet1);
            free(vet2);
            free(tid_sistema);
            fclose(arq);
            exit(-1);
        }
    }

    //Esperando todas as threads terminarem e calculando a soma total das threads
    prod_int_conc_global=0;
    for(int i=0; i<nthreads; i++) {
        if (pthread_join(tid_sistema[i], (void *) &soma_retorno_threads)) {
            printf("--ERRO: pthread_join()\n"); 
            free(vet1);
            free(vet2);
            free(tid_sistema);
            fclose(arq);
            exit(-1);
        }
        prod_int_conc_global += *soma_retorno_threads;
        free(soma_retorno_threads);
    }
    GET_TIME(end);  //Terminando a contagem de tempo para o cálculo concorrente
    delta = end-start;  //Calculando o intervalo de tempo

    //Imprimindo os resultados
    printf("\n");
    printf("Prod interno concorrente = %.26lf\n", prod_int_conc_global);
    printf("Tempo (concorrente): %.6lf\n", delta);

    //Lendo o produto interno registrado no arquivo
    ret = fread(&prod_int_arq, sizeof(double), 1, arq);
    if(!ret) {
        fprintf(stderr, "Erro de leitura do produto interno do arquivo \n");
        free(vet1);
        free(vet2);
        free(tid_sistema);
        fclose(arq);
        return 3;
    }
    printf("\nProd interno lido do arquivo = %.26lf\n", prod_int_arq);

    //Desalocando os espacos de memoria e fechando o arquivo
    free(vet1);
    free(vet2);
    free(tid_sistema);
    fclose(arq);

    //Calculando a variação relativa
    variacao_relativa = (prod_int_arq - prod_int_conc_global) / prod_int_arq;
    if (variacao_relativa < 0) variacao_relativa = -variacao_relativa;   //Será um valor absoluto

    printf("Variacao relativa = %.26f\n\n", variacao_relativa);
    return 0;
}
