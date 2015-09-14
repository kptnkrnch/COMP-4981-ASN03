#include "sending.h"
#include "defines.h"
#include <stdint.h>

/*----------------------------------------------------------------------
--FUNCTION: Create_ClientName_Packet
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell and Ian Davidson
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: uint8_t ** buffer - the packet that is going to be formatted
							  with the disconnect data.
--		  uint32_t buffersize - max amount of data in the buffer array
--		  char * username - pointer to the array containing the client's
							name/alias.
--		  uint32_t namesize - the number of characters in the name/alias.
--
--RETURNS: void
--
--NOTES:
--Function for creating a packet that alerts the server that a new client
--(this client) has joined the chat.
----------------------------------------------------------------------*/
void Create_ClientName_Packet(uint8_t ** buffer, uint32_t bufferSize, char * username, uint32_t namesize) {
    uint32_t pos = 0;
    uint32_t shift = 0;
    (*buffer)[pos] = 0x16;
    pos++;
    (*buffer)[pos] = CLIENTNAME;
    pos++;
    namesize++;
    for (uint32_t i = 0, shift = 24; i < 4 && shift >= 0; i++) {
        (*buffer)[pos] = (uint8_t)(namesize >> shift);
        pos++;
        shift -= 8;
    }

    for (uint32_t i = 0; i < namesize; i++) {
        (*buffer)[pos] = username[i];
        pos++;
    }
    (*buffer)[pos] = '\0';
    pos++;

    for (uint32_t i = 0; i < (((uint32_t)MAXNAMESIZE) - namesize); i++) {
        (*buffer)[pos] = 0;
        pos++;
    }
    (*buffer)[pos] = 0x04;
    pos++;

    for (uint32_t i = pos; i < bufferSize; i++) {
        (*buffer)[i] = 0;
    }
}

/*----------------------------------------------------------------------
--FUNCTION: Create_ChatMessage_Packet
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell and Ian Davidson
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: uint8_t ** buffer - the packet that is going to be formatted
							  with the disconnect data.
--		  uint32_t buffersize - max amount of data in the buffer array
--		  char * username - pointer to the array containing the client's
							name/alias.
--		  uint32_t namesize - the number of characters in the name/alias.
--        char * message - the chat message to be sent
--		  uint32_t messagesize - the size of the message
--
--RETURNS: void
--
--NOTES:
--Function for creating a packet that sends a chat message to the server
--which then forwards it to the other clients.
--note, this client's name is appended.
----------------------------------------------------------------------*/
void Create_ChatMessage_Packet(uint8_t ** buffer, uint32_t bufferSize, char * username, uint32_t namesize, char * message, uint32_t messagesize) {
    uint32_t pos = 0;
    uint32_t shift = 0;
    uint8_t packet[BUFLEN];
    (*buffer)[pos] = 0x16;
    pos++;
    (*buffer)[pos] = CHATMESSAGE;
    pos++;
    namesize++;
    for (uint32_t i = 0, shift = 24; i < 4 && shift >= 0; i++) {
        (*buffer)[pos] = (uint8_t)(namesize >> shift);
        pos++;
        shift -= 8;
    }

    for (uint32_t i = 0; i < namesize; i++) {
        (*buffer)[pos] = username[i];
        pos++;
    }
    (*buffer)[pos] = '\0';
    pos++;

    for (uint32_t i = 0; i < (((uint32_t)MAXNAMESIZE) - namesize); i++) {
        (*buffer)[pos] = 0;
        pos++;
    }

    for (uint32_t i = 0, shift = 24; i < 4 && shift >= 0; i++) {
        (*buffer)[pos] = (uint8_t)(messagesize >> shift);
        pos++;
        shift -= 8;
    }

    for (uint32_t i = 0; i < messagesize; i++) {
        (*buffer)[pos] = message[i];
        pos++;
    }
    (*buffer)[pos] = '\0';
    pos++;

    (*buffer)[pos] = 0x04;
    pos++;

    for (uint32_t i = pos; i < bufferSize; i++) {
        (*buffer)[i] = 0;
    }

    for (uint32_t i = 0; i < BUFLEN; i++) {
        packet[i] = (*buffer)[i];
    }
}

/*----------------------------------------------------------------------
--FUNCTION: Create_Disconnect_Packet
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell and Ian Davidson
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: uint8_t ** buffer - the packet that is going to be formatted
							  with the disconnect data.
--		  uint32_t buffersize - max amount of data in the buffer array
--		  char * username - pointer to the array containing the client's
							name/alias.
--		  uint32_t namesize - the number of characters in the name/alias.
--
--RETURNS: void
--
--NOTES:
--Function for creating a packet that alerts the server and clients of 
--a client disconnect.
----------------------------------------------------------------------*/
void Create_Disconnect_Packet(uint8_t ** buffer, uint32_t bufferSize, char * username, uint32_t namesize) {
    uint32_t pos = 0;
    uint32_t shift = 0;
    (*buffer)[pos] = 0x16;
    pos++;
    (*buffer)[pos] = CLIENTREMOVED;
    pos++;
    namesize++;
    for (uint32_t i = 0, shift = 24; i < 4 && shift >= 0; i++) {
        (*buffer)[pos] = (uint8_t)(namesize >> shift);
        pos++;
        shift -= 8;
    }

    for (uint32_t i = 0; i < namesize; i++) {
        (*buffer)[pos] = username[i];
        pos++;
    }
    (*buffer)[pos] = '\0';
    pos++;

    for (uint32_t i = 0; i < (MAXNAMESIZE - namesize); i++) {
        (*buffer)[pos] = 0;
        pos++;
    }
    (*buffer)[pos] = 0x04;
    pos++;

    for (uint32_t i = pos; i < bufferSize; i++) {
        (*buffer)[i] = 0;
    }
}
