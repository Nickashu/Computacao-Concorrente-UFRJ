/* Disciplina: Computação Concorrente */
/* Profa.: Silvana Rossetto */
/* Laboratório 11 - Atividade 3 item 3 */
/* Nome: Nícolas da Mota Arruda */
/* DRE: 122153341 */

/*
Programa que implementa um contador de números primos em um intervalo usando Callable e Future.
OBS: Este código foi feito com ajuda do código "FutureHello.java" disponibilizado para o laboratório 11
*/

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.ArrayList;
import java.util.List;

//Classe callable para verificar o número de primos em um intervalo
class VerificaPrimos implements Callable<Integer> {
    private final long start;
    private final long end;

    VerificaPrimos(long start, long end) {
        this.start = start;
        this.end = end;
    }

    boolean ehPrimo(long n) {
        int i;
        if(n<=1) return false;
        if(n==2) return true;
        if(n%2==0) return false;
        for(i=3; i< Math.sqrt(n)+1; i+=2) {
            if(n%i==0) return false;
        }
        return true;
    }

    public Integer call() throws Exception {
        int count = 0;
        for (long i = start; i <= end; i++) {
            if (ehPrimo(i)) count++;
        }
        return count;
    }
}


//Classe auxiliar com contagem sequencial de primos para checagem de corretude
class PrimoSequencial {
  private boolean ehPrimo(long n) {
    if(n <= 1) return false;
    if(n == 2) return true;
    if(n % 2 == 0) return false;
    for(long i=3; i <= Math.sqrt(n); i+=2) {
      if(n % i == 0) return false;
    }
    return true;
  }

  public int contarPrimos(long inicio, long fim) {
    int totalPrimos = 0;
    for(long i = inicio; i <= fim; i++) {
      if(ehPrimo(i)) totalPrimos++;
    }
    return totalPrimos;
  }
}

public class FutureHelloPrimoRange  {
  private static final int NTHREADS = 10;

  public static void main(String[] args) {
    long N = 100000;
    if (args.length > 0) {
      try { N = Long.parseLong(args[0]); } catch (NumberFormatException e) {    //Passando o valor de N via linha de comando
        System.err.println("Argumento inválido para N; usando valor padrão " + N);
      }
    }
    
    //Implementando divisão de tarefas em chunks para melhorar a eficiência
    long chunk = 10000;
    if (N / chunk < NTHREADS) chunk = Math.max(1L, N / (NTHREADS * 10));

    ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);   //Pool de threads
    List<Future<Integer>> list = new ArrayList<Future<Integer>>();   //Lista de futures para armazenar os resultados

    for (long start = 1; start <= N; start += chunk) {
        long end = Math.min(N, start + chunk - 1);
        Callable<Integer> worker = new VerificaPrimos(start, end);
        Future<Integer> submit = executor.submit(worker);
        list.add(submit);
    }

    long totalPrimos = 0;
    for (Future<Integer> f : list) {
      try {
        totalPrimos += f.get();   //Recupera o número de primos contados que são retornados por cada tarefa
      } 
      catch (InterruptedException e) {
        e.printStackTrace();
      } 
      catch (ExecutionException e) {
        e.printStackTrace();
      }
    }

    System.out.println("Total de primos no intervalo de 1 até " + N + " (concorrente): " + totalPrimos);

    PrimoSequencial seqPrimo = new PrimoSequencial();
    int totalSeq = seqPrimo.contarPrimos(1, N);
    System.out.println("Total de números primos encontrados (sequencial): " + totalSeq);
    if (totalSeq == totalPrimos) System.out.println("Contagem correta!");
    else System.out.println("Contagem incorreta!");

    executor.shutdown();
  }
}