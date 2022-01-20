#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

#define MQNAME "/test1"
#define MQNAMES "/test2"

// count method which will calculate the number of the 
// integers in the specified file
int count(int numOfFile, char **argu, int min, int max)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *fp;

	fp = fopen(argu[numOfFile], "r"); //opens the specified file

	int counts = 0;


	// i take this line from internet 
	while((read = getline(&line, &len, fp)) != -1)
	{
		if(atoi(line) >= min && atoi(line) <= max)
		{
			counts++;
		}
	}
	
	fclose(fp);
	
	return counts;
}

// maximum number is calculated per file per process
int maxFun(int numOfFile, char **argu )
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *fp;

	fp = fopen(argu[numOfFile], "r");
	
	int max = 0;
	
	// simple finding max algorithm just one value
	// per line is sufficient
	while((read = getline(&line, &len, fp)) != -1)
	{
		if(atoi(line) > max)
		{
			max = atoi(line);
		}
	}
	
	fclose(fp);
	
	return max;
}

// average is calculated i had obtained two value from every file
// one is the avarage and other is the avaragecount and turned them into array
// so that i can pipe them when they were returned
void average(int numOfFile, char **argu, int min, int max, double *rtr)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *fp;

	
	fp = fopen(argu[numOfFile], "r");
	
	double averageVal = 0; // it is normal since if the total number is 0
				// then it will have no effect on overall
	int totalNumber = 0;
	while((read = getline(&line, &len, fp)) != -1)
	{
		// simple algorithm to take avg since i did not want to make
		// overflow i did not sum them 
		if(atoi(line) >= min && atoi(line) <= max)
		{
			averageVal += (atoi(line) - averageVal) / (totalNumber + 1);
			totalNumber++;
		}
	}

	rtr[0] = averageVal;
	rtr[1] = totalNumber;
	
	fclose(fp);
}

// range function, it take rangeOf number in every file since every file just can include the largest
// values in that range
void rangeFun(int numOfFile, char **argu, int min, int max,int rangeOf, int *arrRangeFun, int *numberInArray)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *fp;

	fp = fopen(argu[numOfFile], "r");
	
	while((read = getline(&line, &len, fp)) != -1)
	{
		// i take every integer if the number of elements
		// in array is lesser than the desired range elements
		if(atoi(line) >= min && atoi(line) <= max)
		{
			if((*numberInArray) < rangeOf)
			{
				arrRangeFun[(*numberInArray)] = atoi(line);
				(*numberInArray)++;
			}else// i traversed in array and check if the number 
			{    // is larger than any of in the array
				for(int i = 0; i < rangeOf; i++)
				{
					if(arrRangeFun[i] < atoi(line))
					{
						arrRangeFun[i] = atoi(line);
						break;
					}
				}
			}
		}
	}
}

