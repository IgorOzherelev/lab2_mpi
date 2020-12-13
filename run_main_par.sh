mpicc main_par.c -o main_par -lm
mpirun -n 5 ./main_par
rm main_par