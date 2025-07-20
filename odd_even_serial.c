// odd_even_serial.c
// Implementação de referência do Odd-Even Transposition Sort.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Função para trocar dois elementos
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Função para imprimir os primeiros N elementos de um array
void print_array(int arr[], int n) {
    int limit = (n < 20) ? n : 20;
    for (int i = 0; i < limit; i++) {
        printf("%d ", arr[i]);
    }
    if (n > 20) {
        printf("... (exibindo apenas os 20 primeiros elementos)");
    }
    printf("\n");
}

// Função para gerar um array com números aleatórios
void generate_random_array(int arr[], int n, int max_val) {
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % max_val;
    }
}

// Função que verifica se o array está ordenado
int is_sorted(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0; // Retorna 0 (falso) se encontrar um par fora de ordem
        }
    }
    return 1; // Retorna 1 (verdadeiro) se o array estiver ordenado
}

// Implementação do algoritmo Odd-Even Sort serial 
void odd_even_sort_serial(int arr[], int n) {
    int phase, i;
    for (phase = 0; phase < n; phase++) { // O algoritmo executa n fases 
        if (phase % 2 == 0) { // Fase Par 
            for (i = 1; i < n; i += 2) {
                if (arr[i - 1] > arr[i]) {
                    swap(&arr[i - 1], &arr[i]);
                }
            }
        } else { // Fase Ímpar 
            for (i = 1; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    swap(&arr[i], &arr[i + 1]);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
     #ifdef _WIN32
    // Configurar codificação UTF-8 no Windows
    SetConsoleOutputCP(CP_UTF8);
    #endif
    if (argc != 2) { // Valida os argumentos de entrada 
        printf("Uso: %s <tamanho_array>\n", argv[0]); 
        return 1;
    }

    int n = atoi(argv[1]); 
    int *arr = (int *)malloc(n * sizeof(int));
    if (arr == NULL) {
        printf("Erro na alocação de memória.\n");
        return 1;
    }

    generate_random_array(arr, n, 1000); 
    printf("Array original: ");
    print_array(arr, n);

    // Medição de tempo usando time() para a versão serial
    clock_t start = clock();
    odd_even_sort_serial(arr, n); 
    clock_t end = clock();
    double elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Array ordenado: ");
    print_array(arr, n);

    printf("Array está ordenado? %s\n", is_sorted(arr, n) ? "Sim" : "Não"); 
    printf("SERIAL_TIME: %.6f\n", elapsed_time);

    free(arr); 
    return 0; 
}