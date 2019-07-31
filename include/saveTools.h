#ifndef SAVE_TOOLS_H_
#define SAVE_TOOLS_H_

#include "definitions.h"
#include "circular_buffer.h"


// Saves 2d array "sendlist" in a file defined by "filePath"
// Needs "circBuffer" because "sendList" follows its content
void writeSendList(char* filePath, int **sendList, char **circBuffer);

// Loads 2d array "sendlist" from a file defined by "filePath"
int loadSendList(char* filePath, int **sendList);

// Saves string array "circBuffer" in a file defined by "filePath"
void writeCircBuffer(char* filePath, char **circBuffer);

// Reads a file defined by "filePath" and puts its content in circular buffer "cbuf".
// "cbuf" is just a handle and uses a specific way to insert content(see circular_buffer.h)
// but in reality the content gets stored in a simple string array
int loadCircBuffer(char* filePath, cbuf_handle_t cbuf);

#endif
