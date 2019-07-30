// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
// #include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <inttypes.h>

#include "definitions.h"
#include "messageTools.h"
#include "circular_buffer.h"
#include "threads.h"
#include "ipScan.h"

void writeSendList(char* filePath, int **sendList, char **circBuffer);
int loadSendList(char* filePath, int **sendList);
void writeCircBuffer(char* filePath, char **circBuffer);
int loadCircBuffer(char* filePath, char **circBuffer, cbuf_handle_t cbuf);


void sigintHandler(int sig_num);
int terminateThreads=0;

int main(int argc, char const *argv[])
{
	int i, rc, socketList[MAX_SOCKETS], rc1, rc2;
	char buffer[MSG_SIZE];
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("program_started : %" PRIu64 " : %d\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000, myAEM(NETWORK_DEVICE));

	//List to keep track of where we have sent each message
	int** sendList = (int**)malloc(CIRC_BUFFER_SIZE * sizeof(int*));
	for(i=0;i<CIRC_BUFFER_SIZE; i++) sendList[i] = (int *)malloc(MAX_SOCKETS*sizeof(int));

	//Circular buffer Memory Allocation
	char** circBuffer  = (char**)malloc(CIRC_BUFFER_SIZE * sizeof(char*));
	for(i=0;i<CIRC_BUFFER_SIZE; i++) circBuffer[i] = (char *)malloc(MSG_SIZE*sizeof(char));

	cbuf_handle_t cbuf = circular_buf_init(circBuffer, CIRC_BUFFER_SIZE);

	//Load save Files
	if((rc1 = loadCircBuffer("circBuffer.txt", circBuffer, cbuf)) == -1)
		printf("circBuffer.txt not found!\nStarting with clean buffers\n");
	else if((rc2 = loadSendList("sendList.txt", sendList))==-1)
	{
		printf("sendList.txt not found!\nStarting with clean buffers\n");
		circular_buf_reset(cbuf);
	}
	else if(rc1!=rc2)
	{
		printf("Error in save files!\nStarting with clean buffers\n");
		circular_buf_reset(cbuf);
		for(i=0;i<CIRC_BUFFER_SIZE;i++) memset(sendList[i], 0, MAX_SOCKETS*sizeof(int));
		for(i=0;i<CIRC_BUFFER_SIZE;i++) circBuffer[i][0]='\0';
	}

	//Generator Thread data declaration and initialization
	generator_data genData={1, 5, 1};
	genData.cbuf = &cbuf; genData.sendList = sendList; genData.terminate = &terminateThreads;
	pthread_t genThread;

	//Client Thread data declaration and initialization
	client_data cliData;
	cliData.tid = 0;
	cliData.cbuf = &cbuf;
	cliData.sendList = sendList;
	cliData.circBuffer = circBuffer;
	cliData.socketList = socketList;
	cliData.terminate = &terminateThreads;
	pthread_t cliThread;

	//Server Thread data declaration and initialization
	server_data svrData;
	svrData.tid = 2;
	svrData.cbuf = &cbuf;
	svrData.sendList = sendList;
	svrData.circBuffer = circBuffer;
	svrData.socketList = socketList;
	svrData.terminate = &terminateThreads;
	pthread_t svrThread;


	//initialise int array with zeros
	memset(&socketList, 0, MAX_SOCKETS*sizeof(int));

	//Use time for random generation
	srand(time(0));

	//Create Thread for: Client loop
	rc = pthread_create(&cliThread, NULL, (void*)&client, (void*)&cliData);
	if (rc)
	{
  	printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }

	//Create Thread for: Message generator loop
	rc = pthread_create(&genThread, NULL, (void*)&msgGenerator, (void*)&genData);
	if (rc)
	{
  	printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }

	//Create Thread for: Server loop
	rc = pthread_create(&svrThread, NULL, (void*)&server, (void*)&svrData);
	if (rc)
	{
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	signal(SIGINT, sigintHandler);

	// sigintHandler(SIGINT);

	void* status;
	rc = pthread_join(genThread, &status);
	if(rc) printf("ERROR; return code from pthread_join() is %d\n", rc);
	else
	{
		gettimeofday(&tv, NULL);
		printf("generator_joined : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	}
	rc = pthread_join(svrThread, &status);
	if(rc) printf("ERROR; return code from pthread_join() is %d\n", rc);
	else
	{
		gettimeofday(&tv, NULL);
		printf("server_joined : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	}
	rc = pthread_join(cliThread, &status);
	if(rc) printf("ERROR; return code from pthread_join() is %d\n", rc);
	else
	{
		gettimeofday(&tv, NULL);
		printf("client_joined : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	}


	//save circBuffer and sendList to files
	writeCircBuffer("circBuffer.txt", circBuffer);
	writeSendList("sendList.txt", sendList, circBuffer);

	//free sendList
	for(int i=0;i<CIRC_BUFFER_SIZE;i++) free(sendList[i]); free(sendList);
	//free circBuffer
	for(int i=0;i<CIRC_BUFFER_SIZE;i++) free(circBuffer[i]); free(circBuffer);

	pthread_mutex_destroy(&bufferMutex);
	pthread_mutex_destroy(&socketMutex);
	gettimeofday(&tv, NULL);
	if(1) printf("program_finished : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	pthread_exit(NULL);
	return 0;
}

/* Signal Handler for SIGINT */
void sigintHandler(int sig_num)
{
    /* Reset handler to catch SIGINT next time.
       Refer http://en.cppreference.com/w/c/program/signal */
    // signal(SIGINT, sigintHandler);
    // printf("\n Cannot be terminated using Ctrl+C \n");
		// printf("\nCtrl+C signal detected.Terminating program!!!\n");
		printf("\nTerminating Threads\nWait until each thread finishes it's current loop.(it might take some time)\n");
		pthread_mutex_lock(&terminateMutex); terminateThreads=1; pthread_mutex_unlock(&terminateMutex);


		// exit(0);
    // fflush(stdout);
}

int loadCircBuffer(char* filePath, char **circBuffer, cbuf_handle_t cbuf)
{
	FILE *saveFile = fopen(filePath, "r"); if(saveFile==NULL) return -1;
	char msg[MSG_SIZE];
	int count=0;

	while(fgets(msg, MSG_SIZE, saveFile) != NULL)
	{
		//Put message in buffer
		memset(msg+strlen(msg), ' ', MSG_SIZE-strlen(msg)-1);
		msg[MSG_SIZE-1]='\0';
    circular_buf_put(cbuf, msg);
		count++;
	}
	fclose(saveFile);
	return count;
}

void writeCircBuffer(char* filePath, char **circBuffer)
{
	FILE *saveFile = fopen(filePath, "w");
	int i;
	char message[MSG_SIZE];

	for(i=0;i<CIRC_BUFFER_SIZE;i++)
	{
		if(strlen(circBuffer[i])==0) break;
		strcpy(message, circBuffer[i]);
		message[strcspn(message, "\n")] = 0;
		fprintf(saveFile, "%s\n", message);
	}
	fclose(saveFile);
}

int loadSendList(char* filePath, int **sendList)
{
	FILE *saveFile = fopen(filePath, "r"); if(saveFile==NULL) return -1;
	int lineSize=MAX_SOCKETS*5 + 1;
	char line[lineSize], *token;
	int iterX=0, iterY=0;

	while(fgets(line, lineSize, saveFile) != NULL)
	{
		iterY=0;
		line[strcspn(line, "\n")] = 0;

		token = strtok(line, "-");
		while (token != NULL)
		{
    	sendList[iterX][iterY++] = atoi(token);
      token = strtok(NULL, "-");
    }
		iterX++;
	}
	fclose(saveFile);
	return iterX;
}

void writeSendList(char* filePath, int **sendList, char **circBuffer)
{
	FILE *saveFile = fopen(filePath, "w");
	int i, j;

	for(i=0;i<CIRC_BUFFER_SIZE;i++)
	{
		if(strlen(circBuffer[i])==0) break;
		for(j=0;j<MAX_SOCKETS;j++)
		{
			if(sendList[i][j] == 0) break;
			if(j==0) fprintf(saveFile, "%d", sendList[i][j]);
			else     fprintf(saveFile, "-%d", sendList[i][j]);
		}
		fprintf(saveFile, "\n");
	}
	fclose(saveFile);
}
