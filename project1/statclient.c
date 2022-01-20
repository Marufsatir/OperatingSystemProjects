
      
#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#define MQNAME "/test1"
#define MQNAMES "/test2"

int main()
{
	char *buffer = NULL;
	size_t len;
	mqd_t mq;

	int n, m;
 
    
	mq = mq_open(MQNAME, O_WRONLY); //connecting the servers message queue

	
	if (mq == -1) {
		perror("can not open msg queue\n");
		exit(1);
	}
	printf("mq opened, mq id = %d\n", (int) mq);
	
	mqd_t mqS;
	struct mq_attr mq_attr;
	mq_attr.mq_flags = 0;
    	mq_attr.mq_maxmsg = 10;
    	mq_attr.mq_msgsize = 256;
    	mq_attr.mq_curmsgs = 0;
	
	mqS = mq_open(MQNAMES, O_RDONLY | O_CREAT, 0660, &mq_attr); // creating its message queue
	if (mq == -1) {
		perror("can not create msg queue\n");
		exit(1);
	}
	
	char bufptr[266];
	
	int checkForAvg;
	
	while (1) {
		checkForAvg = 0;
		printf("Please enter the operation and the values\n");
		getline(&buffer, &len, stdin);

		char outBuffer[64];
		sprintf(outBuffer, buffer);
		
		if(strncmp(buffer, "avg", 3) == 0)
		{
			checkForAvg = 1;
		}
		n = mq_send(mq, outBuffer, strlen(outBuffer) + 1, 0); // sends the buffer into server messsage queue

		if (n == -1) {
			perror("mq_send failed\n");
			exit(1);
		}
		
		if(strncmp(buffer, "quit", 4) == 0) // if the message is quit after sending quit message to server quit
		{
			mq_unlink(MQNAMES);
			close(mq);
			exit(0);
		}
		
		while(1)
		{
		m = mq_receive(mqS, bufptr, 266, NULL);
		if (m == -1) {
			perror("mq_receive failed\n");
			exit(1);
		}
			if(checkForAvg) // i made it since avg command will lead double data
			{
				if(atof(bufptr) == -1)
				{
					break;
				}else
				{
					printf("The average is %f\n", atof(bufptr));
				}
			}
			else if(atoi(bufptr) == -1)
			{
				break;
			}else
			{
				printf("%d\n", atoi(bufptr)); // print every value that comes in except -1
				// i used same logic with pipe when the last message is -1 stop listening to
				// server
			}
		}
		
	}

	mq_close(mq);
	return 0;
}

