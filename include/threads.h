#ifndef THREADS_H_
#define THREADS_H_

// Struct used to pass arguements to "generator" thread
typedef struct{
	int minInterval;
  int maxInterval;
	int tid;
	cbuf_handle_t* cbuf;
	int** sendList;
	int* terminate;
}generator_data;

// Struct used to pass arguements to "server" thread
typedef struct{
	int tid;
	cbuf_handle_t* cbuf;
	int** sendList;
	char** circBuffer;
	int* socketList;
	int* terminate;
}server_data;

// Struct used to pass arguements to "client" thread
typedef struct{
	int tid;
	cbuf_handle_t* cbuf;
	int** sendList;
	char** circBuffer;
	int* socketList;
	int* terminate;
}client_data;

// Mutex key for thread safe access to the message storage buffer
pthread_mutex_t bufferMutex;
// Mutex key for thread safe access to the connected socket array
pthread_mutex_t socketMutex;
// Mutex key for thread safe access to the terminate variable used by the threads
// to determine whether they should terminate or not
pthread_mutex_t terminateMutex;

//Thread Functions

// Generates a random message in the form of: origin_destination_creationTime_randomMessage
// on a randomly variable period between "minInterval" and "maxInterval" minutes.
// - origin is the id of this device
// - destination is the id of the device for which the message is destined
// - creationTime is the timestamp on which the message was created
// - randmMessage is a random message of random size between 10 and 200
// These messeges are stored in "circBuffer".
void generator(void* args);

// Scans the network to find available devices and attempts connection with the newfound ones.
// New connections get added in "socketList".
// For every connected device it scans the "circBuffer" and "sendList" and attempts to send the messages
// that have not already been sent to it.(after a successful send it updates the "sendList").
void client(void* args);

// Listens on port SERVER_PORT (defined in definitions.h) and accepts incoming connections and
// adds them in "socketList".
// Accepts incoming messages from connected devices and stores them in "circBuffer".
// Closes broken connections and removes them from "socketList".
void server(void* args);

#endif
