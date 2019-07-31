#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "definitions.h"
#include "ipScan.h"
#include "messageTools.h"
#include "circular_buffer.h"
#include "randString.h"
#include "threads.h"

void generator(void* args)
{
	generator_data *myData = (generator_data*) args;
	int minInterval = myData->minInterval, maxInterval =  myData->maxInterval;
	int** sendList = myData->sendList;
	int* terminate = myData->terminate;
	int thisAEM = getAEM(myIP(NETWORK_DEVICE));
	int destAEM;
	int num=0;
	int run;
	char msg[MSG_SIZE];
	struct timeval tv;

	pthread_mutex_lock(&terminateMutex); run = !(*terminate); pthread_mutex_unlock(&terminateMutex);
	if(minInterval>0) minInterval=minInterval-1;
	while(run)
	{
		//Sleep for "num" minutes.(num: random number between minInterval and maxInterval)
		num = rand_number(minInterval, maxInterval);
		//printf("Sleeping for %d minutes\n", num);
		sleep(num*60);
		// sleep(30);

		//Random message generation
		//Dont generate messages with destination this device
		do{ destAEM=rand_number(7000,9999); }while( destAEM==thisAEM );
		generateMessage(msg, thisAEM, destAEM, 10, 200);
		gettimeofday(&tv, NULL);
		printf("generated_message : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
		strcat(msg, "\n");
		memset(msg+strlen(msg), ' ', MSG_SIZE-strlen(msg)-1);
		msg[MSG_SIZE-1]='\0';

		//Atomic buffer acces to save message
		pthread_mutex_lock (&bufferMutex);
		memset(sendList[circular_buf_head(*(myData->cbuf))], 0, MAX_SOCKETS*sizeof(int));
    circular_buf_put(*(myData->cbuf), msg);
    pthread_mutex_unlock (&bufferMutex);

		pthread_mutex_lock(&terminateMutex); run = !(*terminate); pthread_mutex_unlock(&terminateMutex);
	}

	gettimeofday(&tv, NULL);
	printf("generator_finished : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	pthread_exit((void*)myData->tid);
}

void client(void* args)
{
	int i, j, ipNum = 0, sock = 0, max_sock=0, valread;
	int thisAEM = getAEM(myIP(NETWORK_DEVICE));
	struct sockaddr_in serv_addr;
	int addrlen = sizeof(serv_addr);
	char message[MSG_SIZE];
	char** ipTable=NULL;
	char *broadIP = broadcastIP(NETWORK_DEVICE);
 	int run;
	int tempAEM;
	int tempPos;

	struct timeval tv;

	//Arguements
	client_data *myData = (client_data*) args;
	cbuf_handle_t cbuf = *(myData->cbuf);
	char **circBuffer = myData->circBuffer;
	int** sendList = myData->sendList;
	int tid = myData->tid;
	int* socketList = myData->socketList;
	int* terminate = myData->terminate;

	pthread_mutex_lock(&terminateMutex); run = !(*terminate); pthread_mutex_unlock(&terminateMutex);
	signal(SIGPIPE, SIG_IGN);
	//Main loop
	while(run)
	{
		// Scan IPs in network
		ipNum = getIPs(&ipTable, broadIP, PING_PACKETS);

		//Try to connect with the new devices (if any)
		pthread_mutex_lock(&socketMutex);
		for(i=0;i<ipNum;i++)
		{
			if(ipExists(socketList, MAX_SOCKETS, ipTable[i])) continue;

			// Create new socket
			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				printf("Socket creation error \n");
				continue;
			}

			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(SERVER_PORT);

			// Convert IPv4 and IPv6 addresses from text to binary form
			if(inet_pton(AF_INET, ipTable[i], &serv_addr.sin_addr)<=0)
			{
				printf("Invalid address/ Address not supported \n");
				close(sock);
				continue;
			}

			pthread_mutex_lock(&terminateMutex); run = !(*terminate); pthread_mutex_unlock(&terminateMutex);
			if(!run){ close(sock); break;} //Don't make new connections

			//Try to make connection
			if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
			{
				// printf("Connection Failed \n");
				close(sock);
				continue;
			}

			//Add the new socket to the array of sockets
			for (j = 0; j < MAX_SOCKETS; j++)
			{
				//if position is empty
				if(socketList[j] == 0 )
				{
					socketList[j] = sock;
					gettimeofday(&tv, NULL);
					printf("client_device_connected : %" PRIu64 " : %s\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000, inet_ntoa(serv_addr.sin_addr));
					break;
				}
			}

		}
		pthread_mutex_unlock(&socketMutex);
		if(ipNum>0)free2d(ipTable, ipNum);


		//Send Messages
		for (i = 0; i < MAX_SOCKETS; i++)
		{
			pthread_mutex_lock(&socketMutex);
			sock = socketList[i];
			pthread_mutex_unlock(&socketMutex);

			//For every connected server
			if(!sock) continue;
			getpeername(sock, (struct sockaddr*)&serv_addr , (socklen_t*)&addrlen);
			tempAEM = getAEM(inet_ntoa(serv_addr.sin_addr));

			//Find wich messages in buffer have not been sent yet and send them
			pthread_mutex_lock(&bufferMutex);
			for(j=0;j<CIRC_BUFFER_SIZE;j++)
			{
				if(strlen(circBuffer[j])==0) break; //Buffer empty from this point on.So stop looking.
				tempPos = canSend(circBuffer[j], MSG_SIZE, sendList[j], MAX_SOCKETS, thisAEM, tempAEM);
				if(tempPos != -1)
				{
					if(send(sock, circBuffer[j], MSG_SIZE, 0)==-1)
					{
						// printf("send_error : %d\n", errno);
						//If there is a problem with send() (for example broken pipe), stop trying to send messages
						break;
					}
					else
					{
						sendList[j][tempPos] = tempAEM;
						gettimeofday(&tv, NULL);
						printf("sent_message : %" PRIu64 " : %s\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000, inet_ntoa(serv_addr.sin_addr));
					}

				}
			}
			pthread_mutex_unlock(&bufferMutex);

		}
		pthread_mutex_lock(&terminateMutex); run = !(*terminate); pthread_mutex_unlock(&terminateMutex);
	}

	gettimeofday(&tv, NULL);
	printf("client_finished : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	pthread_exit((void*)tid);
}

void server(void* args)
{
	int opt = TRUE;
	int master_socket, new_socket, activity, i, valread, sd, max_sd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	struct timeval selectTimeout, tv;
	int run;
	char buffer[MSG_SIZE]; //data buffer
	fd_set readfds; //set of socket descriptors for reading

	//Arguements
	server_data *myData = (server_data*) args;
	cbuf_handle_t cbuf = *(myData->cbuf);
	char **circBuffer = myData->circBuffer;
	int** sendList = myData->sendList;
	int tid = myData->tid;
	int* socketList = myData->socketList;
	int* terminate = myData->terminate;

	//create a master socket
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		printf("socket failed\n");
		// perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//set master socket to allow multiple connections
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0 )
	{
		printf("setsockopt\n");
		// perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(SERVER_PORT);

	//bind the socket to localhost port SERVER_PORT
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		printf("bind failed\n");
		// perror("bind failed");
		exit(EXIT_FAILURE);
	}

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0)
	{
		printf("listen\n");
		// perror("listen");
		exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	pthread_mutex_lock(&terminateMutex); run = !(*terminate); pthread_mutex_unlock(&terminateMutex);
	while(run)
	{
		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		//add child sockets to set
		for ( i = 0 ; i < MAX_SOCKETS ; i++)
		{
			//socket descriptor
			pthread_mutex_lock(&socketMutex);
			sd = socketList[i];
			pthread_mutex_unlock(&socketMutex);

			//if valid socket descriptor then add to read list
			if(sd > 0) FD_SET( sd , &readfds);

			//highest file descriptor number, need it for the select function
			if(sd > max_sd) max_sd = sd;
		}

		//wait for an activity on one of the sockets for 2 seconds
		selectTimeout.tv_sec = 2; selectTimeout.tv_usec = 0;
		activity = select( max_sd + 1 , &readfds , NULL , NULL , &selectTimeout);

		if ((activity < 0) && (errno!=EINTR))
		{
			printf("select error\n");
			continue;
		}

		//If something happened on the master socket, then its an incoming connection
		if (FD_ISSET(master_socket, &readfds))
		{
			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				printf("accept_error\n");
				// perror("accept");
				// exit(EXIT_FAILURE);
			}
			else
			{
				//add new socket to array of sockets
				pthread_mutex_lock(&terminateMutex); run = !(*terminate); pthread_mutex_unlock(&terminateMutex);
				if(run)
				{
					pthread_mutex_lock(&socketMutex);
					for (i = 0; i < MAX_SOCKETS; i++)
					{
						//if position is empty
						if(socketList[i] == 0 )
						{
							socketList[i] = new_socket;
							gettimeofday(&tv, NULL);
							printf("server_device_connected : %" PRIu64 " : %s\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000, inet_ntoa(address.sin_addr));
							break;
						}
					}
					pthread_mutex_unlock(&socketMutex);
				}else close(new_socket);
			}

		}

		//else its some IO operation on some other socket
		for (i = 0; i < MAX_SOCKETS; i++)
		{
			pthread_mutex_lock(&socketMutex);
			sd = socketList[i];
			pthread_mutex_unlock(&socketMutex);

			if(FD_ISSET( sd , &readfds))
			{
				//Get socket details
				getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);

				//Check if it was for closing , and also read the incoming message
				if ((valread = read( sd , buffer, MSG_SIZE)) == 0)
				{
					//Close the socket and mark as 0 in list for reuse
					pthread_mutex_lock(&socketMutex);
					gettimeofday(&tv, NULL);
					printf("server_device_disconnected : %" PRIu64 " : %s\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000, inet_ntoa(address.sin_addr));
					close( sd );
					socketList[i] = 0;
					pthread_mutex_unlock(&socketMutex);
				}
				//Store in buffer the message that came in
				else
				{
					//Set the string terminating NULL byte on the end of the data read
					buffer[valread] = '\0';
					gettimeofday(&tv, NULL);

					//Atomic buffer acces to save message
					pthread_mutex_lock (&bufferMutex);
					//Don't put an incoming message in buffer if it already exists
					if(!messageExists(circBuffer, CIRC_BUFFER_SIZE, buffer))
					{
						memset(sendList[circular_buf_head(cbuf)], 0, MAX_SOCKETS*sizeof(int));
						sendList[circular_buf_head(cbuf)][0] = getAEM(inet_ntoa(address.sin_addr));
						circular_buf_put(cbuf, buffer);
						printf("received_accepted_message : %" PRIu64 " : %s\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000, inet_ntoa(address.sin_addr));
					}
					else
					{
						printf("received_rejected_message : %" PRIu64 " : %s\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000, inet_ntoa(address.sin_addr));
					}
			    pthread_mutex_unlock (&bufferMutex);
				}
			}
		}
		pthread_mutex_lock(&terminateMutex); run = !(*terminate); pthread_mutex_unlock(&terminateMutex);
	}

	//close master socket
	close(master_socket);
	pthread_mutex_lock(&socketMutex);
	//close all open sockets
	for(i=0;i<MAX_SOCKETS;i++)
	{
		if(socketList[i])
		{
			getpeername(socketList[i], (struct sockaddr*)&address, (socklen_t*)&addrlen);
			gettimeofday(&tv, NULL);
			printf("server_device_disconnected : %" PRIu64 " : %s\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000, inet_ntoa(address.sin_addr));
			close(socketList[i]);
			socketList[i]=0;
		}
	}
	pthread_mutex_unlock(&socketMutex);

	gettimeofday(&tv, NULL);
	printf("server_finished : %" PRIu64 "\n", (uint64_t)tv.tv_usec+(uint64_t)tv.tv_sec*(uint64_t)1000000);
	pthread_exit((void*)tid);
}
