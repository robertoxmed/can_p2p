all: can run_can

can: can.c can.h
	mpicc -o can can.c

#Changer dans can.h pour le nombre de processus
run_can: can
	mpirun -np 500 can

clean:
	rm -f can
