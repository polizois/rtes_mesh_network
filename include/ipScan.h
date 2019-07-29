#ifndef IP_SCAN_H_
#define IP_SCAN_H_

int getIPs(char ***ipTable, char* broadcastIP, int packetCount);
void free2d(char **ptr, int rows);
int ipExists(int* server_socket, int serverNum, char* ip);
int myAEM(char *dev);
int getAEM(char *ip);

#endif
