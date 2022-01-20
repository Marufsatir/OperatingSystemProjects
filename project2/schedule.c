/**
 * @file schedule.c
 * @author Muharrem Berk Yıldız, Muhammed Maruf Satir
 * @version 0.1
 * @date 2021-11-17
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#define MAXSIZE 1000
#define MAXBURSTLENGTH 401

struct burst
{
    int burstNumber;
    int burstLength;
    int burstArrival;
    int burstRemaining;
};

struct burst arr[MAXSIZE];
struct burst originalBurst[MAXSIZE];

int front = -1, end = -1;
int realTime = 0;
int burstsOnQueue = 0;
int quantumTimer;

void enqueue(struct burst br)

{
    if (burstsOnQueue == 0)
    {
        front = 0;
        end = 0;
        arr[end] = br;
        //printf("enqueue after empty\n");
        burstsOnQueue++;
        return;
    }

    if (front != (end + 1) % MAXSIZE)
    {
        end = (end + 1) % MAXSIZE;
        arr[end] = br;
        //printf("enquee other %d\n", br.burstNumber);
        burstsOnQueue++;
    }
}

struct burst dequeue()
{
    struct burst enValue;
    //Empty burst.
    enValue.burstNumber = -1;
    
    if (burstsOnQueue == 0)
    {
       // printf("can not delete\n");
    }
    else
    {
        if (burstsOnQueue != 0)
        {
            enValue = arr[front];
            front = (front + 1) % MAXSIZE;
            //printf("the deque %d, burstId: %d, finish time: %d\n", realTime - enValue.burstArrival, enValue.burstNumber, realTime);
            burstsOnQueue--;
            return enValue;
        }
        
    }
    return enValue;
}

void increaseTime()
{
    realTime++;
    arr[front].burstRemaining--;
}

void increaseTimeQ()
{
    realTime++;
    arr[front].burstRemaining--;
    if (burstsOnQueue > 0){
        quantumTimer++;
    }
}

int main(int argc, char **argv)
{

    if (argc < 3)
    {
        printf("Please enter <input file> and <quantum>.\n");
        return 1;
    }

    FILE *fp;

    char *line = NULL;
    size_t len = 0;
    fp = fopen(argv[1], "r");
    ssize_t read;
    int fileSize = 0;
    double avgTimeFCFS, avgTimeSJF, avgTimeSRTF, avgTimeRR = 0;

    if (argc < 3 || atoi(argv[2]) <= 9 || atoi(argv[2]) >= 301){
        printf("Time quantum is between 10 and 300.\n");
        return 1;
    }
    
    int quantum = atoi(argv[2]);

    int burstNumber, arrivalTime, burstLength;
    struct burst newBurst;

    //Read bursts from file.
    while ((read = getline(&line, &len, fp)) != -1)
    {
        burstNumber = (int)strtol(line, &line, 0);
        while (isspace(*line))
            ++line;
        arrivalTime = (int)strtol(line, &line, 0);
        while (isspace(*line))
            ++line;
        burstLength = (int)strtol(line, &line, 0);

        newBurst.burstNumber = burstNumber;
        newBurst.burstLength = burstLength;
        newBurst.burstArrival = arrivalTime;
        newBurst.burstRemaining = burstLength;

        originalBurst[fileSize] = newBurst;

        fileSize++;
    }
    //Reset the queue.
    burstsOnQueue = 0;
    int indexBurst = 0;
    int avgIndex = 0;
    int turnaroundTime;

    //FCFS
    while (1)
    {

        if (burstsOnQueue == 0 && (indexBurst >= fileSize))
        {
            break;
        }
        //printf("\n INDEXBURST => %d | realTime=> %d | fileSize => %d | remaining of top => %d | top id %d\n", indexBurst,realTime, fileSize, arr[front].burstRemaining, arr[front].burstNumber);

        //If next burst is ready to be added to the queue.
        if (indexBurst < fileSize && originalBurst[indexBurst].burstArrival == realTime)
        {
            enqueue(originalBurst[indexBurst]);
            indexBurst++;
            continue;
        }

        //If the process finishes.
        if  (burstsOnQueue > 0 && arr[front].burstRemaining == 0)
        {
            turnaroundTime = realTime - dequeue().burstArrival;
            //printf("\nTurnaround time => %d\n", turnaroundTime);
            avgTimeFCFS += (turnaroundTime - avgTimeFCFS) / (avgIndex + 1);
            avgIndex++;
        }
        increaseTime();
    }
    printf("FCFS\t%0.0f\n", round(avgTimeFCFS));

    //Reset the queue.
    burstsOnQueue = 0;
    indexBurst = 0;
    avgIndex = 0;
    turnaroundTime = 0;
    realTime = 0;

    //SJF
    while (1)
    {

        if (burstsOnQueue == 0 && (indexBurst >= fileSize))
        {
            break;
        }
        //printf("\n INDEXBURST => %d | realTime=> %d | fileSize => %d | remaining of top => %d | top id %d\n", indexBurst,realTime, fileSize, arr[front].burstRemaining, arr[front].burstNumber);

        //If next burst is ready to be added to the queue.
        if (indexBurst < fileSize && originalBurst[indexBurst].burstArrival == realTime)
        {
            enqueue(originalBurst[indexBurst]);
            indexBurst++;
            continue;
        }

        //If the process finishes.
        if (burstsOnQueue > 0 && arr[front].burstRemaining == 0)
        {
            turnaroundTime = realTime - dequeue().burstArrival;
            //printf("\nTurnaround time => %d\n", turnaroundTime);
            avgTimeSJF += (turnaroundTime - avgTimeSJF) / (avgIndex + 1);
            avgIndex++;

            //Bring fastest burst to the front of the queue to run.
            int shortestBurstIndex = 0;
            int shortestBurst = MAXBURSTLENGTH;
            int shortestBurstNumber = fileSize + 1;

            //If there are more than 1 bursts remaining.
            if (burstsOnQueue > 1)
            {

                for (int i = 0; i < burstsOnQueue; i++)
                {
                    if (arr[front + i].burstLength < shortestBurst || (arr[front + i].burstLength == shortestBurst && arr[front + i].burstNumber < shortestBurstNumber))
                    {
                        shortestBurstIndex = front + i;
                        shortestBurst = arr[front + i].burstLength;
                        shortestBurstNumber = arr[front + i].burstNumber;
                    }
                }
                //Swap bursts.
                struct burst temp = arr[shortestBurstIndex];
                //printf("SWAP (-)Between %d-%d and %d-%d",temp.burstNumber, temp.burstRemaining, arr[front].burstNumber, arr[front].burstRemaining);
                arr[shortestBurstIndex] = arr[front];
                arr[front] = temp;
            }
        }
        increaseTime();
    }

    printf("SJF\t%0.0f\n", round(avgTimeSJF));

    //Reset the queue.
    burstsOnQueue = 0;
    indexBurst = 0;
    avgIndex = 0;
    turnaroundTime = 0;
    realTime = 0;

    //SRTF
    while (1)
    {

        if (burstsOnQueue == 0 && (indexBurst >= fileSize))
        {
            break;
        }
        //printf("\n INDEXBURST => %d | realTime=> %d | fileSize => %d | remaining of top => %d | top id %d\n", indexBurst,realTime, fileSize, arr[front].burstRemaining, arr[front].burstNumber);

        //If next burst is ready to be added to the queue.
        if (indexBurst < fileSize && originalBurst[indexBurst].burstArrival == realTime)
        {
            enqueue(originalBurst[indexBurst]);
            indexBurst++;

            //Bring fastest burst to the front of the queue to run.
            int shortestBurstIndex = 0;
            int shortestBurst = MAXBURSTLENGTH;

            //If there are more than 2 bursts.
            if (burstsOnQueue > 1)
            {

                for (int i = 0; i < burstsOnQueue; i++)
                {
                    if (arr[front + i].burstRemaining < shortestBurst)
                    {
                        shortestBurstIndex = front + i;
                        shortestBurst = arr[front + i].burstRemaining;
                    }
                }
                //Swap bursts.
                struct burst temp = arr[shortestBurstIndex];
                //printf("SWAP (+)Between %d-%d and %d-%d",temp.burstNumber, temp.burstRemaining, arr[front].burstNumber, arr[front].burstRemaining);
                arr[shortestBurstIndex] = arr[front];
                arr[front] = temp;
            }

            continue;
        }

        //If the process finishes.
        if (burstsOnQueue > 0 && arr[front].burstRemaining == 0)
        {
            turnaroundTime = realTime - dequeue().burstArrival;
            //printf("\nTurnaround time => %d\n", turnaroundTime);
            avgTimeSRTF += (turnaroundTime - avgTimeSRTF) / (avgIndex + 1);
            avgIndex++;

            //Bring fastest burst to the front of the queue to run.
            int shortestBurstIndex = 0;
            int shortestBurst = MAXBURSTLENGTH;
            int shortestBurstNumber = fileSize + 1;

            //If there are more than 1 bursts remaining.
            if (burstsOnQueue > 1)
            {

                for (int i = 0; i < burstsOnQueue; i++)
                {
                    if (arr[front + i].burstRemaining < shortestBurst || (arr[front + i].burstRemaining == shortestBurst && arr[front + i].burstNumber < shortestBurstNumber))
                    {
                        shortestBurstIndex = front + i;
                        shortestBurst = arr[shortestBurstIndex].burstRemaining;
                        shortestBurstNumber = arr[front + i].burstNumber;
                    }
                }
                //Swap bursts.
                struct burst temp = arr[shortestBurstIndex];
                //printf("SWAP (-)Between %d-%d and %d-%d",temp.burstNumber, temp.burstRemaining, arr[front].burstNumber, arr[front].burstRemaining);
                arr[shortestBurstIndex] = arr[front];
                arr[front] = temp;
            }
        }
        increaseTime();
    }
    printf("SRTF\t%0.0f\n", round(avgTimeSRTF));

    //Reset the queue.
    burstsOnQueue = 0;
    indexBurst = 0;
    avgIndex = 0;
    turnaroundTime = 0;
    realTime = 0;
    quantumTimer = 0;

    //RR
    while (1)
    {

        if (burstsOnQueue == 0 && (indexBurst >= fileSize))
        {
            break;
        }
        //printf("\n %d INDEXBURST => %d | realTime=> %d  | burstArrival => %d | fileSize => %d | remaining of top => %d | top id %d\n",burstsOnQueue ,indexBurst,realTime, arr[front].burstArrival, fileSize, arr[front].burstRemaining, arr[front].burstNumber);

        //If time quantum is reached.
        if ( burstsOnQueue > 0 && quantumTimer == quantum && arr[front].burstRemaining != 0){
                //Requeue burst.
                struct burst temp = dequeue();
                enqueue(temp);
                //printf("Requeue (#)Between %d-%d and %d-%d\n",temp.burstNumber, temp.burstRemaining, arr[front].burstNumber, arr[front].burstRemaining);
                quantumTimer = 0;
        }

        //If the process finishes.
        if ( burstsOnQueue > 0 && arr[front].burstRemaining == 0)
        {
            turnaroundTime = realTime - dequeue().burstArrival;
            //printf("\nTurnaround time => %d\n", turnaroundTime);
            avgTimeRR += (turnaroundTime - avgTimeRR) / (avgIndex + 1);
            avgIndex++;
            quantumTimer = 0;
        }


        //If next burst is ready to be added to the queue.
        if (indexBurst < fileSize && originalBurst[indexBurst].burstArrival == realTime)
        {
            enqueue(originalBurst[indexBurst]);
            indexBurst++;
            continue;
        }

        increaseTimeQ();
    }
    printf("RR\t%0.0f\n", round(avgTimeRR));
}