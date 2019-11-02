#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<mpi.h>
#define rows 4
#define columns 4 

int main(){

	int size, rank;
	int **arr;
	int **local_arr;
	int *vector;
	int *res_vector;
    int i;
	int j;
	int row_each;
	MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0){
    	srand(0);
    	// matrix declaration
    	arr = (int**)malloc(sizeof(int*) * rows);
    	for(i=0;i<rows;i++)
    		arr[i] = (int*)malloc(sizeof(int) * columns);
    	//matrix intiliazation
        for(i=0;i<rows;i++)
    		for(j=0;j<columns;j++)
    			arr[i][j] = (rand() % 10) + 1;
        //matrix printing
        for(i=0;i<rows;i++)
            for(j=0;j<columns;j++)
                printf("rank:%d, i:%d, j:%d, arr[i][j]:%d\n", rank, i, j, arr[i][j]);

        //vector initialization and declaration
    	vector = (int*)malloc(sizeof(int) * columns);
    	for(i=0; i<columns; i++)
    		vector[i] = (rand() % 10) + 1;

        //result vector initialization and declaration
        res_vector = (int*)calloc(columns, sizeof(int));

        //printing a vector
        for(i=0; i<columns; i++)
            printf("rank:%d, vector[%d]:%d\n",rank, i, vector[i]);
        
        //how many rows each process got?
        row_each = rows / size;
        
        //let every process know how many rows it got
        MPI_Bcast( &row_each, 1, MPI_INT, 0, MPI_COMM_WORLD);
    	
        //let's broadcast vector to each process
    	MPI_Bcast( vector, columns, MPI_INT, 0, MPI_COMM_WORLD);
    	
        //sending row_each rows to child processes
		for(i = 1; i < size ; i++)
    		for(j=0; j<row_each; j++)
    			MPI_Send(arr[j + (i*row_each)], columns, MPI_INT, (i), 0, MPI_COMM_WORLD);
		
      

        //hybridization i.e openmp code 
        #pragma omp parallel num_threads(row_each)
        {
        #pragma omp for schedule(static, 1)
        for(i =0 ;i < row_each; i++){
             for(j=0; j<columns; j++){
                res_vector[i] += vector[j] * arr[i][j];
            }
        }
    }

        //receiving values from a child process
        for(i = 1; i < size ; i++)
            for(j=0; j<row_each; j++)
                MPI_Recv(&res_vector[(i*2) + j], 1, MPI_INT, i, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
      
        //printing final result
        printf("final result is as follows:\n");
        for(i =0;i<columns;i++)
            printf("%d\n",res_vector[i]); 

      }
	else{
		
		vector = (int*)malloc(sizeof(int) * columns);
		MPI_Bcast( &row_each, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast( vector, columns, MPI_INT, 0, MPI_COMM_WORLD);
        res_vector = (int*)calloc(row_each, sizeof(int));
		
        //creating a space and receiving values
        local_arr = (int**)malloc(sizeof(int*) * row_each);
		for(i=0;i<row_each;i++)
    		local_arr[i] = (int*)malloc(sizeof(int*) * columns);
		for(i=0;i<row_each;i++)
			MPI_Recv(local_arr[i], columns, MPI_INT , 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		
        // child process printing its rank and  row_each 
		printf(" rank:%d, row_each:%d\n", rank, row_each);
    	
        //child process printing vector
        for(i=0; i<columns; i++)
    		printf("rank:%d, vector[%d]:%d\n",rank, i, vector[i]);
    	
        //child process printing its 2d array
        for(i=0;i<row_each;i++)
    		for(j=0;j<columns;j++)
    			printf("rank:%d, i:%d, j:%d, local_arr:%d\n", rank, i, j, local_arr[i][j]);
    
        
        #pragma omp parallel num_threads(rows_each)
		{
		#pragma omp for schedule(static, 1) 
		for(i =0 ;i < row_each; i++){
			for(j=0; j<columns; j++){
				res_vector[i] += vector[j] * local_arr[i][j];
            }
          }
		}  
       
       for(i=0;i<row_each;i++)
            MPI_Send(&res_vector[i],1,MPI_INT,0, 0,MPI_COMM_WORLD);
}    

MPI_Finalize();
return 0;
}