// simple swap function
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


	
	
	
	pid_t  id = 0; // id of the processes
	int numOfFiles = atoi(argv[1]); // to take the number of files 

	
	int fd[2 * numOfFiles];
	int totalCount = 0;
	int allGenerated = 0;
	
	
	
	mqd_t mq; // queue will be created for client to send
	mqd_t mqS; // queue will be used to reach the queue of the client
	
	struct mq_attr mq_attr; // some attribute values
	mq_attr.mq_flags = 0;
    	mq_attr.mq_maxmsg = 10;
    	mq_attr.mq_msgsize = 256;
    	mq_attr.mq_curmsgs = 0;
	int n;

	

	mq = mq_open(MQNAME, O_RDONLY | O_CREAT, 0660, &mq_attr);
	if (mq == -1) {
		perror("can not create msg queue\n");
		exit(1);
	}
	printf("mq created, mq id = %d\n", (int) mq);


 	char bufptr[266]; // to receive the elements from the queue
	
	mqS = mq_open(MQNAMES, O_WRONLY);

	
	if (mq == -1) {
		perror("can not open msg queue\n");
		exit(1);
	}
	
	while(1)
	{
	allGenerated = 0;
	while (1) {
		n = mq_receive(mq, bufptr, 266, NULL); // bufptr will take all of the message
		if (n == -1) {
			perror("mq_receive failed\n");
			exit(1);
		}

		printf("mq_receive success, message size = %d\n", n);
		break; // if the message is received them break to deal with command
	}
	
	// for timing structs
	//struct timeval theStartingTime, finishingTime;
	//starting the starting time
	//gettimeofday(&theStartingTime, NULL);
	// obtain the parts of the bufptr
	char *token = strtok(bufptr, " ");
	int tokenCount = 0;
	char commandFirst[10]; // for the command like max or avg
	int arrForRanges[3]; // the min max or range
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
	
	printf("command is %s\n", commandFirst);
	
	// to kill the process and close others queue and unlink itself
	if(strncmp(commandFirst, "quit", 4) == 0)
	{
			mq_unlink(MQNAME);
			close(mqS);
			exit(0);
	}
	
	for(int i = 0; i < numOfFiles; i++)
	{
		pipe(&fd[2 * i]); // pipe for every process per file
		id = fork();
		if(id == 0)
		{
			close(fd[2 * i]); // close the read end
			if(strncmp(commandFirst, "count", 5) == 0 && tokenCount == 1)
			{
				int counts = count(i + 2, argv, 0, 4194304); // 4194304 this value is taken since you mentioned 2^20 will be max value
				
				write(fd[2 * i + 1], &counts, sizeof(counts)); // count will be sent into parent
				int halt = -1;
				write(fd[2 * i + 1], &halt, sizeof(halt)); // -1 for parent to recognize
			
			}else if(strncmp(commandFirst, "count", 5) == 0 && tokenCount == 3)
			{
				int counts = count(i + 2, argv, arrForRanges[0], arrForRanges[1]); // in this case the values from user will sent into func
				
				write(fd[2 * i + 1], &counts, sizeof(counts));
				int halt = -1;
				write(fd[2 * i + 1], &halt, sizeof(halt));
				
			}else if(strncmp(commandFirst, "max", 3) == 0)
			{
				int max = maxFun(i + 2, argv); // no need to send other than argv and number of file in argv

				write(fd[2 * i + 1], &max, sizeof(max));
				int halt = -1;
				write(fd[2 * i + 1], &halt, sizeof(halt));

			}else if(strncmp(commandFirst, "avg", 3) == 0 && tokenCount == 1)
			{
				double rtArr[2];
				average(i + 2, argv, 0, 4194304, rtArr); //rtArr will obtain the values from func
				double sentAvg, sentCount;	          //by pass by reference
				sentAvg = rtArr[0];
				sentCount = rtArr[1];
				// in this case two message will be sent to parent through pipe
				// first is avarage second is count of integers in that file
				write(fd[2 * i + 1], &sentAvg, sizeof(sentAvg));
				write(fd[2 * i + 1], &sentCount, sizeof(sentCount));
				double halt = -1;
				write(fd[2 * i + 1], &halt, sizeof(halt));
				
			}else if(strncmp(commandFirst, "avg", 3) == 0 && tokenCount == 3)
			{
				double rtArr[2];
				average(i + 2, argv, arrForRanges[0], arrForRanges[1], rtArr); // difference is just min and
				double sentAvg, sentCount;				         // max obtained from user
				sentAvg = rtArr[0];
				sentCount = rtArr[1];
				
				write(fd[2 * i + 1], &sentAvg, sizeof(sentAvg));
				write(fd[2 * i + 1], &sentCount, sizeof(sentCount));
				double halt = -1;
				write(fd[2 * i + 1], &halt, sizeof(halt));
				
			}
			else if(strncmp(commandFirst, "range", 5) == 0 && tokenCount == 4)
			{
				int numInRangeArray = 0; // number of integers in that file conforting min max and at most range values
				int arrRange[1000]; // in the file the values in the range is at most 1000 
						     // this array includes all the elements conforts values in that file
				rangeFun(i + 2, argv, arrForRanges[0], arrForRanges[1], arrForRanges[2], arrRange, &numInRangeArray);
				
				
				// with the gathered values i piped them into parent
				for(int jx = 0; jx < numInRangeArray; jx++)
				{
					int newElementRange = arrRange[jx];
					write(fd[2 * i + 1], &newElementRange, sizeof(newElementRange));
				}
				
				int halt = -1;
				write(fd[2 * i + 1], &halt, sizeof(halt));
				
			}
			
			close(fd[2 * i + 1]); // close the write end
			exit(0); // when the chill dealed with its mission kill the child
		}else if(id > 0)
		{
			
			allGenerated++; 
			if(allGenerated == numOfFiles) // when all the children produced, parents stars the job
			{
			
			int recv_byte = 0; // will be used to receive from pipe
			int getStop = 0; // count for every children send -1
			int arrStopped[numOfFiles]; // to check whether children is sended -1

			for(int k = 0; k < numOfFiles; k++)
			{
				arrStopped[k] = 0;
			}
			
			
			for(int l = 0; l < numOfFiles; l++)
			{
				close(fd[2 * l + 1]); // close the write end
			}
					
			if( strncmp(commandFirst, "max", 3) == 0 || strncmp(commandFirst, "count", 5) == 0 )
			{
			totalCount = 0; // for count output
			int maxValue = 0; // for max
			
			// checks for the inputs from pipe and when all the -1 is gathered
			// exits from loop
			while(getStop < numOfFiles)
			{
				for(int j = 0; j < numOfFiles; j++)
				{
					if(read(fd[2 * j], &recv_byte, sizeof(recv_byte)) > 0)
					{
		        		if(recv_byte == -1 && arrStopped[j] == 0)
		        		{
		        			arrStopped[j] = -1;
	    					getStop++; // if received value is -1 increment
	    				}
	    				else if(arrStopped[j] == 0)
	    				{
	    				if(strncmp(commandFirst, "max", 3) == 0)
	    				{
	    				if(maxValue < recv_byte)
	    				{
	    					maxValue = recv_byte; // simple max from the values that
	    				}			      //  every children provided
	    				}else{
	    					totalCount += recv_byte; // sums the counts from the children
	    					}
	    				}
	    				}
	    		
	    			}
	    		}
	    		
	    		
	    		for(int l = 0; l < numOfFiles; l++)
			{
				close(fd[2 * l]); // closes the read ends
			}
	    		    		
	    		char sendBuffer[64]; // to send message into client
	    		if(strncmp(commandFirst, "max", 3) == 0)
	    		{
	    			sprintf(sendBuffer, "%d", maxValue);
	    		}else{
	    			sprintf(sendBuffer, "%d", totalCount);	
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
			
			int finishSend = -1; // to inform the client
			sprintf(sendBuffer, "%d", finishSend);
			nSend = mq_send(mqS, sendBuffer, strlen(sendBuffer) + 1, 0);	
	    		
	    		if (nSend == -1) {
				perror("mq_send failed\n");
				exit(1);
			}
	    		}//end if count
	    		else if(strncmp(commandFirst, "avg", 3) == 0)
	    		{
	    		double arrForAvg[20]; // odd numbers  index include the avarages 
	    					// and even numbers of index includes
	    					// count of numbers in that file
	    					
	    		for(int j = 0; j < 20; j++)
	    		{
	    			arrForAvg[j] = 0;
	    		}
	    		
	    		double recDoub; // used to receive double from pipe
	    		int firstOrSecond[10]; // used to check if the both average
	    					// and count is taken or not
	    		
	    		for(int il = 0; il < 10; il++)
	    		{
	    			firstOrSecond[il] = 0;
	    		}
	    		
	    		while(getStop < numOfFiles)
			{
			for(int j = 0; j < numOfFiles; j++)
			{
			if(read(fd[2 * j], &recDoub, sizeof(recDoub)) > 0)
			{
		        if(recDoub == -1 && arrStopped[j] == 0)
		        {
		        	arrStopped[j] = -1;
	    			getStop++;
	    			
	    			}
	    		else if(arrStopped[j] == 0)
	    		{
	    			if(firstOrSecond[j] == 0)
	    			{
	    				arrForAvg[j * 2] = recDoub; // take the average into evenplace
	    				firstOrSecond[j] = -1;
	    			}else
	    			{
	    				arrForAvg[j * 2 + 1] = recDoub; // take the count in odd index
	    			}
	    		}
	    		}
	    		}
	    		}
	    		
	    		
	    		
	    		double firstAvg = arrForAvg[0];
	    		double firstAvgCount = arrForAvg[1];
	    		
	    		// i even did not want to make overflow then i used the methods in physics like center of gravity
	    		for(int i = 0; i < numOfFiles - 1; i++)
	    		{
	    			firstAvg += (arrForAvg[2 * i + 2] - firstAvg) * ( arrForAvg[2 * i + 3] / (firstAvgCount + arrForAvg[2 * i + 3]));
	    			firstAvgCount += arrForAvg[2 * i + 3];
	    		}
	    		
	    		
	    		for(int l = 0; l < numOfFiles; l++)
			{
				close(fd[2 * l]); 
			}
	    		
	    		char sendBuffer[64];

	    		sprintf(sendBuffer, "%f", firstAvg); // turns the double number
	    		
	    		//finishing
	    		/*
	    		gettimeofday(&finishingTime, NULL);
   
   printf("The time interval is %lf in microseconds", (double)(finishingTime.tv_sec * 1000000) + (double)finishingTime.tv_usec - (double)(theStartingTime.tv_sec * 1000000) - (double)theStartingTime.tv_usec);
   */

	    		int nSend = mq_send(mqS, sendBuffer, strlen(sendBuffer) + 1, 0);	
	    		
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
	    			int allArrRange[10000]; // all numbers will be filled into this arr
	    			int allArrRangeCount = 0; // total numbers from every children
	    			int takeValueRange; // to take value from pipe
	    		while(getStop < numOfFiles)
			{
			for(int j = 0; j < numOfFiles; j++)
			{
			if(read(fd[2 * j], &takeValueRange, sizeof(takeValueRange)) > 0)
			{
		        if(takeValueRange == -1 && arrStopped[j] == 0)
		        {
		        	arrStopped[j] = -1;
	    			getStop++;
	    			}
	    		else if(arrStopped[j] == 0)
	    		{
				allArrRange[allArrRangeCount] = takeValueRange;
				allArrRangeCount++;
	    		}
	    		}
	    		
	    		
	    		}
	    		}
	    		
	    		quickSort(allArrRange, 0, allArrRangeCount - 1); // send the array to quicksort 
	    		
	    		for(int l = 0; l < numOfFiles; l++)
			{
				close(fd[2 * l]);
			}
			
			char sendBuffer[64];
			
			int nSend;
			
			
			//finishing 
			//gettimeofday(&finishingTime, NULL);
   /*
   printf("The time interval is %lf in microseconds", (double)(finishingTime.tv_sec * 1000000) + (double)finishingTime.tv_usec - (double)(theStartingTime.tv_sec * 1000000) - (double)theStartingTime.tv_usec);
*/
			// decrease rate provides if the elements in the array is lesser than the desired number count
			int decreaseRate = (allArrRangeCount < arrForRanges[2]) ? allArrRangeCount : arrForRanges[2];
			// send every number in loop
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
	
	}
	}

}

	return 0;
}



