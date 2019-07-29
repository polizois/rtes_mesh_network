#ifndef MSG_TOOLS_H_
#define MSG_TOOLS_H_

long int actualMessage(char* buffer, size_t size, char endChar);
int generateMessage(char* message, uint32_t from, uint32_t to, size_t min, size_t max);
int messageExists(char** circBuffer, int bufferSize, char* message);
int messageOrigin(char* message, size_t size);
int messageDestination(char* message, size_t size);
int canSend(char* message, int messageSize, int* list, int listSize, int originAEM, int destinationAEM);

#endif
