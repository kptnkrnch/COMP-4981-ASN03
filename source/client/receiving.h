#ifndef RECEIVING_H
#define RECEIVING_H

#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <fstream>


void ReadServerName(uint8_t * buffer, uint32_t buffersize, std::ofstream * file, bool saveToFile);
void ReadClientName(uint8_t * buffer, uint32_t buffersize, std::ofstream * file, bool saveToFile);
void ReadChatMessage(uint8_t * buffer, uint32_t buffersize, std::ofstream * file, bool saveToFile);
void ReadClientDisconnect(uint8_t * buffer, uint32_t buffersize, std::ofstream * file, bool saveToFile);

#endif
