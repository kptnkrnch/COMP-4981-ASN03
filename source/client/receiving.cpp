#include "receiving.h"
#include "defines.h"
#include <stdint.h>
#include <iostream>
#include <fstream>

using namespace std;

/*----------------------------------------------------------------------
--FUNCTION: ReadClientName
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell and Ian Davidson
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: uint8_t * buffer - the received data from the socket (array)
--		  uint32_t buffersize - max amount of data in the buffer array
--		  ofstream * file - pointer to the file for saving chat data
--		  bool saveToFile - flag for whether we are saving to a file or not.
--
--RETURNS: void
--
--NOTES:
--Function for extracting the server's name from a packet and then 
--printing/saving said server name.
----------------------------------------------------------------------*/
void ReadServerName(uint8_t * buffer, uint32_t buffersize, ofstream * file, bool saveToFile) {
    uint32_t pos = 2;
    uint32_t namesize = 0;
    char name[50];

    namesize = ((uint32_t)buffer[pos + 0] << 24 | (uint32_t)buffer[pos + 1] << 16 | (uint32_t)buffer[pos + 2] << 8 | (uint32_t)buffer[pos + 3] << 0);
    pos += 4;

    for (uint32_t i = 0; i < namesize; i++) {
        name[i] = buffer[pos];
        pos++;
    }

    printf("Joined server: %s\n", name);

    if (saveToFile) {
        *file << "Joined server: " << name << "\n";
    }
}

/*----------------------------------------------------------------------
--FUNCTION: ReadClientName
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell and Ian Davidson
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: uint8_t * buffer - the received data from the socket (array)
--		  uint32_t buffersize - max amount of data in the buffer array
--		  ofstream * file - pointer to the file for saving chat data
--		  bool saveToFile - flag for whether we are saving to a file or not.
--
--RETURNS: void
--
--NOTES:
--Function for extracting new client data from a packet and then 
--printing/saving which client joined.
----------------------------------------------------------------------*/
void ReadClientName(uint8_t * buffer, uint32_t buffersize, ofstream * file, bool saveToFile) {
    uint32_t pos = 2;
    uint32_t namesize = 0;
    char name[50];

    namesize = ((uint32_t)buffer[pos + 0] << 24 | (uint32_t)buffer[pos + 1] << 16 | (uint32_t)buffer[pos + 2] << 8 | (uint32_t)buffer[pos + 3] << 0);
    pos += 4;

    for (uint32_t i = 0; i < namesize; i++) {
        name[i] = buffer[pos];
        pos++;
    }

    printf("%s has joined the chat.\n", name);

    if (saveToFile) {
        *file << name << " has joined the chat.\n";
    }
}

/*----------------------------------------------------------------------
--FUNCTION: ReadChatMessage
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell and Ian Davidson
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: uint8_t * buffer - the received data from the socket (array)
--		  uint32_t buffersize - max amount of data in the buffer array
--		  ofstream * file - pointer to the file for saving chat data
--		  bool saveToFile - flag for whether we are saving to a file or not.
--
--RETURNS: void
--
--NOTES:
--Function for extracting message data from a packet and then printing/saving
--message output and which client sent it.
----------------------------------------------------------------------*/
void ReadChatMessage(uint8_t * buffer, uint32_t buffersize, ofstream * file, bool saveToFile) {
    uint32_t pos = 2;
    uint32_t namesize = 0;
    uint32_t msgsize = 0;
    char name[50];
    char message[2000];

    for (int i = 0; i < 2000; i++) {
        message[i] = '\0';
    }

    namesize = ((uint32_t)buffer[pos + 0] << 24 | (uint32_t)buffer[pos + 1] << 16 | (uint32_t)buffer[pos + 2] << 8 | (uint32_t)buffer[pos + 3] << 0);
    pos += 4;

    for (uint32_t i = 0; i < namesize; i++) {
        name[i] = buffer[pos];
        pos++;
    }

    for (uint32_t i = 0; i < (((uint32_t)MAXNAMESIZE) - namesize); i++) {
        pos++;
    }
    pos++;

    msgsize = ((uint32_t)buffer[pos + 0] << 24 | (uint32_t)buffer[pos + 1] << 16 | (uint32_t)buffer[pos + 2] << 8 | (uint32_t)buffer[pos + 3] << 0);
    pos += 4;

    for (uint32_t i = 0; i < msgsize; i++) {
        message[i] = buffer[pos];
        pos++;
    }

    printf("\n%s: ", name);

    printf("%s\n", message);

    if (saveToFile) {
        *file << "\n" << name << ":" << message << "\n";
    }
}

/*----------------------------------------------------------------------
--FUNCTION: ReadClientDisconnect
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell and Ian Davidson
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: uint8_t * buffer - the received data from the socket (array)
--		  uint32_t buffersize - max amount of data in the buffer array
--		  ofstream * file - pointer to the file for saving chat data
--		  bool saveToFile - flag for whether we are saving to a file or not.
--
--RETURNS: void
--
--NOTES:
--Function for extracting disconnect data from a packet and then printing/saving
--disconnect output.
----------------------------------------------------------------------*/
void ReadClientDisconnect(uint8_t * buffer, uint32_t buffersize, ofstream * file, bool saveToFile) {
    uint32_t pos = 2;
    uint32_t namesize = 0;
    char name[50];

    namesize = ((uint32_t)buffer[pos + 0] << 24 | (uint32_t)buffer[pos + 1] << 16 | (uint32_t)buffer[pos + 2] << 8 | (uint32_t)buffer[pos + 3] << 0);
    pos += 4;

    for (uint32_t i = 0; i < namesize; i++) {
        name[i] = buffer[pos];
        pos++;
    }

    printf("%s has left the chat.\n\n", name);
    if (saveToFile) {
        *file << name << "has left the chat.\n\n";
    }
}
