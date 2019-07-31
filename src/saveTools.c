#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "saveTools.h"

int loadCircBuffer(char* filePath, cbuf_handle_t cbuf)
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
