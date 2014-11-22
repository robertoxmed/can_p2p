#ifndef __CAN_H
#define __CAN_H

//Changer dans le Makefile le nombre de processus
#define N 500
#define NB_POS 3
#define NB_COORD 4
#define MAXABS 1000
#define MAXORD 1000

#define INS 1
#define IOK 2
#define INFO 3
#define PERTE 4
#define GAIN 5
#define CAN_VOIS 6
#define INS_DATA 7
#define INS_DATA_ACK 8
#define SEARCH_DATA 9
#define FOUND 10
#define NOT_FOUND 11

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <time.h>

typedef struct {
	int rank;
	int pos[NB_POS];
	int can_interval[NB_COORD];
} node_voisin;

typedef struct {
    int pos[NB_POS];
} data;

struct node_t{
	int rank;
	int pos[NB_POS];
	int nb_voisins;
	node_voisin voisins[N];
	int can_interval[NB_COORD];
    int nb_data;
	data n_data[10 * N];
};

typedef struct node_t node;

void node_initialize(node *n, int rank);

void coord_exec();

void bootstrap_exec(node *n);

void routage(node * n, int *rang, int * pos, MPI_Status * status, int type);

void inclu (node * appelant, node_voisin * vois, int dest, int * pos, int * coord);

void envoie_info_voisins(node *n, int dest, int * pos, int * coord);

void insertion_data(node *n, int *pos);

void insertion(node *n, int *rang, int *pos, MPI_Status *status);

void node_exec(node *n);


#endif

