/* Disciplina: Computação Concorrente */
/* Profa.: Silvana Rossetto */
/* Laboratório 11 - Atividade 3 item 2 */
/* Nome: Nícolas da Mota Arruda */
/* DRE: 122153341 */

/*
Programa que implementa um verificador de números primos usando Callable e Future.
OBS: Este código foi feito com ajuda do código "FutureHello.java" disponibilizado para o laboratório 11
*/

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.ArrayList;
import java.util.List;

class VerificaPrimos implements Callable<Boolean> {
    private final long num;

    VerificaPrimos(long num) {
        this.num = num;
    }

    boolean ehPrimo(long n) {
        int i;
        if(this.num<=1) return false;
        if(this.num==2) return true;
        if(this.num%2==0) return false;
        for(i=3; i< Math.sqrt(this.num)+1; i+=2) {
            if(this.num%i==0) return false;
        }
        return true;
    }

    public Boolean call() throws Exception {
        boolean ehPrimo = ehPrimo(this.num);
        System.out.println("Número " + this.num + (ehPrimo ? " é primo." : " não é primo."));
        return ehPrimo;
    }
}


public class FutureHelloPrimo  {
  private static final int NTHREADS = 10;

  public static void main(String[] args) {
    long N = 1000;
    if (args.length > 0) {
      try { N = Long.parseLong(args[0]); } catch (NumberFormatException e) {    //Passando o valor de N via linha de comando
        System.err.println("Argumento inválido para N; usando valor padrão " + N);
      }
    }

    ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);   //Pool de threads
    List<Future<Boolean>> list = new ArrayList<Future<Boolean>>();   //Lista de futures para armazenar os resultados

    for (long num = 1; num <= N; num++) {
        Callable<Boolean> worker = new VerificaPrimos(num);
        Future<Boolean> submit = executor.submit(worker);
        list.add(submit);
    }

    for (Future<Boolean> f : list) {
      try {
        f.get();
      } 
      catch (InterruptedException e) {
        e.printStackTrace();
      } 
      catch (ExecutionException e) {
        e.printStackTrace();
      }
    }

    executor.shutdown();
  }
}