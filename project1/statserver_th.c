#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>

#define MQNAME "/testa1"
#define MQNAMES "/testa2"
#define MAXTHREADS  20
pthread_mutex_t lock;
int globCount[10]; // global count every thread increase its section
int max_arr_thread[10]; // every thread puts its location max
double avg_for_threads[20]; // one for average other is count in the file
int range_arr_thread[10][1000]; // for every thread can put their range largest values
				// 1000 is chosen since it is the max number
int range_total_counts[10];	// in range how many number is taken into array

// struct is taken from moodle zip
struct arg {
	int number_of_file;		// to take the file number
	char **arg_u;			
	int n_arg;			/* min value */
	int m_arg;			/* max value */
	int range_thread; 		// desired count of elements
	int t_index;		/* the index of the created thread */
};

void *count(void *arg_ptr_count)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *fp;

	fp = fopen((((struct arg*)arg_ptr_count)->arg_u)[((struct arg*)arg_ptr_count)->number_of_file] , "r");

	int counts = 0;
	

	
	while((read = getline(&line, &len, fp)) != -1)
	{
		// check the max and min
		if(atoi(line) >= ((struct arg*)arg_ptr_count)->n_arg && atoi(line) <= ((struct arg*)arg_ptr_count)->m_arg)
		{
			counts++;
			
		}
	}
	// since the outputs was different everytime i used mutex
	pthread_mutex_lock(&lock);
	globCount[((struct arg*)arg_ptr_count)->t_index] = counts;
	pthread_mutex_unlock(&lock);
	fclose(fp); // close the file

	pthread_exit(NULL);	// kill the thread
}

void *maxFun(void *arg_ptr_max )
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *fp;


	

	fp = fopen((((struct arg*)arg_ptr_max)->arg_u)[((struct arg*)arg_ptr_max)->number_of_file] , "r");
	
	int max = 0;
	
	while((read = getline(&line, &len, fp)) != -1)
	{
		if(atoi(line) > max)
		{
			max = atoi(line);
		}
	}
	
	pthread_mutex_lock(&lock);
	// from t_index the thread can fill the array into their determined place
	max_arr_thread[((struct arg*)arg_ptr_max)->t_index] = max;
	pthread_mutex_unlock(&lock);

	fclose(fp);
	
	pthread_exit(NULL);
}

void *average(void *arg_ptr_average)
{

	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *fp;

	
	
	fp = fopen((((struct arg*)arg_ptr_average)->arg_u)[((struct arg*)arg_ptr_average)->number_of_file] , "r");
	
	double averageVal = 0;
	int totalNumber = 0;
	while((read = getline(&line, &len, fp)) != -1)
	{
		// checks min and max
		if(atoi(line) >= ((struct arg*)arg_ptr_average)->n_arg && atoi(line) <= ((struct arg*)arg_ptr_average)->m_arg)
		{
		averageVal += (atoi(line) - averageVal) / (totalNumber + 1);
		totalNumber++;
		}
	}
	
	fclose(fp);
	pthread_mutex_lock(&lock);
	// evenplaces for avarage val and odd places are for number count
	avg_for_threads[(((struct arg*)arg_ptr_average)->t_index) * 2] = averageVal;
	avg_for_threads[(((struct arg*)arg_ptr_average)->t_index) * 2 + 1] = totalNumber;
	pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}

void *rangeFun(void *arg_ptr_range)
{
	for(int i = 0; i < 10; i++)
	{
		range_total_counts[i] = 0;
	}
	
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *fp;
	

	fp = fopen((((struct arg*)arg_ptr_range)->arg_u)[((struct arg*)arg_ptr_range)->number_of_file] , "r");
	
	pthread_mutex_lock(&lock);
	// algorithm is the same in the process 
	while((read = getline(&line, &len, fp)) != -1)
	{
		if(atoi(line) >= ((struct arg*)arg_ptr_range)->n_arg && atoi(line) <= ((struct arg*)arg_ptr_range)->m_arg)
		{
			if( range_total_counts[((struct arg*)arg_ptr_range)->t_index] < ((struct arg*)arg_ptr_range)->range_thread)
			{
				range_arr_thread[((struct arg*)arg_ptr_range)->t_index][range_total_counts[((struct arg*)arg_ptr_range)->t_index]] = atoi(line);
				range_total_counts[((struct arg*)arg_ptr_range)->t_index]++;
			}else
			{
				for(int i = 0; i < ((struct arg*)arg_ptr_range)->range_thread; i++)
				{
					if(range_arr_thread[((struct arg*)arg_ptr_range)->t_index][i] < atoi(line))
					{
						range_arr_thread[((struct arg*)arg_ptr_range)->t_index][i] = atoi(line);
						break;
					}
				}
			}
		}
	}
	
	pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
	
}

