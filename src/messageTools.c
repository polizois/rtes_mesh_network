#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/time.h>

#include "messageTools.h"
#include "randString.h"

long int actualMessage(char* buffer, size_t size, char endChar)
{
	int i=0;

	for(i=0;i<size;i++)
	{
		if(buffer[i]==endChar)
		{
			buffer[i]='\0';
			break;
		}
	}

	return i;
}

int generateMessage(char* message, uint32_t from, uint32_t to, size_t min, size_t max)
{
	struct timeval tv;
	char temp[max];

	gettimeofday(&tv, NULL);
	rand_string(temp, rand_number(min, max));

  sprintf(message, "%u_%u_%" PRIu64 "_", from, to, tv.tv_usec+tv.tv_sec*1000000);
  strcat(message, temp);

	return strlen(message);
}

int messageExists(char** circBuffer, int bufferSize, char* message)
{
	int i;

	for(i=0;i<bufferSize;i++)
	{
		//If we reached an empty spot in buffer, message doesn't exist
		if(strlen(circBuffer[i])==0) return 0;
		//If we find a matching message in buffer, message exists
		if(!strcmp(circBuffer[i], message)) return 1;
	}

	return 0;
}

int messageOrigin(char* message, size_t size)
{
	// char *token;
	char tempMessage[size];
	// char temp[7];

	strcpy(tempMessage, message);

	return atoi(strtok(tempMessage, "_"));
}

int messageDestination(char* message, size_t size)
{
	// char *token;
	char tempMessage[size];
	// char temp[7];

	strcpy(tempMessage, message);
	strtok(tempMessage, "_");

	return atoi(strtok(NULL, "_"));
}

int canSend(char* message, int messageSize, int* list, int listSize, int originAEM, int destinationAEM)
{
	int i;

	//IF the message ORIGIN is the device I'm trying to send it to, don't send the message
	if(messageOrigin(message, messageSize)==destinationAEM) return -1;
	//IF the message DESTINATION is THIS DEVICE, don't send the message
	if(messageDestination(message, messageSize)==originAEM) return -1;

	for(i=0;i<listSize;i++)
	{
		if(list[i]==0) break;      // List Empty from now on, so AEM not in List
		if(list[i]==destinationAEM) return -1; // AEM exists in List
	}

	if(i==listSize) return -1;
	else return i;
}
