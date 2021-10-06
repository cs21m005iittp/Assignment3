
#include <mpi.h>
#include <stdio.h>
//#include <iostream>
#include <chrono>
//using namespace std::chrono;

double random_number_generator()
{
   double lbound = 0;
   double ubound = 10;
   uniform_real_distribution<double> urd(lbound, ubound);
   default_random_engine re;
   //for (int i = 0; i < 10; i++)
     return urd(re);

}

void example_timer_usage()
{
  auto start = high_resolution_clock::now();
  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(stop - start);
  //std::cout << duration.count() << std::endl;
}


int main(int argc, char** argv) {
    // Initialize the MPI environment
    
    
    MPI_Init(&argc, &argv);
//printf("%d,%c",argc,** argv);


    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
	
	//framework
	/*
	1.one process will generate mtrix of doubles
	i.append ghost eleemnts
	2.for all matrices divide matrix as per process index values.
	
	*/
	
    
    int matrixDim=atoi(argv[1]);
	double Threshold=atoi(argv[2]);
    int remaining_elements=((matrixDim+1)*(matrixDim+1))%world_size;//these are elements which are not equally distributed hence creating imbalance
	//solution 
	/*
	1.remaining eleemnts<world_size
	2.check for world_size-world_rank<=remaining eleemnts if true then add 1 to total elemts assigned to process 
	*/
    int end;
	double data[matrixDim+1][matrixDim+1];//crea
	int i,j;
	if (world_rank==0)
	{
		//addiing zeros to border of matrix
		
		for( j=0;j<matrixDim+1;j++)//1st row
	{
	    data[0][j]=0;
	}
	
	for( j=0;j<matrixDim+1;j++)//last row
	{
	    data[matrixDim][j]=0;
	}
	
		for( j=0;j<matrixDim+1;j++)//1st column
	{
	    data[j][0]=0;
	}
	
	for( j=0;j<matrixDim+1;j++)//last column
	{
	    data[j][matrixDim]=0;
	}
	
	for( i=1;i<matrixDim;i++)//creating matrix of doubles it should be
	{
		for( j=1;j<matrixDim;j++)
	{
	    data[i][j]=random_number_generator();
	}
	
	}
	
	//send signal to all other process that matrix generated
	}
	else
	{
		//wait for process zero to send signal of matrix generation
	}
    //all process execute below code
	//identify how much elemnts(int value) each process need to calculate
	

    // Print off a hello world message
  //  printf("Hello world from processor %s, rank %d out of %d processors\n",
  //         processor_name, world_rank, world_size);
           
        //   printf("%d\n ",n);
           
           int start=(n/world_size)*world_rank;
           
           //assign extra elements to last process to take care of
    if(world_rank==(world_size-1))
   	{
   		  end=(n/world_size)*(world_rank+1);
   		end+=remaining_elements;
   	}
    else{
     end=(n/world_size)*(world_rank+1);
    }
           
		   
		   int local_sum;
		   for( i=start;i<end;i++)
		   {
				local_sum +=data[i];
		   }
 printf("local_sum %d, rank %d out of %d processors\n",
           local_sum, world_rank, world_size);
		   
		   int total_sum;
		   
		   MPI_Reduce(&local_sum,&total_sum,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
		   
		   if(world_rank==0)
		   {
				printf("total_sum %d\n",total_sum);
		   }
    // Finalize the MPI environment.
    MPI_Finalize();
}