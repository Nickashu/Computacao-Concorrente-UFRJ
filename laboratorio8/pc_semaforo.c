/* Disciplina: Computação Concorrente */
/* Profa.: Silvana Rossetto */
/* Laboratório 8 - Exercício 3 */
/* Nome: Nícolas da Mota Arruda */
/* DRE: 122153341 */

/*
Programa que implementa o problema do produtor/consumidor usando apenas semáforos para sincronização por condição e 
exclusão mútua. Teremos apenas 1 produtor e vários consumidores. O produtor gera uma sequência de números inteiros de 1 a N (N é um valor 
recebido como entrada) e os insere em um buffer compartilhado de tamanho M, com M << N. Os consumidores retiram os números do buffer de forma 
concorrente e verificam se são primos. Cada consumidor deve contar quantos números primos encontrou. Ao final, o programa imprime o máximo 
de números primos encontrados por um único consumidor.
OBS: Este código foi feito com ajuda do código "pc.c" disponibilizado para o laboratório 7
*/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <math.h>

#define CONSUMIDORES 5    //Número de threads consumidoras
//#define LOG   //Se não for comentado, imprime mensagens de log (itens produzidos/consumidos)

sem_t bufferCheio, bufferVazio;  //Semáforos para sincronização por condição
sem_t mutex;  //Semáforo geral de sincronização por exclusão mútua

int *buffer;  //Buffer compartilhado
int acabou_producao = 0; //Variável para indicar que o produtor terminou de produzir

long long itens_produzidos = 0, itens_consumidos = 0;   //Variáveis para checagem de corretude

typedef struct {
    long long int N;  //Tamanho da sequência a ser produzida
    int M;  //Tamanho do buffer
} t_arg_prod;

typedef struct {
    int id;  //ID do consumidor
    int M;  //Tamanho do buffer
} t_arg_cons;

int ehPrimo(long long int n) {
    int i;
    if (n<=1) return 0;
    if (n==2) return 1;
    if (n%2==0) return 0;
    for (i=3; i<sqrt(n)+1; i+=2)
        if(n%i==0) return 0;
    return 1;
}

//Função para inserir elementos no buffer
void Insere (int qtd_insercoes, long long int N) {
    static long long int ultimo_item = 1; //Último item produzido
    sem_wait(&bufferVazio);  //Aguarda até que o buffer esteja vazio para inserir
    sem_wait(&mutex);
    for(int i=0; i<qtd_insercoes; i++) {
        buffer[i] = ultimo_item;
        #ifdef LOG
        printf("Prod: inseriu %lld\n", ultimo_item);
        #endif
        if (ultimo_item >= N)  //Se já produziu todos os itens, indica que a produção acabou
            acabou_producao = 1;
        ultimo_item++;
        itens_produzidos++;
    } 
    for(int i=0; i<qtd_insercoes; i++){   //Fazendo post para cada item inserido
        sem_post(&bufferCheio);
    }
    sem_post(&mutex);
}

//Função para retirar um elemento do buffer
int Retira (int id, int tam_buffer) {
    int item;
    static int out=0;
    sem_wait(&bufferCheio);  //Aguarda até que o buffer esteja cheio para retirar
    sem_wait(&mutex);   //Exclusão mútua entre consumidores

    //Se acordou mas a produção terminou e não há mais itens, retorna -1. Acontece quando o main dá os posts finais
    if (acabou_producao && buffer[out] == 0) {
        sem_post(&mutex);
        return -1;
    }
    item = buffer[out];
    buffer[out] = 0;
    out = (out + 1) % tam_buffer;
    #ifdef LOG
    printf("Cons[%d]: retirou %d\n", id, item);
    #endif

    if (out == 0) sem_post(&bufferVazio);  //Sinaliza que o buffer ficou vazio
    if (item != 0) itens_consumidos++;  //Contabiliza o item consumido

    sem_post(&mutex);
    return item;
}

//Produtor 
void *produtor(void * arg) {
    t_arg_prod *args = (t_arg_prod *) arg;
    long long int N = args->N, i=1;
    int qtd_insercoes, M = args->M;
    free(arg);

    while(i <= N) {
        if(i + M > N){
            qtd_insercoes = N - i + 1;  //Se for para inserir mais do que o necessário, insere apenas o que falta
            i = N + 1;
        }
        else{
            qtd_insercoes = M;
            i += M;
        }
        Insere(qtd_insercoes, N);  //Insere qtd_insercoes itens no buffer
    }
    //printf("Produtor terminou!\n");
    pthread_exit(NULL);
}

