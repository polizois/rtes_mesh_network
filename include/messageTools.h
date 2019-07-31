#ifndef MSG_TOOLS_H_
#define MSG_TOOLS_H_

// Accepts a string buffer of size "size" and a character "endChar"
// that sympbolizes the end of the usefull message inside the buffer
// Replaces the ending character with "\0" and returns the length of the usefull message
long int actualMessage(char* buffer, size_t size, char endChar);

// Generates a random message of size between "min" and "max" that will have origin "from" and destination "to".
// Stores the message in "message" in the form: origin_destination_creationTime_actualMessage
// where creationTime is the timestanp on which the message was generated.
// Returns the length of the whole message (with the added info)
int generateMessage(char* message, uint32_t from, uint32_t to, size_t min, size_t max);

// Given an array("circBuffer") that can hold "bufferSize" strings and a string("message"),
// the function returns 1 if the message exists in the array and 0 if it doesn't.
int messageExists(char** circBuffer, int bufferSize, char* message);

// Given a string("message") of this form: origin_destination_creationTime_actualMessage,
// the function extracts the "origin" part and returns it as an interger
int messageOrigin(char* message, size_t size);

// Given a string("message") of this form: origin_destination_creationTime_actualMessage,
// the function extracts the "destination" part and returns it as an interger
int messageDestination(char* message, size_t size);

// Given a message, a list of devices that it has been sent to, this device's id ("thisAEM")
// and the id of the device that we want to send the message to ("destinationAEM"),
// the function returns -1 if the message cannot be sent to this destination
// or an interger from 0 to listSize-1 that indicates the empty spot in the list that will be filled
// by the destination decice's id after the message has been sent
int canSend(char* message, int messageSize, int* list, int listSize, int thisAEM, int destinationAEM);

#endif
