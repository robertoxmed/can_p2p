#include "can.h"

void node_initialize(node *n, int rank){
	int i;

    memset(n, 0, sizeof(node));
    n->rank = rank;
	n->nb_voisins = 0;
    srand(time(NULL) + getpid());
	for (i = 0; i < N; i++) {
		n->voisins[i].rank = -1;
	} 
    if(rank == 0){ // Le noeud coordinateur
        coord_exec(n);
    }
	else if(rank == 1){ // Le noeud bootstrap
        n->pos[0] = (int) (rand() % 1000);
		n->pos[1] = (int) (rand() % 1000);
        printf("%d > Positions de depart (%d %d)\n", n->rank, n->pos[0], n->pos[1]);
        bootstrap_exec(n);
    }
	else{ // Tous les autres noeuds
        n->pos[0] = (int) (rand() % 1000);
		n->pos[1] = (int) (rand() % 1000);
        printf("%d > Positions de depart (%d %d)\n", n->rank, n->pos[0], n->pos[1]);
        node_exec(n);
    }
}

int dans_mes_coord(node *n, int *pos){
	if ((pos[0] >= n->can_interval[0]) && (pos[0] <= n->can_interval[1])
		   	&& (pos[1] >= n->can_interval[2]) && (pos[1] <= n->can_interval[3]))
		return 1;
	return 0;
}

int dans_ses_coord(node_voisin *n, int *pos){
	if ((pos[0] >= n->can_interval[0]) && (pos[0] <= n->can_interval[1])
		   	&& (pos[1] >= n->can_interval[2]) && (pos[1] <= n->can_interval[3]))
		return 1;
	return 0;
}

int adjacent(int *coord, int *vois){
    if(((vois[0] <= coord[0]) && (coord[0] <= vois[1])) || ((vois[0] <= coord[1]) && (coord[1] <= vois[1]))
    || ((coord[1] >= vois[0]) && (vois[0] >= coord[0])) || ((coord[1] >= vois[1]) && (vois[1] >= coord[0]))){
        if((coord[3] + 1== vois[2]) || (coord[2] == vois[3] + 1)){
            return 1;
        }

    }else if(((vois[2] <= coord[2]) && (coord[2] <= vois[3])) || ((vois[2] <= coord[3]) && (coord[3]<= vois[3]))
    || ((coord[3] >= vois[2]) && (vois[2] >= coord[2])) || ((coord[3] >= vois[3]) && (vois[3] >= coord[2]))){
        if((coord[1] + 1 == vois[0]) || (coord[0] == vois[1] + 1) || (coord[1] == vois[1]) || (coord[0] == vois[0])){
            return 1;
        }
    }
    return 0;
}

void data_gen(node *n){
    MPI_Status status;
    int i, rank;
    int ab[NB_POS];
    for(i = 0; i < 10 * N; i++){
        ab[0] = rand() % MAXABS;
        ab[1] = rand() % MAXABS;
        ab[2] = ab[0] + ab[1];
        if((i < 5) || (i > ((N*10) - 5))){
           memcpy(n->n_data[n->nb_data].pos, ab, NB_POS * sizeof(int));
           n->nb_data++;
        }
        rank = 0;
        MPI_Send(&rank, 1, MPI_INT, 1, INS_DATA, MPI_COMM_WORLD);
        MPI_Send(ab, NB_POS, MPI_INT, 1, INS_DATA, MPI_COMM_WORLD);
        MPI_Recv(&rank, 1, MPI_INT, MPI_ANY_SOURCE, INS_DATA_ACK, MPI_COMM_WORLD, &status);
    }
}



