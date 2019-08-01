/*
    Author: Polizois Siois 8535
*/
/*
    Faculty of Electrical and Computer Engineering AUTH
    Final assignemnt for Real Time Embedded Systems course (8th semester)
*/
/*
		Implementation of a very basic mesh network used for message exchanges.
		The program is meant to be run on every device(raspberry PI Zero) in this network.
		The program generates message with destination certain devices. It also connects
		with the available devices in the network and exchanges these messsages with them.
		A messsage circulates from device to device in the network until it reaches it's
		destination.
*/
/*
		-- Available devices --
    This iteration of the program tries to ping the broadcast ip of the network
		in order to find all the available devices and connect to them. So the devices that
		take part in this network should be able to respond to broadcast ping requests.
		In rasbian OS this can be done by writing 0 to /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts.

		-- Message destination --
		The destination device of the genetated messages gets randomly selected by producing
		a random device id in the range of 7000 to 9999. A future iteration of the progran might use
		a given list of device ids.

		-- Device ip --
		The ip of the devive should be set according to the device's id in a certain way.
		If the device's id is 8535 and the netmask is 255.0.0.0 the static ip should be 10.0.85.35.

		-- Device id --
		The device id is a 4 digit number and is the same as the student id in this case.
*/
#include <stdio.h>
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
#include "saveTools.h"

// Gets called when SIGINT signal appears and assgins value 1 to variable "terminateThreads"
void sigintHandler(int sig_num);
// Variable used by the threads to determine whether they should terminate or not
int terminateThreads=0;

int main(int argc, char const *argv[])
{
	int i, rc, socketList[MAX_SOCKETS], rc1, rc2;
	char buffer[MSG_SIZE];
	struct timeval tv;
	void* status;

	gettimeofday(&tv, NULL);
	printf("program_started : %" PRIu64 " : %d\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000, getAEM(myIP(NETWORK_DEVICE)));

	//List to keep track of where we have sent each message
	int** sendList = (int**)malloc(CIRC_BUFFER_SIZE * sizeof(int*));
	for(i=0;i<CIRC_BUFFER_SIZE; i++) sendList[i] = (int *)malloc(MAX_SOCKETS*sizeof(int));

	//Circular buffer Memory Allocation
	char** circBuffer  = (char**)malloc(CIRC_BUFFER_SIZE * sizeof(char*));
	for(i=0;i<CIRC_BUFFER_SIZE; i++) circBuffer[i] = (char *)malloc(MSG_SIZE*sizeof(char));

	cbuf_handle_t cbuf = circular_buf_init(circBuffer, CIRC_BUFFER_SIZE);

	//Load save Files
	if((rc1 = loadCircBuffer("circBuffer.txt", cbuf)) == -1)
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
	generator_data genData;
	genData.tid = 1;
	genData.minInterval = 1;
	genData.maxInterval = 5;
	genData.cbuf = &cbuf;
	genData.sendList = sendList;
	genData.terminate = &terminateThreads;
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
	rc = pthread_create(&genThread, NULL, (void*)&generator, (void*)&genData);
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

	// Handle Ctrl+C signal
	signal(SIGINT, sigintHandler);

	//Wait for threads to join
	rc = pthread_join(genThread, &status);
	if(rc) printf("ERROR; return code from pthread_join() is %d\n", rc);
	// else
	// {
	// 	gettimeofday(&tv, NULL);
	// 	printf("generator_joined : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	// }
	rc = pthread_join(svrThread, &status);
	if(rc) printf("ERROR; return code from pthread_join() is %d\n", rc);
	else
	// {
	// 	gettimeofday(&tv, NULL);
	// 	printf("server_joined : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	// }
	rc = pthread_join(cliThread, &status);
	if(rc) printf("ERROR; return code from pthread_join() is %d\n", rc);
	else
	// {
	// 	gettimeofday(&tv, NULL);
	// 	printf("client_joined : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	// }


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
	printf("program_finished : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	pthread_exit(NULL);
	return 0;
}

/* Signal Handler for SIGINT */
void sigintHandler(int sig_num)
{
		printf("\nTerminating Threads\nWait until each thread finishes it's current loop.(it might take some time)\n");
		pthread_mutex_lock(&terminateMutex); terminateThreads=1; pthread_mutex_unlock(&terminateMutex);
}
