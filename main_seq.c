#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ISIZE 1000
#define JSIZE 1000

int main(int argc, char** argv) {
	double a[ISIZE][JSIZE];
	int i, j;
	clock_t start = clock();

	FILE *ff;

    for (i = 0; i < ISIZE; i++) {
        for (j = 0; j < JSIZE; j++) {
            a[i][j] = 10 * i + j;
        }
    }

	for (int i = 0; i < ISIZE; i++) {
		for (int j = 0; j < JSIZE; j++) {
			a[i][j] = sin(0.00001 * a[i][j]) ;
		}
	}

	ff = fopen("result.txt", "w");
	clock_t end = clock();
	printf("TIME[%f]\n", (double)(end - start) / CLOCKS_PER_SEC);

	for (i = 0; i < ISIZE; i++) {
		for (j = 0; j < JSIZE; j++) {
			fprintf(ff, "%f ", a[i][j]);
		}
		fprintf(ff, "\n");
	}
	fclose(ff);

	return 0;
}