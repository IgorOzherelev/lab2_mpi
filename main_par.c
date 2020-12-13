#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <mpi.h>

#define ISIZE 1000
#define JSIZE 1000

typedef struct
{
	size_t isBoth;
	size_t block_size;
	size_t mod;
	void (*ptr_work_fun)(double* , size_t, size_t);
} run_params;

void work_ext(double* a, size_t start, size_t end);
void work_both(double* a, size_t start, size_t end);

void run(int rank, int size, double* a, run_params params);
void write_file(const char* name, double* a);

int main(int argc, char **argv) {
	int rank, size;
	double start_t, middle_t, end_t;
	run_params ext, both;
	double* a_ext;
	double* a_both;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	ext.isBoth = 0;
	ext.block_size = ISIZE / size;
	ext.mod = ISIZE % size;
	ext.ptr_work_fun = &work_ext;

	both.isBoth = 1;
	both.block_size = ISIZE * JSIZE / size;
	both.mod = ISIZE * JSIZE % size;
	both.ptr_work_fun = &work_both;
	
	if (rank == 0) {
		a_ext = (double*)malloc(ISIZE * JSIZE * sizeof(double));
		a_both = (double*)malloc(ISIZE * JSIZE * sizeof(double));
	}

	MPI_Barrier(MPI_COMM_WORLD);
	start_t = MPI_Wtime();
	run(rank, size, a_ext, ext);
	MPI_Barrier(MPI_COMM_WORLD);
	middle_t = MPI_Wtime();
	run(rank, size, a_both, both);
	MPI_Barrier(MPI_COMM_WORLD);
	end_t = MPI_Wtime();

	if (rank == 0) {
		printf("EXT TIME[%f]\n", middle_t - start_t);
		printf("BOTH TIME[%f]\n", end_t - middle_t);
		write_file("result_par.txt", a_ext);
		write_file("result_par.txt", a_both);
	}
	MPI_Finalize();
	return 0;
}

void write_file(const char* name, double* a) {
	FILE *ff;
	ff = fopen(name, "w");
	for(int i = 0; i < ISIZE; i++) {
		for (int j = 0; j < JSIZE; j++) { 
			fprintf(ff, "%f", a[i * ISIZE + j]);
		}
		fprintf(ff, "\n");
	}
	fclose(ff);
	free(a);
}

void run(int rank, int size, double* a, run_params params) {
	MPI_Status status;
	size_t mod = params.mod;

	size_t len = params.block_size;
	len = (rank >= mod) ? len : len + 1.0;

    	size_t start = rank * len;
    	start = (rank >= mod) ? start + mod : start;

	if (rank != 0) {
        double* tmp = (double*)malloc(len * JSIZE * sizeof(double));
		params.ptr_work_fun(tmp, start, len + start);
		int send_size = (params.isBoth == 0) ? len * JSIZE : len;
		MPI_Send(tmp, send_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		free(tmp);
	} else {
		params.ptr_work_fun(a, start, len + start);
        for (size_t i = 1; i < size; i++) {
        	if (start == 0) {
        		start = (params.isBoth == 0) ? len * JSIZE : len;
        	}
			len = (i == mod) ? len - 1.0 : len;
			int recv_size = (params.isBoth == 0) ? len * JSIZE : len;
			MPI_Recv(a + start, recv_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
			if (params.isBoth == 0) {
				start += len * JSIZE;
			} else {
				start += len;
			}
		}
	}
}

void work_ext(double* a, size_t start, size_t end) {
	for (size_t i = 0; i < end - start; i++) {
	    for (size_t j = 0; j < JSIZE; j++) {
	    	a[i * JSIZE + j] = sin(0.00001 * (10 * (i + start) + j));
	    }
	}
}

void work_both(double* a, size_t start, size_t end) {
	for (size_t i = 0; i < end - start; i++) {
	    a[i] = sin(0.00001 * (10 * ((i + start) / ISIZE) + ((i + start) % JSIZE)));
	}
}
