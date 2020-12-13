#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define ISIZE 1000
#define JSIZE 1000

void write_file(const char* name, double* a);
void run_first_ext(int rank, int size, double* a);
void work_first_ext(double* a, size_t start, size_t end);

int main(int argc, char** argv) {
	int rank, size;
	int i, j;
	double start_t, end_t;
	double* a;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Barrier(MPI_COMM_WORLD);
	start_t = MPI_Wtime();

	size_t len = ISIZE / size;
	size_t mod = ISIZE % size;

	len = (rank >= mod) ? len : len + 1.0;
    	size_t start = rank * len;
    	start = (rank >= mod) ? start + mod : start;
	
	a = (double*)malloc(ISIZE * JSIZE * sizeof(double));
	run_first_ext(rank, size, a);

	for(i = start; i < start + len; i++) {
		if(i < ISIZE - 1) {
			for(j = 1; j < JSIZE; j++) {
				a[i * JSIZE + j] = sin(0.00001 * a[(i + 1) * JSIZE + j - 1]);
			}
		}
	}

	size_t send_size = JSIZE * len;
	int recv_size[size];
	int positions[size];
	
	positions[0] = 0;
	recv_size[0] = JSIZE * len;
	
	for(i = 1; i < size; i++) {
	   	positions[i] = positions[i - 1] + (i <= mod ? JSIZE * (ISIZE / size + 1) : JSIZE * (ISIZE / size));
		recv_size[i] = (i < mod ? JSIZE * (ISIZE / size + 1) : JSIZE * (ISIZE / size));
	} 	
	
	MPI_Gatherv(a + start * JSIZE, send_size, MPI_DOUBLE, a, recv_size, positions, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	end_t = MPI_Wtime();

	if (rank == 0) {
		printf("TIME[%f] SIZE[%d]\n", end_t - start_t, size);
		write_file("2a_par.txt", a);
	}	
	MPI_Finalize();
	return 0;
}

void write_file(const char* name, double* a) {
	FILE *ff;
	ff = fopen(name, "w");
	for(int i = 0; i < ISIZE; i++) {
		for (int j = 0; j < JSIZE; j++) { 
			fprintf(ff, "%f ", a[i * ISIZE + j]);
		}
		fprintf(ff, "\n");
	}
	fclose(ff);
	free(a);
}

void run_first_ext(int rank, int size, double* a) {
	MPI_Status status;
	size_t len = ISIZE / size;
	size_t mod = ISIZE % size;

	len = (rank >= mod) ? len : len + 1.0;
    	size_t start = rank * len;
    	start = (rank >= mod) ? start + mod : start;

	if (rank != 0) {
        double* tmp = (double*)malloc(len * JSIZE * sizeof(double));
		work_first_ext(tmp, start, len + start);
		int send_size = len * JSIZE;
		MPI_Send(tmp, send_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		free(tmp);
	} else {
		work_first_ext(a, start, len + start);
        for (size_t i = 1; i < size; i++) {
        	if (start == 0) {
        		start = len * JSIZE;
        	}
			len = (i == mod) ? len - 1.0 : len;
			int recv_size = len * JSIZE;
			MPI_Recv(a + start, recv_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
			start += len * JSIZE;
		}

		for (size_t i = 1; i < size; i++) {
			MPI_Send(a, JSIZE * ISIZE, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
		}
	}

	if (rank != 0) {
		MPI_Recv(a, JSIZE * ISIZE, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, NULL);
	}
}

void work_first_ext(double* a, size_t start, size_t end) {
	for (size_t i = 0; i < end - start; i++) {
	    for (size_t j = 0; j < JSIZE; j++) {
	    	a[i * JSIZE + j] = 10 * (i + start) + j;
	    }
	}
}
