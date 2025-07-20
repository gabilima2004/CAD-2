CC = gcc
CFLAGS = -O3 -Wall
OMP_FLAG = -fopenmp

MPI_INC = "C:\Program Files (x86)\Microsoft SDKs\MPI\Include"
MPI_LIB = "C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64"

mpi: odd_even_mpi.c
	gcc $(CFLAGS) -I $(MPI_INC) -o odd_even_mpi.exe odd_even_mpi.c -L $(MPI_LIB) -lmsmpi

all: serial openmp

serial: odd_even_serial.c
	$(CC) $(CFLAGS) -o odd_even_serial.exe odd_even_serial.c

openmp: odd_even_openmp.c
	$(CC) $(CFLAGS) $(OMP_FLAG) -o odd_even_openmp.exe odd_even_openmp.c

clean:
	del /f *.exe

test: all
	.\odd_even_serial.exe 1000
	.\odd_even_openmp.exe 1000 4
	@echo "Para testar MPI, execute manualmente:"
	@echo "& 'C:\Program Files\Microsoft MPI\Bin\mpiexec.exe' -n 4 .\odd_even_mpi.exe 1000"