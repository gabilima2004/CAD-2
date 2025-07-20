// odd_even_openmp.c
// Implementação paralela com OpenMP do Odd-Even Transposition Sort.

#include <stdio.h>
#include <stdlib.h>
#include <omp.h> // [cite: 116]
#include <time.h>  // Para time(NULL)

#ifdef _WIN32
#include <windows.h>
#endif

// Funções auxiliares (swap, print_array, generate_random_array, is_sorted)
// (Copiar as mesmas funções do arquivo serial aqui para o código ser autocontido)

void swap(int *a, int *b) { int temp = *a; *a = *b; *b = temp; }

void print_array(int arr[], int n) {
    int limit = (n < 20) ? n : 20;
    for (int i = 0; i < limit; i++) { printf("%d ", arr[i]); }
    if (n > 20) { printf("... (exibindo apenas os 20 primeiros elementos)"); }
    printf("\n");
}

void generate_random_array(int arr[], int n, int max_val) {
    srand(time(NULL));
    for (int i = 0; i < n; i++) { arr[i] = rand() % max_val; }
}

int is_sorted(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) { if (arr[i] > arr[i + 1]) return 0; }
    return 1;
}

void odd_even_sort_openmp(int arr[], int n, int num_threads) {
    int phase;
    #pragma omp parallel num_threads(num_threads) shared(arr, n) private(phase)
    {
        for (phase = 0; phase < n; phase++) {
            // Fase única otimizada
            #pragma omp for schedule(static)
            for (int i = phase % 2; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    int temp = arr[i];
                    arr[i] = arr[i + 1];
                    arr[i + 1] = temp;
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
    if (argc != 3) {
        printf("Uso: %s <tamanho_array> <numero_threads>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    
    int *arr = (int *)malloc(n * sizeof(int));
    if (arr == NULL) {
        printf("Erro na alocação de memória.\n");
        return 1;
    }

    generate_random_array(arr, n, 1000);
    printf("Array original: ");
    print_array(arr, n);

    // Medição de tempo com omp_get_wtime() [cite: 115, 121]
    double start_time = omp_get_wtime();
    
    // Para testar as diferentes políticas, altere o 'schedule(static)' no código
    // ou crie diferentes funções para cada uma.
    odd_even_sort_openmp(arr, n, num_threads);

    double end_time = omp_get_wtime(); // [cite: 128]
    double elapsed_time = end_time - start_time; // [cite: 129, 130, 131]

    printf("Array ordenado: ");
    print_array(arr, n);

    printf("Array está ordenado? %s\n", is_sorted(arr, n) ? "Sim" : "Não");
    printf("OPENMP_TIME: %.6f\n", elapsed_time);

    free(arr);
    return 0;
}