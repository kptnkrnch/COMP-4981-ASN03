#ifndef SENDING_H
#define SENDING_H

#include <stdint.h>
#include <stdio.h>

void Create_ClientName_Packet(uint8_t ** buffer, uint32_t bufferSize, char * username, uint32_t namesize);
void Create_ChatMessage_Packet(uint8_t ** buffer, uint32_t bufferSize, char * username, uint32_t namesize, char * message, uint32_t messagesize);
void Create_Disconnect_Packet(uint8_t ** buffer, uint32_t bufferSize, char * username, uint32_t namesize);

#endif
