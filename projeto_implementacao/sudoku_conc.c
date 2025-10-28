#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

// Define o tamanho do grid
#define SIZE 9
#define SUBGRID_SIZE 3

//Structs e Variáveis Globais

typedef struct {   //Struct para passar dados para cada thread. Cada thread precisa de sua cópia do grid.
    int **grid, size, size_sub_grid;
} ThreadData;

//Variáveis globais compartilhadas
pthread_mutex_t solution_mutex;   //Mutex para exclusão mútua
short int solution_found = 0;   //Flag: 1 se a solução foi encontrada
int **solution_grid;         //Matriz onde a solução final será armazenada

//Funções Auxiliares

//Imprimir o grid
void print_grid(int **grid, int size) {
    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++)
            printf("%2d", grid[r][c]);
        printf("\n");
    }
}

//Procura por uma célula vazia (valor 0). Retorna 1 se encontrou, 0 se o grid está cheio
int find_empty_cell(int **grid, int size, int *row, int *col) {
    for (*row = 0; *row < size; (*row)++) {
        for (*col = 0; *col < size; (*col)++) {
            if (grid[*row][*col] == 0)    //Ao sair dessa função, row e col apontarão para a primeira célula vazia encontrada
                return 1;
        }
    }
    return 0;
}

//Verifica se um número é válido em uma dada posição
int is_valid(int **grid, int size, int size_sub_grid, int row, int col, int num) {
    //Verifica a linha
    for (int c = 0; c < size; c++)
        if (grid[row][c] == num) return 0;

    //Verifica a coluna
    for (int r = 0; r < size; r++)
        if (grid[r][col] == num) return 0;

    //Verifica sub-grid
    int startRow = row - row % size_sub_grid;
    int startCol = col - col % size_sub_grid;
    for (int r = 0; r < size_sub_grid; r++)
        for (int c = 0; c < size_sub_grid; c++)
            if (grid[r + startRow][c + startCol] == num)
                return 0;
    
    return 1;
}


/*
Solucionador Sequencial (Backtracking). Esta é a função que resolve o Sudoku de forma sequencial, e será chamada por 
cada thread em sua própria cópia do grid
 */
int solve_sequential(int **grid, int size, int size_sub_grid) {
    int row, col;

    // Antes de fazer qualquer coisa, verificamos se outra thread já encontrou a solução
    pthread_mutex_lock(&solution_mutex);
    short int found = solution_found;
    pthread_mutex_unlock(&solution_mutex);

    if (found) {
        return 0;     //Para este ramo da recursão, já que alguém já ganhou
    }

    if (!find_empty_cell(grid, size, &row, &col)) {
        return 1;   //Se não há células vazias, o Sudoku está resolvido (sucesso)
    }

    //Tenta números de 1 a 9
    for (int num = 1; num <= 9; num++) {
        if (is_valid(grid, size, size_sub_grid, row, col, num)) {   //Se a posição é válida para o número
            grid[row][col] = num;

            if (solve_sequential(grid, size, size_sub_grid)) {   //Chama recursivamente
                return 1;
            }

            grid[row][col] = 0;   //Falhou, desfaz a tentativa (backtrack)
        }
    }

    return 0;   //Nenhuma solução encontrada neste ramo
}


/*
Função que as threads executam ao serem criadas para resolver o Sudoku de forma concorrente
 */
void* solver_thread_func(void* arg) {
    ThreadData* data = (ThreadData*) arg;   //Recebe os dados (sua cópia do grid)

    //Tenta resolver o Sudoku sequencialmente a partir deste ponto
    if (solve_sequential(data->grid, data->size, data->size_sub_grid)) {

        //Se tiver sucesso, tenta registrar como a solução global
        pthread_mutex_lock(&solution_mutex);
        
        if (!solution_found) {
            solution_found = 1;
            for (int r = 0; r < data->size; r++) {  //Copia a solução local para a global
                memcpy(solution_grid[r], data->grid[r], data->size * sizeof(int));
            }
        }

        pthread_mutex_unlock(&solution_mutex);
    }

    //Liberando a memória alocada para esta thread
    if (data->grid) {
        for (int r = 0; r < data->size; r++) free(data->grid[r]);
        free(data->grid);
    }
    free(data);
    
    pthread_exit(NULL);
}


