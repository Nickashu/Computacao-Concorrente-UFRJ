/* Disciplina: Computação Concorrente */
/* Profa.: Silvana Rossetto */
/* Laboratório 3 Exercício 1 */
/* Nome: Nícolas da Mota Arruda */
/* DRE: 122153341 */

/*
Programa sequencial que gera dois vetores de entrada (float) de dimensão n (long int), com valores randômicos, calcula 
o produto interno desses dois vetores e escreve a dimensão n, os dois vetores e o resultado encontrado em um arquivo binário.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 1000  //Valor máximo de um elemento do vetor

#define LOG    //Descomentar o define caso deseje imprimir uma versao do vetor gerado no formato texto

int main(int argc, char*argv[]) {
   float *vetor1, *vetor2;    //Vetores que serão gerados
   long int n;  //Quantidade de elementos dos vetores
   float elem;  //Valor gerado para incluir nos vetores de forma aleatória
   double prod_interno = 0;   //Produto interno dos vetores
   int fator=1;      //Fator multiplicador para gerar números positivos e negativos alternados nos vetores
   FILE * descritorArquivo; //Descritor do arquivo de saída
   size_t ret;    //Retorno da funcao de escrita no arquivo de saída

   //Recebendo os argumentos de entrada
   if(argc < 3) {
      fprintf(stderr, "Digite: %s <dimensao> <nome arquivo saida>\n", argv[0]);
      return 1;
   }
   n = atoi(argv[1]);

   //Alocando memória para os vetores
   vetor1 = (float*) malloc(sizeof(float) * n);
   if(!vetor1) {
      fprintf(stderr, "Erro de alocao da memoria do vetor1\n");
      return 2;
   }

   vetor2 = (float*) malloc(sizeof(float) * n);
   if(!vetor2) {
      fprintf(stderr, "Erro de alocao da memoria do vetor2\n");
      return 2;
   }

   //Preenchendo os vetores com valores float aleatórios e calculando o produto interno
   srand(time(NULL));

   for(long int i=0; i<n; i++) {
      elem = (rand() % MAX)/3.0 * fator;     //Aqui a forma de geração pode variar
      vetor1[i] = elem;
      elem = (rand() % MAX)/3.0 * fator; 
      vetor2[i] = elem;

      prod_interno += vetor1[i] * vetor2[i];    //Calcula o produto interno dos vetores
      fator*=-1;
   }

   //Imprimindo na saída padrão os vetores gerados e o resultado do produto interno
   #ifdef LOG
   fprintf(stdout, "%ld\n", n);
   fprintf(stdout, "Vetor 1: ");
   for(long int i=0; i<n; i++) {
      fprintf(stdout, "%f ",vetor1[i]);
   }
   fprintf(stdout, "\n");
   fprintf(stdout, "Vetor 2: ");
   for(long int i=0; i<n; i++) {
      fprintf(stdout, "%f ",vetor2[i]);
   }
   fprintf(stdout, "\n");
   fprintf(stdout, "%lf\n", prod_interno);
   #endif

   //Abrindo o arquivo para escrita binária
   descritorArquivo = fopen(argv[2], "wb");
   if(!descritorArquivo) {
      fprintf(stderr, "Erro de abertura do arquivo\n");
      return 3;
   }
   //Escrevendo a dimensão
   ret = fwrite(&n, sizeof(long int), 1, descritorArquivo);

   //Escrevendo os elementos dos vetores
   ret = fwrite(vetor1, sizeof(float), n, descritorArquivo);
   if(ret < n) {
      fprintf(stderr, "Erro de escrita do vetor1 no arquivo\n");
      return 4;
   }
   ret = fwrite(vetor2, sizeof(float), n, descritorArquivo);
   if(ret < n) {
      fprintf(stderr, "Erro de escrita do vetor2 no arquivo\n");
      return 4;
   }

   //Escrevendo o produto interno
   ret = fwrite(&prod_interno, sizeof(double), 1, descritorArquivo);

   //Finalizando o uso das variáveis
   fclose(descritorArquivo);
   free(vetor1);
   free(vetor2);
   return 0;
} 



//Certifique-se da corretude desse programa. Execute-o gerando arquivos de teste com diferentes valores de n e os armazene. 
