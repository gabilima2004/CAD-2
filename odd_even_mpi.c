#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h> // Adicionado para memcpy

#ifdef _WIN32
#include <windows.h>
#endif

// Funções auxiliares
void swap(int *a, int *b) { 
    int temp = *a;
    *a = *b;
    *b = temp;
}

int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int is_sorted(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0;
        }
    }
    return 1;
}

// Função para mesclar dois arrays ordenados
void merge(int *a, int *b, int *c, int na, int nb) {
    int i = 0, j = 0, k = 0;
    while (i < na && j < nb) {
        if (a[i] < b[j]) {
            c[k++] = a[i++];
        } else {
            c[k++] = b[j++];
        }
    }
    while (i < na) c[k++] = a[i++];
    while (j < nb) c[k++] = b[j++];
}

int main(int argc, char *argv[]) {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif
    
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc != 2) {
        if (rank == 0) printf("Uso: mpirun -np <num_procs> %s <tamanho_array>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }
    
    int n_global = atoi(argv[1]);
    int *global_arr = NULL;
    int *local_arr = NULL;
    int local_n;
    
    // Distribuição equilibrada
    int *counts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));
    
    int base = n_global / size;
    int remainder = n_global % size;
    
    for (int i = 0; i < size; i++) {
        counts[i] = base + (i < remainder ? 1 : 0);
        displs[i] = (i == 0) ? 0 : displs[i-1] + counts[i-1];
    }
    
    local_n = counts[rank];
    local_arr = (int *)malloc(local_n * sizeof(int));
    
    // Processo 0 gera os dados
    if (rank == 0) {
        global_arr = (int *)malloc(n_global * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < n_global; i++) {
            global_arr[i] = rand() % 1000;
        }
    }
    
    // Distribuir dados
    MPI_Scatterv(global_arr, counts, displs, MPI_INT, 
                local_arr, local_n, MPI_INT, 
                0, MPI_COMM_WORLD);
    
    // Medição de tempo
    MPI_Barrier(MPI_COMM_WORLD);
    double total_start = MPI_Wtime();
    double comm_time = 0.0;
    
    // Ordenação local inicial
    qsort(local_arr, local_n, sizeof(int), compare);
    
    // Algoritmo Odd-Even Transposition Sort distribuído
    for (int phase = 0; phase < size; phase++) {
        int partner;
        
        // Determinar parceiro baseado na fase
        if (phase % 2 == 0) { // Fase par: pares entre ranks i (par) e i+1 (ímpar)
            if (rank % 2 == 0) {
                partner = rank + 1;
            } else {
                partner = rank - 1;
            }
        } else { // Fase ímpar: pares entre ranks i (ímpar) e i+1 (par)
            if (rank % 2 == 1) {
                partner = rank + 1;
            } else {
                partner = rank - 1;
            }
        }
        
        // Verificar se o parceiro é válido
        if (partner < 0 || partner >= size) {
            continue;
        }
        
        // Tamanho do bloco do parceiro
        int partner_n = counts[partner];
        int *recv_arr = (int *)malloc(partner_n * sizeof(int));
        int *merged_arr = (int *)malloc((local_n + partner_n) * sizeof(int));
        
        // Troca de blocos inteiros
        double comm_start = MPI_Wtime();
        MPI_Sendrecv(local_arr, local_n, MPI_INT, partner, 0,
                     recv_arr, partner_n, MPI_INT, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        comm_time += (MPI_Wtime() - comm_start);
        
        // Mesclar os dois blocos
        merge(local_arr, recv_arr, merged_arr, local_n, partner_n);
        
        // Determinar qual metade manter
        if (rank < partner) {
            // Este processo fica com a metade inferior
            memcpy(local_arr, merged_arr, local_n * sizeof(int));
        } else {
            // Este processo fica com a metade superior
            memcpy(local_arr, merged_arr + local_n, local_n * sizeof(int));
        }
        
        // Reordenar localmente
        qsort(local_arr, local_n, sizeof(int), compare);
        
        free(recv_arr);
        free(merged_arr);
    }
    
    // Coletar resultados
    MPI_Gatherv(local_arr, local_n, MPI_INT, 
               global_arr, counts, displs, MPI_INT, 
               0, MPI_COMM_WORLD);
    
    // Finalizar medição de tempo
    MPI_Barrier(MPI_COMM_WORLD);
    double total_end = MPI_Wtime();
    double total_time = total_end - total_start;
    
    // Reduzir e exibir resultados
    double total_time_max, comm_time_sum;
    MPI_Reduce(&total_time, &total_time_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&comm_time, &comm_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("Array ordenado? %s\n", is_sorted(global_arr, n_global) ? "Sim" : "Não");        
        double overhead_percent = (comm_time_sum / total_time_max) * 100;
        printf("MPI_TOTAL_TIME: %.6f\n", total_time_max);
        printf("MPI_COMM_TIME: %.6f\n", comm_time_sum);
        printf("MPI_OVERHEAD: %.2f\n", overhead_percent);
        
        free(global_arr);
    }
    
    free(local_arr);
    free(counts);
    free(displs);
    MPI_Finalize();
    return 0;
}