void data_search(node * n){
    int i;
    int rank;
    int pos[NB_POS], pos_al[NB_POS];
    MPI_Status status;

    for (i = 0; i < n->nb_data; i++) {
        printf("Coordinateur > je cherche (%d, %d).\n", n->n_data[i].pos[0], n->n_data[i].pos[1]);
        MPI_Send(&n->rank, 1, MPI_INT, 1, SEARCH_DATA, MPI_COMM_WORLD);
        MPI_Send(n->n_data[i].pos, NB_POS, MPI_INT, 1, SEARCH_DATA, MPI_COMM_WORLD);
        MPI_Recv(&rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_TAG == FOUND){
            printf("Coordinateur > Ma requête (%d, %d) a été trouvée.\n", n->n_data[i].pos[0], n->n_data[i].pos[1]);
            MPI_Recv(pos, NB_POS, MPI_INT, status.MPI_SOURCE, FOUND, MPI_COMM_WORLD, &status);
            printf("Coordinateur > La donnéé (%d, %d) %d dans le noeud %d.\n", pos[0], pos[1], pos[2], status.MPI_SOURCE);
        }else if(status.MPI_TAG == NOT_FOUND){
            printf("Coordinateur > Ma requête (%d, %d) n'a pas été trouvée.\n", n->n_data[i].pos[0], n->n_data[i].pos[1]);
        }
    }
    sleep(1);
    printf("\nCoordinateur > Recherche aléatoire.\n");
    for(i = 0; i < 4; i++){
        pos_al[0] = rand() % MAXABS;
        pos_al[1] = rand() % MAXORD;
        printf("Coordinateur > je cherche (%d, %d).\n", pos[0],pos[1]);
        MPI_Send(&n->rank, 1, MPI_INT, 1, SEARCH_DATA, MPI_COMM_WORLD);
        MPI_Send(pos_al, NB_POS, MPI_INT, 1, SEARCH_DATA, MPI_COMM_WORLD);
        MPI_Recv(&rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_TAG == FOUND){
            printf("Coordinateur > Ma requête (%d, %d) a été trouvée.\n", pos_al[0], pos_al[1]);
            MPI_Recv(pos, NB_POS, MPI_INT, status.MPI_SOURCE, FOUND, MPI_COMM_WORLD, &status);
            printf("Coordinateur > La donnéé (%d, %d) %d dans le noeud %d.\n", pos[0], pos[1], pos[2], status.MPI_SOURCE);
        }else if(status.MPI_TAG == NOT_FOUND){
            printf("Coordinateur > Ma requête (%d, %d) n'a pas été trouvée.\n", pos_al[0], pos_al[1]);
        }
    }
}


void coord_exec(node * n){
    int i;
	int rang;
    MPI_Status status;

    // Etape d'insertion des noeuds
    sleep(2);
	printf("\n");
    for(i = 1; i < N; i++){
		printf("\nCoordinateur > envoie INS à %d\n", i);
		MPI_Send(&n->rank, 1, MPI_INT, i, INS, MPI_COMM_WORLD);
		printf("Coordinateur > reception IOK");
		MPI_Recv(&rang, 1, MPI_INT, i, IOK, MPI_COMM_WORLD, &status);
		printf(" de %d\n", status.MPI_SOURCE);
    }
	printf("\nCoordinateur > Tous les noeuds ont été rajoutés\n");
    sleep(2);
    data_gen(n);
    sleep(2);
    printf("\nCoordinateur > Fin insertion!!!\n");
    sleep(2);
    printf("\nCoordinateur > Recherche des données\n");
    data_search(n);
    sleep(2);
    printf("\nCoordinateur > Fin Recherche des données\n");
}

int search(node *n, int *pos){
    int i;
    for(i= 0; i < n->nb_data; i++){
        if((n->n_data[i].pos[0] == pos[0]) && (n->n_data[i].pos[1] == pos[1])){
            MPI_Send(&n->rank, 1, MPI_INT, 0, FOUND, MPI_COMM_WORLD);
            MPI_Send(n->n_data[i].pos, NB_POS, MPI_INT, 0, FOUND, MPI_COMM_WORLD);
            return 1;
        }
    }
    MPI_Send(&n->rank, 1, MPI_INT, 0, NOT_FOUND, MPI_COMM_WORLD);
    return 0;
}

