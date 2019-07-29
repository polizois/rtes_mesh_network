#ifndef THREADS_H_
#define THREADS_H_

typedef struct{
	int minInterval;
  int maxInterval;
	int tid;
	cbuf_handle_t* cbuf;
	int** sendList;
	int* terminate;
}generator_data;

typedef struct{
	int tid;
	cbuf_handle_t* cbuf;
	int** sendList;
	char** circBuffer;
	int* socketList;
	int* terminate;
}server_data;

typedef struct{
	int tid;
	cbuf_handle_t* cbuf;
	int** sendList;
	char** circBuffer;
	int* socketList;
	int* terminate;
}client_data;

pthread_mutex_t bufferMutex;
pthread_mutex_t socketMutex;
pthread_mutex_t terminateMutex;

//Thread Functions
void msgGenerator(void* args);
void client(void* args);
void server(void* args);

#endif
