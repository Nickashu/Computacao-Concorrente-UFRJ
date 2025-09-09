/* Disciplina: Computação Concorrente */
/* Profa.: Silvana Rossetto */
/* Laboratório 5 - Exercício 2 */
/* Nome: Nícolas da Mota Arruda */
/* DRE: 122153341 */

/*
Modificação do programa "soma-lock-atom.c" visto no laboratório 4. Na versão original, uma thread de log imprime o valor da variável 
compartilhada 'soma' sempre que ela é múltiplo de 10. Porém, como a leitura de 'soma' não era atômica, poderia ocorrer de a thread de log ler 
um valor de 'soma' que é múltiplo de 10, mas antes que ela imprima esse valor, outra thread incrementa 'soma', fazendo com que o valor impresso 
não seja mais múltiplo de 10. Também não havia garantia de que todos os múltiplos de 10 que 'soma' poderia passar a valer eram impressos.
Na versão atual, o código será modificado para que a leitura de 'soma' seja atômica, ou seja, que nenhuma outra thread possa modificar 'soma' enquanto 
estamos lendo seu valor, e, usando variáveis de condição, faremos com que a thread de log imprima todos os valores de 'soma' que sejam múltiplos de 1000.
OBS: Este código foi feito com ajuda do código "soma_lock_atom.c" disponibilizado para o laboratório 4
*/

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

long int soma = 0; //variavel compartilhada entre as threads
pthread_mutex_t mutex; //variavel de lock para exclusao mutua
pthread_cond_t cond; //variavel de condicao para sincronizacao por condicao

int liberado_para_imprimir = 0;   //Variável de controle para indicar se a thread de log pode imprimir o valor de 'soma'

void *ExecutaTarefa (void *arg) {
  long int id = (long int) arg;
  printf("Thread : %ld esta executando...\n", id);

  for (int i=0; i<100000; i++) {
    pthread_mutex_lock(&mutex);
    while (liberado_para_imprimir){   //Enquanto a thread de log não tiver imprimido o múltiplo anterior, a execução das threads de tarefa será pausada
      pthread_cond_signal(&cond);
      pthread_cond_wait(&cond, &mutex);
    }
    soma++;  //Incrementa a variavel compartilhada
    if(!(soma % 1000)){  //Se for múltiplo de 1000, a execução da tarefa será pausada para que a thread de log possa imprimir o valor de 'soma'
      liberado_para_imprimir = 1;
      while(liberado_para_imprimir) { //Espera até a impressão
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond, &mutex);
      }
    }
    pthread_mutex_unlock(&mutex);
  }
  printf("Thread : %ld terminou!\n", id);
  pthread_exit(NULL);
}

void *extra (void *arg) {
  printf("Extra : esta executando...\n");
  long int numThreads = (long int) arg;    //Passando o número de threads de tarefa para a thread de log
  while (1) {
    pthread_mutex_lock(&mutex);  //OBS: Apenas usar o mutex, sem a condição, não garante que todos os múltiplos de 1000 serão impressos, mas garante que apenas múltiplos de 1000 serão impressos
    while(!liberado_para_imprimir) {
      pthread_cond_wait(&cond, &mutex);  //Vai esperar até que possa imprimir
    }
    printf("soma = %ld \n", soma);
    liberado_para_imprimir = 0;  //Liberando as threads de tarefa
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    if (soma >= 100000 * numThreads) break;   //Quando 'soma' atingir o valor máximo possível, a thread de log termina sua execução
  }
  printf("Extra : terminou!\n");
  pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
  pthread_t *tid; //identificadores das threads no sistema
  long int nthreads; //qtde de threads (passada linha de comando)

  //--le e avalia os parametros de entrada
  if(argc<2) {
    printf("Digite: %s <numero de threads>\n", argv[0]);
    return 1;
  }
  nthreads = atoi(argv[1]);

  //--aloca as estruturas
  tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
  if(tid==NULL) {puts("ERRO--malloc"); return 2;}

  /* Inicializa o mutex (lock de exclusao mutua) e a variavel de condicao */
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init (&cond, NULL);

  //--cria as threads
  for(long int t=0; t<nthreads; t++) {
    if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  //--cria thread de log
  if (pthread_create(&tid[nthreads], NULL, extra, (void *)nthreads)) {
    printf("--ERRO: pthread_create()\n"); exit(-1);
  }

  //--espera todas as threads terminarem
  for (int t=0; t<nthreads+1; t++) {
    if (pthread_join(tid[t], NULL)) {
      printf("--ERRO: pthread_join() \n"); 
      exit(-1); 
    } 
  } 

  /* Desaloca variaveis e termina */
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
  printf("Valor de 'soma' = %ld\n", soma);
  return 0;
}