#include<iostream>
//#include <ostream>
#include <climits>
#include <cstdlib>
#include <mpi.h>
#include <omp.h>
#include <math.h>
#include <vector>
#include <stdio.h>
using namespace std;


double random_number_generator()
{
   double lbound = 0;
   double ubound = 10;
  
     return rand()%11;

}
struct Neighbour
{
	int row,column;
	double data;
};

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
	
	
	//printf("starting..");
    
    int matrixDim=atoi(argv[1]);
	vector<int> row;
	vector<int> column;
	vector<double> data;
	//vector<double> maxDifferences;//used by p0 only
	Neighbour N1,N2,N3,N4;
	double maxDifference=0,oldValue=0;
	double globalMaxDifference;
	double Threshold=atof(argv[2]);
	printf("fetching inputs..");
    int remaining_elements=((matrixDim)*(matrixDim))%world_size;//these are elements which are not equally distributed hence creating imbalance
	//solution 
	/*
	1.remaining eleemnts<world_size
	2.check for world_size-world_rank<=remaining eleemnts if true then add 1 to total elemts assigned to process 
	*/
    int end;
	
	int i,j,test=0;
	int elements=((matrixDim)*(matrixDim))/world_size;
	int elementsAllocatedToMe=elements;
	int allocatedElementByNeibour=0;
	int row_index,column_index;
	//checking below wether current process need to have extra elements
	if(remaining_elements>0)
	{
		
		if((world_size-world_rank)<=remaining_elements)//checking here remaining element distributed in current process or not
		{
			elementsAllocatedToMe+=1;
		}
	}
	
	//calculating elements taken by neibour allocatedElementByNeibour
	for(int i=0;i<=world_rank-1;i++)
	{
		if((world_size-i)<=remaining_elements)//checking here remaining element distributed in current process or not(for balanced distribution)
		{
			allocatedElementByNeibour+=1+elements;
		}
		else{
			allocatedElementByNeibour+=elements;
		}
	}
	row_index=ceil(allocatedElementByNeibour/matrixDim);
	column_index=(allocatedElementByNeibour)%matrixDim;
	
	//adding elements to row vectoe column vector and generating random values at data vector
	
	for(int i=0;i<elementsAllocatedToMe;i++)
	{
		if(column_index>matrixDim-1)
		{
			row_index++;
			column_index=0;
		}
		if(row_index<=matrixDim-1&&column_index<=matrixDim-1)
		{
			row.push_back(row_index);
			column.push_back(column_index++);
			data.push_back(random_number_generator());
			
		}
	}
	
	//checking balanced distribution of data
	cout<<endl;
	
	printf("I processor %s, rank %d out of %d processors received \n",
           processor_name, world_rank, world_size);
	cout<<"\nrows indexs:";
	for(int i=0;i<elementsAllocatedToMe;i++)
            {
                 cout<<" "<<row[i];
            }
	cout<<"\ncolumn indexs:";
	for(int i=0;i<elementsAllocatedToMe;i++)
            {
                 cout<<" "<<column[i];
            }
	
	cout<<"\nData values:";
	for(int i=0;i<elementsAllocatedToMe;i++)
            {
                 cout<<" "<<data[i];
            }
	//check done
	
	//for each element from elements
	/*                             N1 
	1.check for corner cases       :	
	 i.row==0&column==0-----> N4<-data-->N2  N1=0,N4=0 for this case
	 				:
	                              N3
	  
	                               N1 
	1.check for corner cases       :	
	 i.row==0&column==n-----> N4<-data-->N2  N1=0,N2=0 for this case
	 				:
	                              N3                            
	 
	                               N1 
	1.check for corner cases       :	
	 i.row==n&column==0-----> N4<-data-->N2  N3=0,N4=0 for this case
	 				:
	                              N3                            
	
	                               N1 
	1.check for corner cases       :	
	 i.row==n&column==n-----> N4<-data-->N2  N3=0,N2=0 for this case
	 				:
	                              N3                            

	
	                              
      	where N1,N2,N3,N4 are neibours
	*/
	/*
	in below for loop we are calculating for each element 
	we have 3 cases
	I]it is one of 4 corner elements
	II]It is boundry element which is not corner eleement
	III]its element properly resides in matrix neither at Ith nor IIth case
	
	1.identify its neibour
	2.sending data to neibour
	3.receiving data from neibour
	4.updating average n place of data
	
	*/
	
	while(true)
	{
	cout<<"entered in while loop";
	MPI_Barrier(MPI_COMM_WORLD);
	for(int i=0;i<elementsAllocatedToMe;i++)//
	{
	   bool N1_avail=false;
	   bool N2_avail=false;
	   bool N3_avail=false;
	   bool N4_avail=false;
	   
	   //case I as mentioned above
	   if(row[i]==0&&column[i]==0)//leftmost,topmost element
	   {
	   	N1.data=0;
	   	N1_avail=true;
	   	N4.data=0;
	   	N4_avail=true;
	   	N2.row=row[i];
	   	N2.column=column[i]+1;
	   	N3.row=row[i]+1;
	   	N3.column=column[i];
	   	
	   	//check N2 and N3 belongs to me only(current process)
	   	#pragma omp parallel
	   	for(int j=0;j<elementsAllocatedToMe;j++)
	   	{
	   	   if(row[j]==N3.row&&column[j]==N3.column)
	   	   {
	   	   	N3_avail=true;
	   	   }
	   	   
	   	   if(row[j]==N2.row&&column[j]==N2.column)
	   	   {
	   	   	N2_avail=true;
	   	   }
	   	}
	   	
	   	//check N3 available
		int N3_process,N2_process;
	   	if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N3_process=world_rank+ceil(matrixDim/elements);
	   		 cout<<"corner 1 N3_process:"<<N3_process;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD);
	   	}
	   	if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N2_process=world_rank+1;
	   		 cout<<"corner 1 N2_process:"<<N2_process;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD);
	   	}
	   	
	   	if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N3.data,1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N3_avail=true;
	   		cout<<"corner 1 N3_process: data received"<<N3_process<<" "<<N3.data;
	   		
	   	}
	   	if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		MPI_Recv(&N2.data,1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N2_avail=true;
	   	}
	   	
	   	if(N3_avail&&N2_avail)
	   	{
	   		//means here all data available
	   		//calculate average of eleemnt and update
	   		oldValue=data[i];
	   		data[i]=(N1.data+N2.data+N3.data+N4.data+data[i])/5;
	   		if(maxDifference<abs(oldValue-data[i]))
	   		{
	   		   maxDifference=abs(oldValue-data[i]);
	   		}
	   		
	   	}
	   	
	   	continue;
	   }//end of 1st corner case
	   
	   
	   //start case II
	   /*
	     
	                               N1 
				        :	
	 i.row==0&column==n-----> N4<-data-->N2  N1=0,N2=0 for this case
	 				:
	                              N3                            

	   */
	   	   //case II as mentioned above
	   if(row[i]==0&&column[i]==matrixDim-1)//rightmost,topmost element
	   {
	   	N1.data=0;
	   	N1_avail=true;
	   	N2.data=0;
	   	N2_avail=true;
	   	N4.row=row[i];
	   	N4.column=matrixDim-2;
	   	N3.row=row[i]+1;
	   	N3.column=column[i];
	   	
	   	//check N2 and N3 belongs to me only(current process)
	   	#pragma omp parallel
	   	for(int j=0;j<elementsAllocatedToMe;j++)
	   	{
	   	   if(row[j]==N3.row&&column[j]==N3.column)
	   	   {
	   	   	N3_avail=true;
	   	   }
	   	   
	   	   if(row[j]==N4.row&&column[j]==N4.column)
	   	   {
	   	   	N4_avail=true;
	   	   }
	   	}
	   	
	   	//check N3 available
		int N3_process,N4_process;
	   	if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N3_process=world_rank+ceil(matrixDim/elements);
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		cout<<"corner II N2_process:"<<N3_process;
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD);
	   	}
	   	if(N4_avail==false)//means N4 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N4_process=world_rank-1;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD);
	   	}
	   	
	   	if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N3.data,1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N3_avail=true;
	   		cout<<"corner II N3_process: data received"<<N3_process<<" "<<N3.data;
	   		
	   	}
	   	if(N4_avail==false)//means N4 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		MPI_Recv(&N4.data,1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N4_avail=true;
	   	}
	   	
	   	if(N3_avail&&N4_avail)
	   	{
	   		//means here all data available
	   		//calculate average of eleemnt and update
	   		oldValue=data[i];
	   		data[i]=(N1.data+N4.data+N3.data+N2.data+data[i])/5;
	   		if(maxDifference<abs(oldValue-data[i]))
	   		{
	   		   maxDifference=abs(oldValue-data[i]);
	   		}
	   		
	   	}
	   	
	   continue;	
	   }//end of 2nd corner case
	   
	   //start of 3rd corner case
	   /*
	   	                           N1 
	1.check for 3rd corner cases   :	
	 i.row==n&column==0-----> N4<-data-->N2  N3=0,N4=0 for this case
								   :
	                               N3                            

	   */
	   	   	   //case III as mentioned above
	   if(row[i]==matrixDim-1&&column[i]==0)//leftmost,bottom element
	   {
	   	N3.data=0;
	   	N3_avail=true;
	   	N4.data=0;
	   	N4_avail=true;
	   	N1.row=row[i]-1;
	   	N1.column=column[i];
	   	N2.row=row[i];
	   	N2.column=column[i]+1;
	   	
	   	//check N1 and N2 belongs to me only(current process)
	   	#pragma omp parallel
	   	for(int j=0;j<elementsAllocatedToMe;j++)
	   	{
	   	   if(row[j]==N1.row&&column[j]==N1.column)
	   	   {
	   	   	N1_avail=true;
	   	   }
	   	   
	   	   if(row[j]==N2.row&&column[j]==N2.column)
	   	   {
	   	   	N2_avail=true;
	   	   }
	   	}
	   	
	   	//check N3 available
		int N1_process,N2_process;
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N1_process=world_rank-ceil(matrixDim/elements);
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD);
	   	}
	   	if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N2_process=world_rank+1;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD);
	   	}
	   	
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N1.data,1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N1_avail=true;
	   		
	   	}
	   	if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		MPI_Recv(&N2.data,1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N2_avail=true;
	   	}
	   	
	   	if(N1_avail&&N2_avail)
	   	{
	   		//means here all data available
	   		//calculate average of eleemnt and update
	   		oldValue=data[i];
	   		data[i]=(N1.data+N4.data+N2.data+N3.data+data[i])/5;
	   		if(maxDifference<abs(oldValue-data[i]))
	   		{
	   		   maxDifference=abs(oldValue-data[i]);
	   		}
	   		
	   	}
	   	
	   continue;	
	   }//end of 3rd corner case
	   
	   //start 4th corner case
	   /*
	                               N1 
	1.check for corner cases       :	
	 i.row==n&column==n-----> N4<-data-->N2  N3=0,N2=0 for this case
	 		                	   :
	                              N3                            

	   
	   */
	   
	   	   if(row[i]==matrixDim-1&&column[i]==matrixDim-1)//leftmost,bottom element
	   {
	   	N3.data=0;
	   	N3_avail=true;
	   	N2.data=0;
	   	N2_avail=true;
	   	N1.row=row[i]-1;
	   	N1.column=column[i];
	   	N4.row=row[i];
	   	N4.column=column[i]-1;
	   	
	   	//check N1 and N2 belongs to me only(current process)
	   	#pragma omp parallel
	   	for(int j=0;j<elementsAllocatedToMe;j++)
	   	{
	   	   if(row[j]==N1.row&&column[j]==N1.column)
	   	   {
	   	   	N1_avail=true;
	   	   }
	   	   
	   	   if(row[j]==N4.row&&column[j]==N4.column)
	   	   {
	   	   	N4_avail=true;
	   	   }
	   	}
	   	
	   	//check N3 available
		int N1_process,N4_process;
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N1_process=world_rank-ceil(matrixDim/elements);
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD);
	   	}
	   	if(N4_avail==false)//means N4 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N4_process=world_rank-1;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD);
	   	}
	   	
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N1.data,1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N1_avail=true;
	   		
	   	}
	   	if(N4_avail==false)//means N4 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		MPI_Recv(&N4.data,1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N4_avail=true;
	   	}
	   	
	   	if(N1_avail&&N4_avail)
	   	{
	   		//means here all data available
	   		//calculate average of eleemnt and update
	   		oldValue=data[i];
	   		data[i]=(N1.data+N4.data+N2.data+N3.data+data[i])/5;
	   		if(maxDifference<abs(oldValue-data[i]))
	   		{
	   		   maxDifference=abs(oldValue-data[i]);
	   		}
	   		
	   	}
	   	
	   continue;	
	   }//end of 4th corner case
	   
	  // MPI_Barrier(MPI_COMM_WORLD);
	  
	  /*
	  //for each element from elements
	/*                              N1 
	1.check for border cases        :	
	 i.row==0&column!=(0 or n) N4<-data-->N2  N1=0 for this case
					:
	                               N3
	  
	  	                       N1 
	1.check for border cases       :	
	 i.row!=(0 or n)&column==n N4<-data-->N2  N2=0 for this case
									:
	                               N3                            

	                               N1 
	1.check for border cases       :	
	 i.row==n&column!=(0 or n) N4<-data-->N2  N3=0 for this case
					:
	                               N3                            


	                               N1 
	1.check for border cases       :	
	 i.row!=(0 or n)&column==0 N4<-data-->N2  N4=0 for this case
					:
					N3                            
	 
	

	
	                              
      	where N1,N2,N3,N4 are neibours
	  
	  */
	  
	  //start of 1st border case
	   if(row[i]==0&&(column[i]!=matrixDim-1&&column[i]!=0))//topmost,border element
	   {
	   	N1.data=0;
	   	N1_avail=true;
		N2.row=row[i];
	   	N2.column=column[i]+1;
	   	N3.row=row[i]+1;
	   	N3.column=column[i];
	   	N4.row=row[i];
	   	N4.column=column[i]-1;
	   	
	   	//check N3 and N2,N4 belongs to me only(current process)
	   	#pragma omp parallel
	   	for(int j=0;j<elementsAllocatedToMe;j++)
	   	{
	   	   if(row[j]==N2.row&&column[j]==N3.column)
	   	   {
	   	   	N2_avail=true;
	   	   }
	   	   
	   	   if(row[j]==N3.row&&column[j]==N3.column)
	   	   {
	   	   	N3_avail=true;
	   	   }
		   
		   if(row[j]==N4.row&&column[j]==N4.column)
	   	   {
	   	   	N4_avail=true;
	   	   }
	   	}
	   	
	   	//check N3 available
		int N2_process,N3_process,N4_process;
	   	if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N2_process=world_rank+1;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		cout<<"border 1 N3_process:"<<N3_process<<endl;
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD);
	   	}
		if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N3_process=world_rank+ceil(matrixDim/elements);
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD);
	   	}
	   	if(N4_avail==false)//means N4 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N4_process=world_rank-1;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD);
	   	}
	   	
	   	if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N2.data,1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N2_avail=true;
	   		
	   	}
		if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N3.data,1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N3_avail=true;
	   		
	   	}
	   	if(N4_avail==false)//means N4 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		MPI_Recv(&N4.data,1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N4_avail=true;
	   	}
	   	
	   	if(N2_avail&&N3_avail&&N4_avail)
	   	{
	   		//means here all data available
	   		//calculate average of eleemnt and update
	   		oldValue=data[i];
	   		data[i]=(N1.data+N4.data+N2.data+N3.data+data[i])/5;
	   		if(maxDifference<abs(oldValue-data[i]))
	   		{
	   		   maxDifference=abs(oldValue-data[i]);
	   		}
	   		
	   	}
	   	
	   continue;	
	   }//end of 1st border case

	   
	   //start of 2nd border case
	   if((row[i]!=matrixDim-1&&row[i]!=0)&&(column[i]==matrixDim-1))//rightmost,border element
	   {
	   	N2.data=0;
	   	N2_avail=true;
		N1.row=row[i]-1;
	   	N1.column=column[i];
	   	N3.row=row[i]+1;
	   	N3.column=column[i];
	   	N4.row=row[i];
	   	N4.column=column[i]-1;
	   	
	   	//check N1 and N3,N4 belongs to me only(current process)
	   	#pragma omp parallel
	   	for(int j=0;j<elementsAllocatedToMe;j++)
	   	{
	   	   if(row[j]==N1.row&&column[j]==N1.column)
	   	   {
	   	   	N1_avail=true;
	   	   }
	   	   
	   	   if(row[j]==N3.row&&column[j]==N3.column)
	   	   {
	   	   	N3_avail=true;
	   	   }
		   
		   if(row[j]==N4.row&&column[j]==N4.column)
	   	   {
	   	   	N4_avail=true;
	   	   }
	   	}
	   	
	   	//check N3 available
		int N1_process,N3_process,N4_process;
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N1_process=world_rank-ceil(matrixDim/elements);//world_rank+1;//
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD);
	   	}
		if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N3_process=world_rank+ceil(matrixDim/elements);
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD);
	   	}
	   	if(N4_avail==false)//means N4 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N4_process=world_rank-1;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD);
	   	}
	   	
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N1.data,1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N1_avail=true;
	   		
	   	}
		if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N3.data,1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N3_avail=true;
	   		
	   	}
	   	if(N4_avail==false)//means N4 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		MPI_Recv(&N4.data,1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N4_avail=true;
	   	}
	   	
	   	if(N1_avail&&N3_avail&&N4_avail)
	   	{
	   		//means here all data available
	   		//calculate average of eleemnt and update
	   		oldValue=data[i];
	   		data[i]=(N1.data+N4.data+N2.data+N3.data+data[i])/5;
	   		if(maxDifference<abs(oldValue-data[i]))
	   		{
	   		   maxDifference=abs(oldValue-data[i]);
	   		}
	   		
	   	}
	   	
	   continue;	
	   }//end of 2nd border case
	   
	   //start of III border case
	   
	   if(row[i]==matrixDim-1&&(column[i]!=matrixDim-1&&column[i]!=0))//bottom,border elements
	   {
	   	N3.data=0;
	   	N3_avail=true;
		N1.row=row[i]-1;
	   	N1.column=column[i];
	   	N2.row=row[i];
	   	N2.column=column[i]+1;
	   	N4.row=row[i];
	   	N4.column=column[i]-1;
	   	
	   	//check N1 and N2,N4 belongs to me only(current process)
	   	#pragma omp parallel
	   	for(int j=0;j<elementsAllocatedToMe;j++)
	   	{
	   	   if(row[j]==N1.row&&column[j]==N1.column)
	   	   {
	   	   	N1_avail=true;
	   	   }
	   	   
	   	   if(row[j]==N2.row&&column[j]==N2.column)
	   	   {
	   	   	N2_avail=true;
	   	   }
		   
		   if(row[j]==N4.row&&column[j]==N4.column)
	   	   {
	   	   	N4_avail=true;
	   	   }
	   	}
	   	
	   	//check N3 available
		int N1_process,N2_process,N4_process;
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N1_process=world_rank-ceil(matrixDim/elements);//world_rank+1;//
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD);
	   	}
		if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N2_process=world_rank+1;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD);
	   	}
	   	if(N4_avail==false)//means N4 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N4_process=world_rank-1;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD);
	   	}
	   	
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N1.data,1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N1_avail=true;
	   		
	   	}
		if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N2.data,1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N2_avail=true;
	   		
	   	}
	   	if(N4_avail==false)//means N4 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		MPI_Recv(&N4.data,1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N4_avail=true;
	   	}
	   	
	   	if(N1_avail&&N2_avail&&N4_avail)
	   	{
	   		//means here all data available
	   		//calculate average of eleemnt and update
	   		oldValue=data[i];
	   		data[i]=(N1.data+N4.data+N2.data+N3.data+data[i])/5;
	   		if(maxDifference<abs(oldValue-data[i]))
	   		{
	   		   maxDifference=abs(oldValue-data[i]);
	   		}
	   		
	   	}
	   	
	   continue;	
	   }//end of 3rd border case
	   
	   //start of IV border case
	   	   if((row[i]!=matrixDim-1&&row[i]!=0)&&(column[i]==0))//leftmost,border elements
	   {
	   	N4.data=0;
	   	N4_avail=true;
		N1.row=row[i]-1;
	   	N1.column=column[i];
	   	N2.row=row[i];
	   	N2.column=column[i]+1;
	   	N3.row=row[i]+1;
	   	N3.column=column[i];
	   	
	   	//check N1 and N2,N3 belongs to me only(current process)
	   	#pragma omp parallel
	   	for(int j=0;j<elementsAllocatedToMe;j++)
	   	{
	   	   if(row[j]==N1.row&&column[j]==N1.column)
	   	   {
	   	   	N1_avail=true;
	   	   }
	   	   
	   	   if(row[j]==N2.row&&column[j]==N2.column)
	   	   {
	   	   	N2_avail=true;
	   	   }
		   
		   if(row[j]==N3.row&&column[j]==N3.column)
	   	   {
	   	   	N3_avail=true;
	   	   }
	   	}
	   	
	   	//check N3 available
		int N1_process,N2_process,N3_process;
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N1_process=world_rank-ceil(matrixDim/elements);//world_rank+1;//
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD);
	   	}
		if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N2_process=world_rank+1;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD);
	   	}
	   	if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N3_process=world_rank+ceil(matrixDim/elements);
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD);
	   	}
	   	
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N1.data,1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N1_avail=true;
	   		
	   	}
		if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N2.data,1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N2_avail=true;
	   		
	   	}
	   	if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		MPI_Recv(&N3.data,1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N3_avail=true;
	   	}
	   	
	   	if(N1_avail&&N2_avail&&N3_avail)
	   	{
	   		//means here all data available
	   		//calculate average of eleemnt and update
	   		oldValue=data[i];
	   		data[i]=(N1.data+N4.data+N2.data+N3.data+data[i])/5;
	   		if(maxDifference<abs(oldValue-data[i]))
	   		{
	   		   maxDifference=abs(oldValue-data[i]);
	   		}
	   		
	   	}
	   	
	   continue;	
	   }//end of IV th border case
	   
	   //below case is for that element which is not at border or at corner
	  	   
	   	  
	   	   if((row[i]!=matrixDim-1&&row[i]!=0)&&(column[i]!=matrixDim-1&&column[i]!=0))//leftmost,border elements
	   {
	   
		N1.row=row[i]-1;
	   	N1.column=column[i];
	   	N2.row=row[i];
	   	N2.column=column[i]+1;
	   	N3.row=row[i]+1;
	   	N3.column=column[i];
		N4.row=row[i];
	   	N4.column=column[i]-1;
	   	
	   	//check N1 and N2,N3,N4 belongs to me only(current process)
	   	#pragma omp parallel
	   	for(int j=0;j<elementsAllocatedToMe;j++)
	   	{
	   	   if(row[j]==N1.row&&column[j]==N1.column)
	   	   {
	   	   	N1_avail=true;
	   	   }
	   	   
	   	   if(row[j]==N2.row&&column[j]==N2.column)
	   	   {
	   	   	N2_avail=true;
	   	   }
		   
		   if(row[j]==N3.row&&column[j]==N3.column)
	   	   {
	   	   	N3_avail=true;
	   	   }
		   
		   if(row[j]==N4.row&&column[j]==N4.column)
	   	   {
	   	   	N4_avail=true;
	   	   }
	   	}
	   	
	   	//check N's available
		int N1_process,N2_process,N3_process,N4_process;
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N1_process=world_rank-ceil(matrixDim/elements);//world_rank+1;//
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		cout<<"non-border non corner case N1_process: data received"<<N1_process<<endl;
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD);
	   	}
		if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N2_process=world_rank+1;
	   		 cout<<"non-border non corner case N1_process: data received"<<N2_process<<endl;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD);
	   	}
	   	if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N3_process=world_rank+ceil(matrixDim/elements);
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		cout<<"non-border non corner case N1_process: data received"<<N3_process<<endl;
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD);
	   	}
		if(N4_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		 N4_process=world_rank-1;
	   		//MPI_Send(void* data,int count,MPI_Datatype datatype,int destination,int tag,MPI_Comm communicator)
	   		cout<<"non-border non corner case N1_process: data received"<<N4_process<<endl;
	   		//send data first
	   		MPI_Send(&data[i],1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD);
	   	}
	   	
	   	if(N1_avail==false)//means N1 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N1.data,1,MPI_DOUBLE,N1_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N1_avail=true;
	   		
	   	}
		if(N2_avail==false)//means N2 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		//MPI_Recv(void* data,int count,MPI_Datatype datatype,int source,int tag,MPI_Comm communicator,MPI_Status* status)
	   		MPI_Recv(&N2.data,1,MPI_DOUBLE,N2_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N2_avail=true;
	   		
	   	}
	   	if(N3_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		MPI_Recv(&N3.data,1,MPI_DOUBLE,N3_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N3_avail=true;
	   	}
		if(N4_avail==false)//means N3 resides in other process 1.identify that process here 2.send data to that process 
	   	{
	   		MPI_Recv(&N4.data,1,MPI_DOUBLE,N4_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	   		N4_avail=true;
	   	}
	   	
	   	if(N1_avail&&N2_avail&&N3_avail&&N4_avail)
	   	{
	   		//means here all data available
	   		//calculate average of eleemnt and update
	   		oldValue=data[i];
	   		data[i]=(N1.data+N4.data+N2.data+N3.data+data[i])/5;
	   		if(maxDifference<abs(oldValue-data[i]))
	   		{
	   		   maxDifference=abs(oldValue-data[i]);
	   		}
	   		
	   	}
	   	
	   continue;	
	   }//end of non-border and non-corner  case
	   

	   
	}
	
	//MPI_Barrier(MPI_COMM_WORLD);
	//all elements average calculated till here
	//MPI_Barrier(MPI_COMM_WORLD);

	cout<<"max Difference: "<<maxDifference<<"at process rank"<<world_rank<<endl;
	   //get maxDifference from all processes
	  MPI_Reduce(&maxDifference,&globalMaxDifference,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
	  
	  //broadcast globalmax diff
  	//other process will receive it
	  
	  {
  cout<<"globalMaxDifference =\n"<< globalMaxDifference;
 // MPI_Bcast(void* data, int count,MPI_Datatype datatype,int root,MPI_Comm communicator)
 MPI_Barrier(MPI_COMM_WORLD);
 //p0 will brodcast what max difference it has received
 MPI_Bcast(&globalMaxDifference, 1,MPI_DOUBLE,0,MPI_COMM_WORLD);
 
  }
  		
  	
  	
  		
	  
	  MPI_Barrier(MPI_COMM_WORLD);
	  if(globalMaxDifference<Threshold)
	  {
		  break;
	  }
	  //setting maxDiff=0 for next STENCIL CALCULATION
	  maxDifference=INT_MIN;
	  
	   //checking  data
	cout<<"inside of while loop "<<endl;
	
	printf("I processor %s, rank %d out of %d processors received \n",
           processor_name, world_rank, world_size);
	cout<<"\nrows indexs:";
	for(int i=0;i<elementsAllocatedToMe;i++)
            {
                 cout<<" "<<row[i];
            }
	cout<<"\ncolumn indexs:";
	for(int i=0;i<elementsAllocatedToMe;i++)
            {
                 cout<<" "<<column[i];
            }
	
	cout<<"\nData values:";
	for(int i=0;i<elementsAllocatedToMe;i++)
            {
                 cout<<" "<<data[i];
            }
	//check done
	  
	}
	//till here stencil computation completed
	
	//checking  data
	cout<<"come out of while loop "<<endl;
	
	printf("I processor %s, rank %d out of %d processors received \n",
           processor_name, world_rank, world_size);
	cout<<"\nrows indexs:";
	for(int i=0;i<elementsAllocatedToMe;i++)
            {
                 cout<<" "<<row[i];
            }
	cout<<"\ncolumn indexs:";
	for(int i=0;i<elementsAllocatedToMe;i++)
            {
                 cout<<" "<<column[i];
            }
	
	cout<<"\nData values:";
	for(int i=0;i<elementsAllocatedToMe;i++)
            {
                 cout<<" "<<data[i];
            }
	//check done
	

       // Finalize the MPI environment.
    MPI_Finalize();
}
