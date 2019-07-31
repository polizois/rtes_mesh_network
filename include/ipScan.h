#ifndef IP_SCAN_H_
#define IP_SCAN_H_


// Finds available ips in network by pinging the broadcast ip(broadcastIP) with a number (packetCount) of packets
// Allocates memory to **ipTable as a 2d char table and stores an ip as string in each row
// Returns the number of fount ips
int getIPs(char ***ipTable, char* broadcastIP, int packetCount);

// Frees a 2d char array given its pointer and its rows
void free2d(char **ptr, int rows);

// Given an array of socket descriptors, its size and an ip(string)
// the function returns the position of the socket that has this ip
// Returns -1 if no socket has thid ip
int ipExists(int* server_socket, int serverNum, char* ip);

// Grabs the last 2 sections of the given ip and returns them as interger
// For example if the ip is 10.0.85.35, it returns 8535
int getAEM(char *ip);

// Given the name of the network device in use, the function returns the
// device's ip
char *myIP(char *dev);

// Given the name of the network device in use, the function returns the broadcast ip of the network
char *broadcastIP(char *dev);

#endif
