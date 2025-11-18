/* Disciplina: Computação Concorrente */
/* Profa.: Silvana Rossetto */
/* Laboratório 11 - Atividade 1 item 4 */
/* Nome: Nícolas da Mota Arruda */
/* DRE: 122153341 */

/*
Programa que implementa um pool de threads para verificar se números são primos ou não.
OBS: Este código foi feito com ajuda do código "MyPool.java" disponibilizado para o laboratório 11
*/

import java.util.LinkedList;

class FilaTarefas {
    private final int nThreads;
    private final MyPoolThreads[] threads;   //Um array de workers
    private final LinkedList<Runnable> queue;   //Fila de tarefas
    private boolean shutdown;   //Indica se o pool está finalizando
    private int totalPrimos = 0;   //Contador de números primos encontrados

    public FilaTarefas(int nThreads) {   //Construtor aloca e inicia as threads
        this.shutdown = false;
        this.nThreads = nThreads;
        queue = new LinkedList<Runnable>();
        threads = new MyPoolThreads[nThreads];
        for (int i=0; i<nThreads; i++) {
            threads[i] = new MyPoolThreads();
            threads[i].start();
        } 
    }

    public void execute(Runnable r) {  //Adiciona uma nova tarefa na fila
        synchronized(queue) {   //Protege o acesso à fila com o lock do objeto queue
            if (this.shutdown) return;   //Se o pool está finalizando, não aceita novas tarefas
            queue.addLast(r);
            queue.notify();   //Acorda uma thread que esteja esperando por tarefas
        }
    }
    
    public void shutdown() {
        synchronized(queue) {  //Protege o acesso à fila com o lock do objeto queue
            this.shutdown=true;
            queue.notifyAll();  //Acorda todas as threads para que elas possam finalizar
        }
        for (int i=0; i<nThreads; i++) {
          try { threads[i].join(); } catch (InterruptedException e) { return; }
        }
    }

    //Métodos auxiliares para contar o número de primos encontrados (precisam ser synchronized)
    public synchronized void incPrimo() {
      totalPrimos++;
    }

    public synchronized int getTotalPrimos() {
      return totalPrimos;
    }

    private class MyPoolThreads extends Thread {
       public void run() {  //Cada worker entra em um loop infinito pegando tarefas da fila e executando-as
         Runnable r;
         while (true) {
           synchronized(queue) {   //Protege o acesso à fila com o lock do objeto queue
             while (queue.isEmpty() && (!shutdown)) {   //Enquanto a fila estiver vazia e o pool não estiver finalizando, espera por tarefas
               try { queue.wait(); }
               catch (InterruptedException ignored){}
             }
             if (queue.isEmpty()) return;   
             r = (Runnable) queue.removeFirst();   //Pega a primeira tarefa da fila para executar
           }
           try { r.run(); }
           catch (RuntimeException e) { System.out.println("Erro na execução da tarefa"); }
         } 
       } 
    } 
}

//Classe que verifica se os números são primos
class Primo implements Runnable {
  long numero;
  FilaTarefas pool;

  public Primo(long n, FilaTarefas pool) {
    this.numero = n;
    this.pool = pool;
  }

  private boolean ehPrimo(long n) {
    if(n <= 1) return false;
    if(n == 2) return true;
    if(n % 2 == 0) return false;
    for(long i=3; i <= Math.sqrt(n); i+=2) {
      if(n % i == 0) return false;
    }
    return true;
  }

  public void run() {
    if(ehPrimo(this.numero)){
      System.out.println(this.numero + " é primo.");
      pool.incPrimo();
    }
    else {
      //System.out.println(this.numero + " não é primo.");
    }
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

//Classe da aplicação (método main)
class MyPoolPrimo {
    private static final int NTHREADS = 5;

    public static void main (String[] args) {
      FilaTarefas pool = new FilaTarefas(NTHREADS);
      long tarefas = 100000;  //Número de tarefas a serem executadas (números a serem verificados)

      for (long i = 0; i < tarefas; i++) {
        Runnable primo = new Primo(i, pool);
        pool.execute(primo);
      }

      pool.shutdown();  //Aguarda todas as tarefas terminarem
      System.out.println("Total de números primos encontrados (concorrente): " + pool.getTotalPrimos());

      PrimoSequencial seqPrimo = new PrimoSequencial();
      int totalSeq = seqPrimo.contarPrimos(0, tarefas - 1);
      System.out.println("Total de números primos encontrados (sequencial): " + totalSeq);
      if (totalSeq == pool.getTotalPrimos()) System.out.println("Contagem correta!");
      else System.out.println("Contagem incorreta!");
      System.out.println("Terminou");
   }
}
