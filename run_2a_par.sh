mpicc 2a_par.c -o 2a_par -lm
mpirun -n 5 ./2a_par
rm 2a_par