void node_exec(node *n){
    int pos[NB_POS];
	int coord[NB_COORD];
	int i;
	int rang;
    MPI_Status status;
    
	MPI_Recv(&rang, 1, MPI_INT, 0, INS, MPI_COMM_WORLD, &status);
	MPI_Send(&n->rank, 1, MPI_INT, 1, INS, MPI_COMM_WORLD); 
	MPI_Send(n->pos, NB_POS, MPI_INT, 1, INS, MPI_COMM_WORLD);
	
    // Réception des messages d'insertion et affectation à la structure
	MPI_Recv(pos, NB_POS, MPI_INT, MPI_ANY_SOURCE, INFO, MPI_COMM_WORLD, &status);
	MPI_Recv(coord, NB_COORD, MPI_INT, MPI_ANY_SOURCE, INFO, MPI_COMM_WORLD, &status);
	memcpy(n->pos, pos, NB_POS * sizeof(int));
	memcpy(n->can_interval, coord, NB_COORD * sizeof(int));

	printf("%d > Node Exec: Mes positions %d, %d. Après INFO\n",n->rank, n->pos[0], n->pos[1]);
	printf("%d > Node Exec: Mes coordonnées CAN ((%d,%d);(%d,%d))\n", n->rank, n->can_interval[0], n->can_interval[1], n->can_interval[2], n->can_interval[3]);
	// Boucle sur les insertions des autres noeuds, 
	// sur l'insertion de données et sur la recherche de celles-ci
	for(;;) {
		memset(&rang, 0, sizeof(int));
		memset(&pos, 0, NB_POS * sizeof(int));
		memset(&coord, 0, NB_COORD * sizeof(int));
		MPI_Recv(&rang, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		switch(status.MPI_TAG){
			case INS:
    			MPI_Recv(pos, NB_POS, MPI_INT, MPI_ANY_SOURCE, INS, MPI_COMM_WORLD, &status);
    			printf("%d > Node Exec: Requête reçue (%d, %d), %d\n", n->rank, pos[0], pos[1], rang);
    			if(dans_mes_coord(n, pos)){
    				printf("%d > Node Exec: Dans mes positions => Insertion\n", n->rank);
    				insertion(n, &rang, pos, &status);
    				printf("%d > Node Exec: Mes coordonnées CAN ((%d,%d);(%d,%d)) après INS \n", n->rank,
    						n->can_interval[0], n->can_interval[1], n->can_interval[2], n->can_interval[3]);
    			}else{
    				routage(n, &rang, pos, &status, INS);
    			}
				break;
            case IOK:
	            printf("%d > Node Exec: Nombre de voisins après IOK %d\n", n->rank, n->nb_voisins);
				MPI_Send(&n->rank, 1, MPI_INT, 0, IOK, MPI_COMM_WORLD); // On communique au coordinateur     
				break;
			case PERTE:
				if (n->voisins[rang].rank != -1) {	
					n->voisins[rang].rank = -1;
					n->nb_voisins--;
				}
				break;
			case GAIN:
				if (n->voisins[rang].rank == -1) {
					n->voisins[rang].rank = rang;
					n->nb_voisins++;
				}
				MPI_Recv(pos, NB_POS, MPI_INT, MPI_ANY_SOURCE, GAIN, MPI_COMM_WORLD, &status);
				MPI_Recv(coord, NB_COORD, MPI_INT, MPI_ANY_SOURCE, GAIN, MPI_COMM_WORLD, &status);
				memcpy(n->voisins[rang].pos, pos, NB_COORD * sizeof(int));
				memcpy(n->voisins[rang].can_interval, coord, NB_COORD * sizeof(int));
				break;
			case CAN_VOIS:
				MPI_Recv(coord, NB_COORD, MPI_INT, MPI_ANY_SOURCE, CAN_VOIS, MPI_COMM_WORLD, &status);
				if ((n->voisins[rang].rank != -1) && (rang != n->rank)){
					//printf("%d > Node Exec: CAN_VOIS mise à jour de %d\n", n->rank, rang);
					memcpy(n->voisins[rang].can_interval, coord, NB_COORD * sizeof(int));
					if(!adjacent(n->voisins[rang].can_interval, n->can_interval)){
                        printf("%d > CAN_VOIS le noeud %d n'est plus mon voisin\n", n->rank, rang);
						n->voisins[rang].rank = -1;
						n->nb_voisins--;
					}
				}
				break;
			case INS_DATA:
                MPI_Recv(pos, NB_POS, MPI_INT, MPI_ANY_SOURCE, INS_DATA, MPI_COMM_WORLD, &status);
                if(dans_mes_coord(n, pos)){
                    printf("%d > INS_DATA dans mes intervalles (%d, %d)\n", n->rank, pos[0], pos[1]);
                    insertion_data(n, pos);
                }else{
                    routage(n, &rang, pos, &status, INS_DATA);
                }
				break;
			case SEARCH_DATA:
                MPI_Recv(pos, NB_POS, MPI_INT, MPI_ANY_SOURCE, SEARCH_DATA, MPI_COMM_WORLD, &status);
                if(dans_mes_coord(n, pos)){
                    search(n, pos);
                }else{
                    routage(n, &rang, pos, &status, SEARCH_DATA);
                }
				break;
			default:
				break;
		}
	}	
}

void bootstrap_exec(node *n){
    int i;
	int rang;
	int pos[NB_POS];
	int coord[NB_COORD];
    MPI_Status status;
    
    // Premiere insertion
    MPI_Recv(&rang, 1, MPI_INT, 0, INS, MPI_COMM_WORLD, &status); //Message du coordinateur
    n->can_interval[0] = 0;
    n->can_interval[1] = MAXABS;
    n->can_interval[2] = 0;
    n->can_interval[3] = MAXORD;
	MPI_Send(&n->rank, 1, MPI_INT, 0, IOK, MPI_COMM_WORLD);

	for(;;){
		memset(&rang, 0, sizeof(int));
		memset(&pos, 0, NB_POS * sizeof(int));
		memset(&coord, 0, NB_COORD * sizeof(int));
		MPI_Recv(&rang, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		switch(status.MPI_TAG){
			case INS:
				MPI_Recv(pos, NB_POS, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				printf("\nBootstrap > Requête reçue, (%d, %d), %d\n", pos[0], pos[1], rang);
				if(dans_mes_coord(n, pos)){ // Si on insère dans la plage du bootstrap
					printf("Bootstrap > Dans mes coordonnées => Insertion.\n");
					insertion(n, &rang, pos, &status);
					printf("Bootstrap > Mes coordonnées CAN ((%d,%d);(%d,%d))\n", n->can_interval[0],
						   n->can_interval[1], n->can_interval[2], n->can_interval[3]);
				}else{
					routage(n, &rang, pos, &status, INS);
				}
				break;
			case PERTE:
				if (n->voisins[rang].rank != -1) {
					n->voisins[rang].rank = -1;
					n->nb_voisins--;
				}
				break;
			case GAIN:
				if (n->voisins[rang].rank == -1) {	
					n->voisins[rang].rank = rang;
					n->nb_voisins++;
				}
				MPI_Recv(pos, NB_POS, MPI_INT, MPI_ANY_SOURCE, GAIN, MPI_COMM_WORLD, &status);
				MPI_Recv(coord, NB_COORD, MPI_INT, MPI_ANY_SOURCE, GAIN, MPI_COMM_WORLD, &status);
				memcpy(n->voisins[rang].pos, pos, NB_POS * sizeof(int));
				memcpy(n->voisins[rang].can_interval, coord, NB_COORD * sizeof(int));
				break;
			case CAN_VOIS:
				MPI_Recv(coord, NB_COORD, MPI_INT, MPI_ANY_SOURCE, CAN_VOIS, MPI_COMM_WORLD, &status);
				if ((n->voisins[rang].rank != -1) && (n->rank != rang)){
					//printf("Bootstrap > CAN_VOIS mise à jour de %d\n", rang);
					memcpy(n->voisins[rang].can_interval, coord, NB_COORD * sizeof(int));
					if(!adjacent(n->voisins[rang].can_interval, n->can_interval)){
                        printf("Bootstrap > CAN_VOIS le noeud %d n'est plus mon voisin\n", rang);
						n->voisins[rang].rank = -1;
						n->nb_voisins--;
					}
				}
				break;
			case INS_DATA:
                MPI_Recv(pos, NB_POS, MPI_INT, MPI_ANY_SOURCE, INS_DATA, MPI_COMM_WORLD, &status);
                if(dans_mes_coord(n, pos)){
                    printf("Bootstrap > INS_DATA dans mes intervalles (%d, %d)\n", pos[0], pos[1]);
                    insertion_data(n, pos);
                }else{
                    routage(n, &rang, pos, &status, INS_DATA);
                }
				break;
			case SEARCH_DATA:
                MPI_Recv(pos, NB_POS, MPI_INT, MPI_ANY_SOURCE, SEARCH_DATA, MPI_COMM_WORLD, &status);
                if(dans_mes_coord(n, pos)){
                    search(n, pos);
                }else{
                    routage(n, &rang, pos, &status, SEARCH_DATA);
                }
				break;
			default:
				break;
		}
	}
}


void routage(node *n, int *rang, int *pos, MPI_Status *status, int type){
	int i;
	int min = MAXABS + 1;
	int noeud = 0;

	for(i = 0; i < N; i++){ //Parcourir les voisins et chercher celui à qui router
		if (n->voisins[i].rank != -1) {
			if(dans_ses_coord(&n->voisins[i], pos)){ // L'insertion se trouve dans un voisin
				noeud = n->voisins[i].rank;
                goto fin;
			}
        }
    }
    if(pos[0] > n->can_interval[1]){ //Il est a droite
        for(i = 0; i < N; i++){
            if(n->voisins[i].rank != -1){
                if(n->voisins[i].can_interval[0] > n->can_interval[1]){
                    noeud = n->voisins[i].rank;
                    goto fin;
                }
            }
        }
    }
    if(pos[0] < n->can_interval[0]){ //Il est a gauche
        for(i = 0; i < N; i++){
            if(n->voisins[i].rank != -1){
                if(n->voisins[i].can_interval[1] < n->can_interval[0]){
                    noeud = n->voisins[i].rank;
                    goto fin;
                }
            }
        }
    }
    if(pos[1] > n->can_interval[3]){ //Il est en haut
        for(i = 0; i < N; i++){
            if(n->voisins[i].rank != -1){
                if(n->voisins[i].can_interval[2] > n->can_interval[3]){
                    noeud = n->voisins[i].rank;
                    goto fin;
                }
            }
        }
    }
    if(pos[1] < n->can_interval[2]){ //Il est en bas
        for(i = 0; i < N; i++){
            if(n->voisins[i].rank != -1){
                if(n->voisins[i].can_interval[3] < n->can_interval[2]){
                    noeud = n->voisins[i].rank;
                    goto fin;
                }
            }
        }
    }
fin:
	if(noeud > 0){
        //printf("%d > Routage vers %d, requête de %d\n", n->rank, noeud, *rang);
        MPI_Send(rang, 1, MPI_INT, noeud, type, MPI_COMM_WORLD);
        MPI_Send(pos, NB_POS, MPI_INT, noeud, type, MPI_COMM_WORLD);
	}else{
		printf("%d> Impossible de router la demande\n", n->rank);
	}
}

void inclu (node * appelant, node_voisin * vois, int dest, int * pos, int * coord) {

    if(dest != vois->rank){
        //Tester sur coord pour rajouter les voisins au nouveau noeud insérer
        if(adjacent(coord, vois->can_interval)){
            //Rajouter un voisin
            printf("%d > J'envois à %d son nouveau voisin %d\n", appelant->rank, dest, vois->rank);
        	MPI_Send(&vois->rank, 1, MPI_INT, dest, GAIN, MPI_COMM_WORLD);
        	MPI_Send(vois->pos, NB_POS, MPI_INT, dest, GAIN, MPI_COMM_WORLD);
        	MPI_Send(vois->can_interval, NB_COORD, MPI_INT, dest, GAIN, MPI_COMM_WORLD);

        	printf("%d > J'envois à %d son nouveau voisin %d\n", appelant->rank, vois->rank, dest);
        	MPI_Send(&dest, 1, MPI_INT, vois->rank, GAIN, MPI_COMM_WORLD);
        	MPI_Send(pos, NB_POS, MPI_INT, vois->rank, GAIN, MPI_COMM_WORLD);
        	MPI_Send(coord, NB_COORD, MPI_INT, vois->rank, GAIN, MPI_COMM_WORLD);
        }
        //Tester pour voir si l'appelant perd des voisins
        if(!adjacent(vois->can_interval, appelant->can_interval)){
            printf("%d > J'envois à %d qui me perd comme voisin\n", appelant->rank, vois->rank);
            MPI_Send(&appelant->rank, 1, MPI_INT, vois->rank, PERTE, MPI_COMM_WORLD);
            appelant->nb_voisins--;
            appelant->voisins[vois->rank].rank = -1;
        }
    }
}

//Broadcast à tous sauf à moi même ni au voisin qui vient d'être inséré
void maj_can_interval (node *n, int node_id){
	int i;
	int r = n->rank;
	for(i = 1; i < node_id; i++){
		if((n->rank != i) && (n->voisins[i].rank != -1)){
			//printf("%d > J'envois mes nouvelles coord à %d\n", n->rank, i);
			MPI_Send(&r, 1, MPI_INT, i, CAN_VOIS, MPI_COMM_WORLD);
			MPI_Send(n->can_interval, NB_COORD, MPI_INT, i, CAN_VOIS, MPI_COMM_WORLD);
		}
	}
}

void insertion_data(node *n, int *pos){
    memcpy(n->n_data[n->nb_data].pos, pos, NB_POS * sizeof(int));
    n->nb_data++;
    printf("%d > J'informe le coordinateur de l'insertion des données\n", n->rank);
    MPI_Send(&n->rank, 1, MPI_INT, 0, INS_DATA_ACK, MPI_COMM_WORLD);
}

void insertion(node *n, int *rang, int *pos, MPI_Status *status){
	int i;
	int node_id = *rang;
	// Tester la position du noeud max((a'-a);(b'-b))
    if ((n->can_interval[1] - n->can_interval[0]) >= (n->can_interval[3] - n->can_interval[2])) {
        // Si (a'-a) est le max et si a > (a'-a) / 2 et a' < (a'-a) / 2 (ou l'inverse) dans ce cas on peux diviser l'espace
		if ((pos[0] <= (((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0])) 
        		&& (n->pos[0] >  (((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0]))) {
            // on divise l'espace selon les abscisses
			n->voisins[node_id].pos[0] = pos[0];
            n->voisins[node_id].can_interval[0] = n->can_interval[0]; //int_coord[0];
            n->voisins[node_id].can_interval[1] = ((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0]; //int_coord[1]
            n->voisins[node_id].can_interval[2] = n->can_interval[2]; //int_coord[2];
            n->voisins[node_id].can_interval[3] = n->can_interval[3]; //int_coord[3];

            n->can_interval[0] = (((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0]) + 1;
    	}
		else if ((pos[0] > (((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0])) 
                && (n->pos[0] <=  (((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0]))) {
            // on divise l'espace selon les abscisses
			n->voisins[node_id].pos[0] = pos[0];
            n->voisins[node_id].can_interval[0] = (((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0]) + 1;
            n->voisins[node_id].can_interval[1] = n->can_interval[1];
            n->voisins[node_id].can_interval[2] = n->can_interval[2];
            n->voisins[node_id].can_interval[3] = n->can_interval[3];

			n->can_interval[1] = (((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0]);
        }
		else {
		// le noeud a insérer se trouve dans la même partie que le noeud qui effectue l'insertion
			if (n->pos[0] <= (((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0])) {
				n->voisins[node_id].pos[0] = 
						(rand()%(n->can_interval[1] + 1 - ((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0] + 1)) 
						+ (((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0] + 1);
            	n->voisins[node_id].can_interval[0] = ((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0] + 1;
				n->voisins[node_id].can_interval[1] = n->can_interval[1];
	            n->voisins[node_id].can_interval[2] = n->can_interval[2];
	            n->voisins[node_id].can_interval[3] = n->can_interval[3];

	            n->can_interval[1] = ((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0];
			}
			else {
				n->voisins[node_id].pos[0] =
				   	(rand()%((((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0])  + 1 - n->can_interval[0])) 
						+ n->can_interval[0];
	            n->voisins[node_id].can_interval[0] = n->can_interval[0];
    	       	n->voisins[node_id].can_interval[1] = (((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0]);
        		n->voisins[node_id].can_interval[2] = n->can_interval[2];
	            n->voisins[node_id].can_interval[3] = n->can_interval[3];

	            n->can_interval[0] = ((n->can_interval[1] - n->can_interval[0]) / 2) + n->can_interval[0] + 1;
			}
		}
		n->voisins[node_id].pos[1] = pos[1];
    }
	else { // ((n->can_interval[1] - n->can_interval[0]) < (n->can_interval[3] - n->can_interval[2])){
    // Si (b'-b) est le max et si b > (b'-b) / 2 et b' < (b'-b) / 2 (ou l'inverse) dans ce cas on divise l'espace selon les ordonnées
        if ((pos[1] <= (((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2]))
                && (n->pos[1] >  (((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2]))) {
            // on divise l'espace selon les ordonnées
				n->voisins[node_id].pos[1] = pos[1];
            n->voisins[node_id].can_interval[0] = n->can_interval[0];
            n->voisins[node_id].can_interval[1] = n->can_interval[1];
            n->voisins[node_id].can_interval[2] = n->can_interval[2];
            n->voisins[node_id].can_interval[3] = ((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2];

            n->can_interval[2] = ((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2] + 1;
        }
		else if ((pos[1] > (((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2])) 
                && (n->pos[1] <=  (((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2]))) {
            // on divise l'espace selon les ordonnées
			n->voisins[node_id].pos[1] = pos[1];
            n->voisins[node_id].can_interval[0] = n->can_interval[0];
            n->voisins[node_id].can_interval[1] = n->can_interval[1];
            n->voisins[node_id].can_interval[2] = ((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2] + 1;
            n->voisins[node_id].can_interval[3] = n->can_interval[3];

			n->can_interval[3] = ((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2];
        }
		else {
			if (n->pos[1] <= (((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2])) {
				n->voisins[node_id].pos[1] =
				   	(rand()%(n->can_interval[3] + 1 - (((n->can_interval[3] - n->can_interval[2]) / 2) + 1 + n->can_interval[2])))
					+ (((n->can_interval[3] - n->can_interval[2]) / 2) + 1 + n->can_interval[2]);
            	n->voisins[node_id].can_interval[0] = n->can_interval[0];
    	     	n->voisins[node_id].can_interval[1] = n->can_interval[1];
        	  	n->voisins[node_id].can_interval[2] = ((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2] + 1;
            	n->voisins[node_id].can_interval[3] = n->can_interval[3];

	            n->can_interval[3] = ((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2];
			}
			else {
				n->voisins[node_id].pos[1] =
					(rand()%(((n->can_interval[3] - n->can_interval[2]) / 2 + n->can_interval[2]) + 1 - n->can_interval[2]))
					+ (n->can_interval[2]);
    	       	n->voisins[node_id].can_interval[0] = n->can_interval[0];
            	n->voisins[node_id].can_interval[1] = n->can_interval[1];
            	n->voisins[node_id].can_interval[2] = n->can_interval[2];
            	n->voisins[node_id].can_interval[3] = ((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2];

    	    	n->can_interval[2] = ((n->can_interval[3] - n->can_interval[2]) / 2) + n->can_interval[2] + 1;
			}
		} 
		n->voisins[node_id].pos[0] = pos[0];
	}
	//Envoyer les infos au noeud qui est inséré
	MPI_Send(n->voisins[node_id].pos, NB_POS, MPI_INT, node_id, INFO, MPI_COMM_WORLD);
	MPI_Send(n->voisins[node_id].can_interval, NB_COORD, MPI_INT, node_id, INFO, MPI_COMM_WORLD);
	// envoie des informations sur les noeuds voisins
	MPI_Send(&n->rank, 1, MPI_INT, node_id, GAIN, MPI_COMM_WORLD);	
	MPI_Send(n->pos, NB_POS, MPI_INT, node_id, GAIN, MPI_COMM_WORLD);
	MPI_Send(n->can_interval, NB_COORD, MPI_INT, node_id, GAIN, MPI_COMM_WORLD);
    n->voisins[node_id].rank = node_id;
	n->nb_voisins++;
	
	for (i = 0; i < N; i++) {
		if (n->voisins[i].rank != -1) {
	  		inclu(n, &n->voisins[i], node_id, n->voisins[node_id].pos, n->voisins[node_id].can_interval);
        }
    }
	maj_can_interval(n, node_id);

	MPI_Send(&n->rank, 1, MPI_INT, node_id, IOK, MPI_COMM_WORLD);
}

int main(int argc, char **argv){
    node the_node;
    int my_rank;
    int size;
    
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    node_initialize(&the_node, my_rank);

    MPI_Finalize();
    return 0;
}