//Função main
int main() {
    //Aloca memória para o grid de solução
    solution_grid = (int **) malloc(SIZE * sizeof(int *));
    if (solution_grid == NULL) {
        printf("Erro na alocação de memória para a solução\n");
        return 1;
    }
    for (int i = 0; i < SIZE; i++) {
        solution_grid[i] = (int *) malloc(SIZE * sizeof(int));
        if (solution_grid[i] == NULL) {
            printf("Erro na alocação de memória para a solução\n");
            for (int k = 0; k < i; k++) free(solution_grid[k]);
            free(solution_grid);
            return 1;
        }
    }

    // Um Sudoku "difícil" para testar (O 0 representa células vazias)
    int puzzle_grid[SIZE][SIZE] = {
        {5, 3, 4, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}
    };

    for (int i = 0; i < SIZE; i++) {  //Inicializa a solução com o grid inicial
        memcpy(solution_grid[i], puzzle_grid[i], SIZE * sizeof(int));
    }

    printf("--- Solucionador de Sudoku Concorrente ---\n");
    printf("Grid Inicial:\n");
    print_grid(solution_grid, SIZE);
    printf("\n");

    // Inicializa o mutex
    if (pthread_mutex_init(&solution_mutex, NULL)) {
        printf("\nFalha ao inicializar o mutex\n");
        for (int i = 0; i < SIZE; i++) free(solution_grid[i]);
        free(solution_grid);
        return 1;
    }

    int row, col;

    if (!find_empty_cell(solution_grid, SIZE, &row, &col)) {   //Encontra a primeira célula vazia para ser o ponto de divisão
        printf("Grid inicial já está resolvido.\n");   //Caso em que o grid já está resolvido
        solution_found = 1;
    }
    else{
        pthread_t threads[SIZE];   //Array para guardar os IDs dos threads
        int thread_count = 0;

        //Tenta todos os números (1-9) para essa única célula
        for (int num = 1; num <= 9; num++) {
            if (is_valid(solution_grid, SIZE, SUBGRID_SIZE, row, col, num)) {
                //Se o número for válido, cria uma thread para essa hipótese
                //Aloca memória para os dados da thread
                ThreadData* thread_data = (ThreadData*) malloc(sizeof(ThreadData));
                if (thread_data == NULL) {
                    printf("Erro na alocação de memória para os dados do thread\n");
                    return 1;
                }
                thread_data->size = SIZE;
                thread_data->size_sub_grid = SUBGRID_SIZE;
                
                //Copia o grid e insere o palpite
                //Aloca a matriz para thread_data->grid
                thread_data->grid = (int**) malloc(SIZE * sizeof(int*));
                if (thread_data->grid == NULL) { free(thread_data); printf("Erro malloc\n"); return 1; }
                for (int r = 0; r < SIZE; r++) {
                    thread_data->grid[r] = (int*) malloc(SIZE * sizeof(int));
                    if (thread_data->grid[r] == NULL) {
                        for (int k = 0; k < r; k++) free(thread_data->grid[k]);
                        free(thread_data->grid);
                        free(thread_data);
                        printf("Erro malloc\n");
                        return 1;
                    }
                    memcpy(thread_data->grid[r], solution_grid[r], SIZE * sizeof(int));
                }

                thread_data->grid[row][col] = num;

                //Cria a thread
                printf("Iniciando thread para a hipótese: Célula (%d, %d) = %d\n", row, col, num);
                if (pthread_create(&threads[thread_count], NULL, solver_thread_func, thread_data)) {
                    fprintf(stderr, "Erro: pthread_create falhou para hipótese %d\n", num);
                    for (int k = 0; k < SIZE; k++) free(thread_data->grid[k]);
                    free(thread_data->grid);
                    free(thread_data);
                    continue;
                }

                thread_count++;
            }
        }
        
        printf("%d threads de trabalho criadas. Aguardando conclusão...\n", thread_count);

        //Espera todas as threads terminarem
        for (int i = 0; i < thread_count; i++) {
            if (pthread_join(threads[i], NULL)) {
                printf("Erro ao esperar thread\n");
                for (int k = 0; k < SIZE; k++) free(solution_grid[k]);
                free(solution_grid);
                return 1;
            }
        }

        printf("Todas as threads terminaram.\n");
    }


    // Imprime o resultado
    if (solution_found) {
        printf("\nSolução Encontrada:\n");
        print_grid(solution_grid, SIZE);
    } 
    else {
        printf("\nNenhuma solução foi encontrada para o Sudoku.\n");
    }

    // Destrói o mutex e desaloca memória
    pthread_mutex_destroy(&solution_mutex);
    for (int i = 0; i < SIZE; i++) {
        free(solution_grid[i]);
    }
    free(solution_grid);

    return 0;
}