#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "ipScan.h"

int getIPs(char ***table, char* broadcastIP, int packetCount)
{
  FILE *pf;
  char command[256];
  char buffer[16];
  int ipNum = 0;
  char **ipTable=NULL;

  // Setup our pipe for reading and execute our command.
  sprintf(command, "ip neigh flush all");
  pf = popen(command, "r");
  while ( fgets( buffer, 16, pf) != NULL){}
  pclose(pf);
  sprintf(command, "ping -c%d -b %s 2>/dev/null", packetCount, broadcastIP);
  // sprintf(command, "ping -c%d -b %s 2>&1", packetCount, broadcastIP);
  pf = popen(command, "r");
  while ( fgets( buffer, 16, pf) != NULL){}
  pclose(pf);
  sprintf(command, "arp -n | grep -v \"incomplete\" | grep -v \"Address\" | grep -Eo '^[^ ]+'");
  pf = popen(command,"r");
  // printf("pf == %d\n", (int)pf);

  // Get the data from the process execution
  while ( fgets( buffer, 16, pf) != NULL)
  {
    ipNum++;

    // Memory Allocation/Reallocation
    ipTable = (char **)realloc(ipTable, ipNum*sizeof(char *));
    // for(int i=0;i<16;i++) ipTable[ipNum-1] = (char *)malloc(16*sizeof(char));
    ipTable[ipNum-1] = (char *)malloc(16*sizeof(char));

    //Store new ip in table
    buffer[strcspn(buffer, "\n")] = 0;
    strcpy(ipTable[ipNum-1], buffer);
  }

  int er = pclose(pf);
  // printf("pclose: %d\n", er);
  if (er != 0)
  {
    // fprintf(stderr," Error: Failed to close command stream \n");
    return -1;
  }

  *table = ipTable;
  return ipNum;
}


void free2d(char **ptr, int rows)
{
  for(int i=0;i<rows;i++) free(ptr[i]);
  free(ptr);
}

int ipExists(int* server_socket, int serverNum, char* ip)
{
	struct sockaddr_in serv_addr;
	int addrlen = sizeof(serv_addr);
	int i;

	for (i = 0; i < serverNum; i++)
	{
		if( server_socket[i] == 0 ) continue;

		getpeername(server_socket[i], (struct sockaddr*)&serv_addr, (socklen_t*)&addrlen);
		if(!strcmp(ip, inet_ntoa(serv_addr.sin_addr))) return 1;
	}

	return 0;
}

char *myIP(char *dev)
{
  FILE *pf;
  char command[256];
  static char buffer[16];

  sprintf(command, "ifconfig %s | grep inet | awk '{split($0,a,\" \"); print a[2]}'", dev);
  pf = popen(command,"r");
  fgets(buffer, 16, pf);

  if(pclose(pf)) return NULL;
	return buffer;
}

char *broadcastIP(char *dev)
{
	FILE *pf;
	char command[256];
	static char buffer[16];

	sprintf(command, "ifconfig %s | grep broadcast | awk '{split($0,a,\" \"); print a[6]}'", dev);
	pf = popen(command,"r");
	fgets(buffer, 16, pf);

	if(pclose(pf)) return NULL;
  buffer[strcspn(buffer, "\n")] = '\0';
	return buffer;
}


int getAEM(char *ip)
{
	char *token;
	char tempIP[16];
	char temp[7];

	strcpy(tempIP, ip);

	strtok(tempIP, ".");
	strtok(NULL, ".");
	strcpy(temp, strtok(NULL, "."));
	strcat(temp, strtok(NULL, "."));

	return atoi(temp);
}