//Consumidor
void *consumidor(void * arg) {
    t_arg_cons *args = (t_arg_cons *) arg;
    int item, tam_buffer = args->M, id = args->id, contPrimos = 0, *ret;
    free(arg);
    while(1) {
        item = Retira(id, tam_buffer);   //Retira um item
        if (item == -1) break;    //Se o produtor já terminou e o buffer está vazio, termina a thread
        if (ehPrimo(item)) contPrimos++;
    }

    //Retornando a quantidade de primos encontrados por esta thread
    ret = (int*) malloc(sizeof(int));
    if (ret!=NULL) *ret = contPrimos;
    else printf("ERRO: malloc() para retorno da thread %d\n", id);

    //printf("Consumidor %d terminou!\n", id);
    pthread_exit((void*) ret);
}

void desaloca_memoria(pthread_t *tid, int *buffer) {
    free(tid);
    free(buffer);
    sem_destroy(&mutex);
    sem_destroy(&bufferCheio);
    sem_destroy(&bufferVazio);
}

int main(int argc, char **argv) {
    int n_threads = CONSUMIDORES + 1;  //Número total de threads (produtor + consumidores)
    pthread_t *tid = (pthread_t *) malloc(sizeof(pthread_t) * n_threads);
    if(tid==NULL) { 
        printf("--ERRO: malloc()\n");
        exit(1);
    }

    int i, M;
    long long int N;
    //Recebendo os valores de entrada
    if(argc < 3) { printf("Use: %s <N (tamanho da sequencia)> <M (tamanho do buffer)> \n", argv[0]); exit(-1); }
    N = atoi(argv[1]);   //Lendo o tamanho da sequencia a ser produzida
    M = atoi(argv[2]);   //Lendo o tamanho do buffer
    if (M >= N) {
        printf("Tamanho do buffer deve ser menor que o tamanho da sequencia\n");
        free(tid);
        exit(1);
    }

    buffer = malloc(sizeof(int) * M);
    if (buffer == NULL) {
        printf("Erro na alocacao do buffer\n");
        free(tid);
        exit(1);
    }

    //Inicia os semáforos
    sem_init(&mutex, 0, 1);
    sem_init(&bufferCheio, 0, 0);
    sem_init(&bufferVazio, 0, 1);

    printf("Iniciando com N=%lld, M=%d, Consumidores=%d\n", N, M, CONSUMIDORES);
    
    //Inicia o produtor
    t_arg_prod *arg = (t_arg_prod *) malloc(sizeof(t_arg_prod));
    if (arg == NULL) {
        printf("Erro na alocacao dos argumentos do produtor\n");
        desaloca_memoria(tid, buffer);
        exit(1);
    }
    arg->N = N;
    arg->M = M;
    if (pthread_create(&tid[0], NULL, produtor, (void *) arg)) {
        printf("Erro na criacao do thread produtor\n");
        desaloca_memoria(tid, buffer);
        exit(1);
    }

    //Inicia os consumidores
    for(i=0; i<CONSUMIDORES; i++) {
        int id = i + 1;
        t_arg_cons *arg = (t_arg_cons *) malloc(sizeof(t_arg_cons));
        if (arg == NULL) {
            printf("Erro na alocacao dos argumentos do consumidor %d\n", id);
            desaloca_memoria(tid, buffer);
            exit(1);
        }
        arg->id = id;
        arg->M = M;
        if (pthread_create(&tid[i + 1], NULL, consumidor, (void *) arg)) {
            printf("Erro na criacao do thread consumidor\n");
            desaloca_memoria(tid, buffer);
            exit(1);
        }
    }

    if(pthread_join(tid[0], NULL)){  //Aguarda o produtor terminar
        printf("--ERRO: pthread_join()\n");
        desaloca_memoria(tid, buffer);
        exit(1);
    }

    //Depois que o produtor termina, todos os consumidores serão acordados
    for (i = 0; i < CONSUMIDORES; i++) {
        sem_post(&bufferCheio);
    }

    int *qtd_primos_thread, qtd_primos_total = 0, max_primos=0, id_thread_vencedora=0;

    //Aguarda os consumidores terminarem
    for (i = 1; i < n_threads; i++) {
        if (pthread_join(tid[i], (void *) &qtd_primos_thread)) {
            printf("--ERRO: pthread_join()\n"); 
            desaloca_memoria(tid, buffer);
            exit(1);
        }
        qtd_primos_total += *qtd_primos_thread;
        if(*qtd_primos_thread > max_primos) {
            max_primos = *qtd_primos_thread;
            id_thread_vencedora = i;
        }
        printf("Consumidor %d encontrou %d primos\n", i, *qtd_primos_thread);
    }

    printf("Itens produzidos: %lld\n", itens_produzidos);
    printf("Itens consumidos: %lld\n", itens_consumidos);
    printf("Total de primos encontrados: %d\n", qtd_primos_total);
    printf("Maximo de primos encontrados por uma thread: %d (Consumidor %d)\n", max_primos, id_thread_vencedora);

    desaloca_memoria(tid, buffer);
    printf ("FIM\n");
}  