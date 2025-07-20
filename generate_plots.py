import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Configurar estilo científico
plt.style.use('seaborn-v0_8-poster')
sns.set_context("paper", font_scale=1.5)
plt.rcParams['font.family'] = 'DejaVu Sans'

df = pd.read_csv('results.csv')

# Gráfico 1: Comparação de Speedup
fig, ax = plt.subplots(figsize=(12, 8))
for size in [10000, 50000, 100000]:
    serial_time = df[(df['size'] == size) & (df['type'] == 'serial')]['time'].mean()
    
    # OpenMP
    omp_df = df[(df['size'] == size) & (df['type'] == 'openmp')]
    omp_df['speedup'] = serial_time / omp_df['time']
    ax.plot(omp_df['threads'], omp_df['speedup'], 'o-', linewidth=3, markersize=10, label=f'OpenMP {size//1000}K')
    
    # MPI
    mpi_df = df[(df['size'] == size) & (df['type'] == 'mpi')]
    mpi_df['speedup'] = serial_time / mpi_df['time']
    ax.plot(mpi_df['threads'], mpi_df['speedup'], 's--', linewidth=3, markersize=10, label=f'MPI {size//1000}K')

ax.plot([1, 8], [1, 8], 'k:', label='Speedup Ideal')
ax.set_xlabel('Número de Threads/Processos')
ax.set_ylabel('Speedup')
ax.set_title('Comparação de Speedup: OpenMP vs MPI')
ax.legend()
ax.grid(True, linestyle='--', alpha=0.7)
plt.savefig('comparacao_speedup.png', dpi=300, bbox_inches='tight')

# Gráfico 2: Overhead MPI
fig, ax = plt.subplots(figsize=(12, 8))
for size in [10000, 50000, 100000]:
    mpi_df = df[(df['size'] == size) & (df['type'] == 'mpi')]
    ax.plot(mpi_df['threads'], mpi_df['overhead'], 'd-.', linewidth=2, markersize=12, label=f'{size//1000}K elementos')

ax.set_xlabel('Número de Processos')
ax.set_ylabel('Overhead de Comunicação (%)')
ax.set_title('Overhead MPI por Tamanho do Problema')
ax.legend()
ax.grid(True, linestyle='--', alpha=0.7)
plt.savefig('overhead_mpi.png', dpi=300, bbox_inches='tight')