void swap(int* a, int* b) 
{ 
    int t = *a; 
    *a = *b; 
    *b = t; 
} 

//taken from cs202 homework just quicksort
int partition (int arr[], int low, int high) 
{ 
    int pivot = arr[high]; // pivot 
    int i = (low - 1); // Index of smaller element and indicates the right position of pivot found so far
  
    for (int j = low; j <= high - 1; j++) 
    { 
        // If current element is smaller than the pivot 
        if (arr[j] < pivot) 
        { 
            i++; // increment index of smaller element 
            swap(&arr[i], &arr[j]); 
        } 
    } 
    swap(&arr[i + 1], &arr[high]); 
    return (i + 1); 
} 
  

void quickSort(int arr[], int low, int high) 
{ 
    if (low < high) 
    { 
        /* pi is partitioning index, arr[p] is now 
        at right place */
        int pi = partition(arr, low, high); 
  
        // Separately sort elements before 
        // partition and after partition 
        quickSort(arr, low, pi - 1); 
        quickSort(arr, pi + 1, high); 
    } 
} 

int main(int argc, char **argv)
{

 	pthread_mutex_init(&lock, NULL); 
	
	
	
	

	int numOfFiles = atoi(argv[1]);

	

	mqd_t mq;
	mqd_t mqS;
	struct mq_attr mq_attr; //attribute of the message queue that will be created
	 mq_attr.mq_flags = 0;
   	 mq_attr.mq_maxmsg = 10;
   	 mq_attr.mq_msgsize = 256;
   	 mq_attr.mq_curmsgs = 0;
	int n;

	
	
	// threads
	pthread_t *tids;
	
	// structs for the thread arguments
	
	
	mq = mq_open(MQNAME, O_RDONLY | O_CREAT, 0660, &mq_attr);
	if (mq == -1) {
		perror("can not create msg queue\n");
		exit(1);
	}
	printf("mq created, mq id = %d\n", (int) mq);


 	char bufptr[266];
	
	mqS = mq_open(MQNAMES, O_WRONLY);

	
	if (mq == -1) {
		perror("can not open msg queue\n");
		exit(1);
	}
	
	while(1)
	{
	
	while (1) {
		n = mq_receive(mq, bufptr, 266, NULL);
		if (n == -1) {
			perror("mq_receive failed\n");
			exit(1);
		}

		printf("mq_receive success, message size = %d\n", n);
		printf("The message was %s", bufptr);
		break;
	}
	
	// for timing structs
	//struct timeval theStartingTime, finishingTime;
	//starting the starting time
	//gettimeofday(&theStartingTime, NULL);
	
	// obtain the parts of the bufptr
	char *token = strtok(bufptr, " ");
	int tokenCount = 0;
	char commandFirst[10];
	int arrForRanges[3];
	while(token != NULL)
	{
		if(tokenCount == 0)
		{
			strcpy(commandFirst, token);
		}else
		{
			arrForRanges[tokenCount - 1] = atoi(token);
		}
		
		tokenCount++;
		token = strtok(NULL, " ");
	}
	
	// kill the process and message queue
	if(strncmp(commandFirst, "quit", 4) == 0)
	{
			mq_unlink(MQNAME);
			close(mqS);
			exit(0);
	}
	
	struct arg* t_args = malloc(numOfFiles * sizeof(struct arg));
	tids = malloc(numOfFiles * sizeof(pthread_t));
	for(int i = 0; i < numOfFiles; i++)
	{

			
			if(strncmp(commandFirst, "count", 5) == 0 && tokenCount == 1)
			{
				t_args[i].number_of_file = i + 2;
				t_args[i].arg_u = argv;
				t_args[i].n_arg = 0;
				t_args[i].m_arg = 4194304;
				t_args[i].range_thread = 0; //not deal with this in count
				t_args[i].t_index = i;
				int ret = pthread_create(&(tids[i]),
				     NULL, count, (void *) &(t_args[i]));
				if (ret != 0) {
					printf("thread create failed \n");
				exit(1);
				}
				
			}else if(strncmp(commandFirst, "count", 5) == 0 && tokenCount == 3)
			{
				t_args[i].number_of_file = i + 2;
				t_args[i].arg_u = argv;
				t_args[i].n_arg = arrForRanges[0]; //just ranges changed from before
				t_args[i].m_arg = arrForRanges[1];
				t_args[i].range_thread = 0; //not deal with this in count
				t_args[i].t_index = i;
				int ret = pthread_create(&(tids[i]),
				     NULL, count, (void *) &(t_args[i]));
				if (ret != 0) {
					printf("thread create failed \n");
				exit(1);
				}

			}else if(strncmp(commandFirst, "max", 3) == 0)
			{
				t_args[i].number_of_file = i + 2;
				t_args[i].arg_u = argv;
				t_args[i].n_arg = 0; // is not important
				t_args[i].m_arg = 0; // is not important
				t_args[i].range_thread = 0; //not deal with this in count
				t_args[i].t_index = i;
				int ret = pthread_create(&(tids[i]),
				     NULL, maxFun, (void *) &(t_args[i]));
				if (ret != 0) {
					printf("thread create failed \n");
				exit(1);
				}
				

			}else if(strncmp(commandFirst, "avg", 3) == 0 && tokenCount == 1)
			{
				t_args[i].number_of_file = i + 2;
				t_args[i].arg_u = argv;
				t_args[i].n_arg = 0; // is not important just for taking all elements
				t_args[i].m_arg = 4194304; 
				t_args[i].range_thread = 0; //not deal with this in count
				t_args[i].t_index = i;
				int ret = pthread_create(&(tids[i]),
				     NULL, average, (void *) &(t_args[i]));
				if (ret != 0) {
					printf("thread create failed \n");
				exit(1);
				}
			}else if(strncmp(commandFirst, "avg", 3) == 0 && tokenCount == 3)
			{
				t_args[i].number_of_file = i + 2;
				t_args[i].arg_u = argv;
				t_args[i].n_arg = arrForRanges[0]; // is not important just for taking all elements
				t_args[i].m_arg = arrForRanges[1]; 
				t_args[i].range_thread = 0; //not deal with this in count
				t_args[i].t_index = i;
				int ret = pthread_create(&(tids[i]),
				     NULL, average, (void *) &(t_args[i]));
				if (ret != 0) {
					printf("thread create failed \n");
				exit(1);
				}
			}
			else if(strncmp(commandFirst, "range", 5) == 0 && tokenCount == 4)
			{
				t_args[i].number_of_file = i + 2;
				t_args[i].arg_u = argv;
				t_args[i].n_arg = arrForRanges[0]; // is not important just for taking all elements
				t_args[i].m_arg = arrForRanges[1]; 
				t_args[i].range_thread = arrForRanges[2]; //not deal with this in count
				t_args[i].t_index = i;
				int ret = pthread_create(&(tids[i]),
				     NULL, rangeFun, (void *) &(t_args[i]));
				if (ret != 0) {
					printf("thread create failed \n");
				exit(1);
				}

			}
			}
			// parent process continues from here


			// from moodle code wait for threads to die
			for (int i = 0; i < numOfFiles; i++) {
				int ret = pthread_join(tids[i], NULL);
			if (ret != 0) {
				printf("thread join failed \n");
				exit(0);
			}
			}
			
			free(t_args);
			free(tids);
			if( strncmp(commandFirst, "max", 3) == 0 || strncmp(commandFirst, "count", 5) == 0 )
			{
			int maxValue = 0;
					
	    		char sendBuffer[64];
	    		int countAll = 0;
	    		// from global value decides the max or count with respect to command
	    		if(strncmp(commandFirst, "max", 3) == 0)
	    		{
	    			for(int i = 0; i < numOfFiles; i++)
	    			{
	    				if(max_arr_thread[i] > maxValue)
	    				{
	    					maxValue = max_arr_thread[i];
	    				}
	    			}
	    		sprintf(sendBuffer, "%d", maxValue);
	    		}else{
	    		for(int k = 0; k < numOfFiles; k++)
	    		{
	    			countAll += globCount[k];

	    		}
	    		sprintf(sendBuffer, "%d", countAll);	
	    		}
	    		
	    			    		// finishing time
	    			    		/*
	    		gettimeofday(&finishingTime, NULL);
   
   printf("The time interval is %lf in microseconds", (double)(finishingTime.tv_sec * 1000000) + (double)finishingTime.tv_usec - (double)(theStartingTime.tv_sec * 1000000) - (double)theStartingTime.tv_usec);
   */
	    		
	    		int nSend = mq_send(mqS, sendBuffer, strlen(sendBuffer) + 1, 0);	
	    		
	    				if (nSend == -1) {
			perror("mq_send failed\n");
			exit(1);
			}
			int finishSend = -1;
			sprintf(sendBuffer, "%d", finishSend);
			nSend = mq_send(mqS, sendBuffer, strlen(sendBuffer) + 1, 0);	
	    		
	    		if (nSend == -1) {
			perror("mq_send failed\n");
			exit(1);
		}


	    		}//end if count
	    		
	    		else if(strncmp(commandFirst, "avg", 3) == 0)
	    		{
	    		
	    		
	    		double firstAvg = avg_for_threads[0];
	    		double firstAvgCount = avg_for_threads[1];
	    		//same algorithm for the process
	    		for(int i = 0; i < numOfFiles - 1; i++)
	    		{
	    			firstAvg += (avg_for_threads[2 * i + 2] - firstAvg) * ( avg_for_threads[2 * i + 3] / (firstAvgCount + avg_for_threads[2 * i + 3]));
	    			firstAvgCount += avg_for_threads[2 * i + 3];
	    		}
	    		

	    		
	    		char sendBuffer[64];
			
	    		sprintf(sendBuffer, "%f", firstAvg);
				    		// finishing time
				    		/*
	    		gettimeofday(&finishingTime, NULL);
   
   printf("The time interval is %lf in microseconds", (double)(finishingTime.tv_sec * 1000000) + (double)finishingTime.tv_usec - (double)(theStartingTime.tv_sec * 1000000) - (double)theStartingTime.tv_usec);
   */
	    		int nSend = mq_send(mqS, sendBuffer, strlen(sendBuffer) + 1, 0); // sends the avarage to client
	    		
	    		
	    		if (nSend == -1) {
				perror("mq_send failed\n");
				exit(1);
			}
			double finishSend = -1;
			sprintf(sendBuffer, "%f", finishSend);
			nSend = mq_send(mqS, sendBuffer, strlen(sendBuffer) + 1, 0);	
	    		
	    		if (nSend == -1) {
				perror("mq_send failed\n");
				exit(1);
	    		
	    		}
	    		}else if(strncmp(commandFirst, "range", 5) == 0)
	    		{
	    		int allArrRange[10000]; // array for all elements from all array
	    		int allArrRangeCount = 0;// count of all of the threads count
	    		

	    		for(int i = 0; i < 10; i++)
	    		{
	    			for(int j = 0; j < range_total_counts[i]; j++)
	    			{
	    				allArrRange[allArrRangeCount] = range_arr_thread[i][j];
	    				allArrRangeCount++;
	    			}
	    		}
	    		
	    		quickSort(allArrRange, 0, allArrRangeCount - 1);
	    		

			
			char sendBuffer[64];
			
			int nSend;
			
				    		// finishing time
				    		/*
	    		gettimeofday(&finishingTime, NULL);
   
   printf("The time interval is %lf in microseconds", (double)(finishingTime.tv_sec * 1000000) + (double)finishingTime.tv_usec - (double)(theStartingTime.tv_sec * 1000000) - (double)theStartingTime.tv_usec);
   */
			

			int decreaseRate = (allArrRangeCount < arrForRanges[2]) ? allArrRangeCount : arrForRanges[2];
			// send them in the loop
			for(int iR = allArrRangeCount - decreaseRate; iR < allArrRangeCount; iR++)
			{
	    		sprintf(sendBuffer, "%d", allArrRange[iR]);

	    		nSend = mq_send(mqS, sendBuffer, strlen(sendBuffer) + 1, 0);	
	    		
	    		if (nSend == -1) {
				perror("mq_send failed\n");
				exit(1);
			}
			}
			int finishSend = -1;
			sprintf(sendBuffer, "%d", finishSend);
			nSend = mq_send(mqS, sendBuffer, strlen(sendBuffer) + 1, 0);	
	    		
	    				if (nSend == -1) {
			perror("mq_send failed\n");
			exit(1);
	    		
	    		}
			
	    		
		}
		
	
	
	
	}
	return 0;
}

	




