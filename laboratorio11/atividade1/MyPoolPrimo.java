/* Disciplina: Computação Concorrente */
/* Profa.: Silvana Rossetto */
/* Laboratório 11 - Atividade 1 */
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

class Primo implements Runnable {
  long numero;

  public Primo(long n) {
    this.numero = n;
  }

  public void run() {
    int i;
    boolean ehPrimo = false;

    if(this.numero == 2) ehPrimo = true;
    else if(this.numero > 2){
      if(this.numero % 2 != 0){
        for(i=3; i< Math.sqrt(this.numero)+1; i+=2) {
          if(this.numero % i == 0){
            break;
          }
        }
        if(i >= Math.sqrt(this.numero)+1) ehPrimo = true;
      }
    }
    
    if(ehPrimo) System.out.println(this.numero + " é primo.");
    else System.out.println(this.numero + " não é primo.");
  }
}

//Classe da aplicação (método main)
class MyPoolPrimo {
    private static final int NTHREADS = 10;

    public static void main (String[] args) {
      //--PASSO 2: cria o pool de threads
      FilaTarefas pool = new FilaTarefas(NTHREADS);
      long tarefas = 10000;  //Número de tarefas a serem executadas (números a serem verificados)
      
      //--PASSO 3: dispara a execução dos objetos runnable usando o pool de threads
      for (int i = 0; i < tarefas; i++) {
        Runnable primo = new Primo(i);
        pool.execute(primo);
      }

      //--PASSO 4: esperar pelo termino das threads
      pool.shutdown();
      System.out.println("Terminou");
   }
}
