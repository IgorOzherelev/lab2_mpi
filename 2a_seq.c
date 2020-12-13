#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ISIZE 1000
#define JSIZE 1000


int main(int argc, char** argv) {
	double a[ISIZE][JSIZE];
	clock_t start = clock();

	FILE *ff;

	for (int i = 0; i < ISIZE; i++) {
		for (int j = 0; j < JSIZE; j++) {
			a[i][j] = 10 * i + j;
		}
	}

	for (int i = 0; i < ISIZE - 1; i++) {
		for (int j = 1; j < JSIZE; j++) {
			a[i][j] = sin(0.00001 * a[i + 1][j - 1]);
		}
	}
	clock_t end = clock();
	ff = fopen("2a_seq.txt", "w");
	printf("TIME[%f]\n", (double)(end - start) / CLOCKS_PER_SEC);
	for (int i = 0; i < ISIZE; i++) {
		for (int j = 0; j < JSIZE; j++) {
			fprintf(ff, "%f ", a[i][j]);
		}
		fprintf(ff, "\n");
	}
	fclose(ff);

	return 0;
}