//
//  parser.c
//  protocol
//
//  Created by Mike Dice on 12/2/17.
//  Copyright © 2017 Mike Dice. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"


// Keep track of the thing we are parsing
enum parserState{
    Ready0, // default state
    Ready1, // [
    Ready2, // -
    Ready3, // -
    Ready4, // \r
    Ready5, // \n
    RH1, // P
    RH2, // K
    RH3, // T
    RH4, // .
    RH5, // V
    RH6, // 0-9 1-*
    RH7, // ' '
    RH8, // A-F 1-*
    RH9, // \r,
    RH10,  // \n
    LEN1, // L
    LEN2, // E
    LEN3, // N
    LEN4, // :
    LEN5, // 0-9 1-*
    LEN6, // \r
    LEN7, // \n
    RecvData,
    EndP1, // -
    EndP2, // -
    EndP3  //]
    
};

// Internal data used by the parser
enum parserState state = Ready0;

unsigned int CurrDataLen = 0;
unsigned int CurrDataRecvd = 0;
unsigned char PacketSig[MAX_PACKETSIG];
unsigned char PacketCommand[MAX_PACKETCOMMAND];
unsigned char PacketData[MAX_PACKETDATA];
unsigned char PacketLen[MAX_PACKETLEN];

unsigned int PacketSigIdx = 0;
unsigned int PacketCommandIdx = 0;
unsigned int PacketDataIdx = 0;
unsigned int PacketLenIdx = 0;
Packet resultPacket;


Packet* CreatePacket(){
    resultPacket.Sig = PacketSig;
    resultPacket.Sig[PacketSigIdx] = '\0';
    
    resultPacket.Command = PacketCommand;
    resultPacket.Command[PacketCommandIdx] = '\0';
    
    PacketLen[PacketLenIdx] = '\0';
    resultPacket.DataLength = atoi((const char*)PacketLen);
    
    if (resultPacket.DataLength > 0)
    {
        resultPacket.Data = PacketData;
    }
    else
    {
        resultPacket.Data = 0;
    }

    return &resultPacket;
}

void DeletePacket(Packet* packet)
{
    // TODO
}

/*
 The heap has to be avoided on the arduino because that device has
 almost no heap. Deprecated these functions and replaced with
 ones that only use static data
 
 
// When parsing of a packet is done this thing constructs a packet
Packet* CreatePacket()
{
    Packet *packet = (Packet*)malloc(sizeof(Packet));
    memcpy(packet->Sig, PacketSig, PacketSigIdx);
    packet->Sig[PacketSigIdx] = '\0';
    
    memcpy(packet->Command, PacketCommand, PacketCommandIdx);
    packet->Command[PacketCommandIdx] = '\0';
    
    memcpy(packet->Data, PacketData, PacketDataIdx);
    
    PacketLen[PacketLenIdx] = '\0';
    packet->DataLength = atoi((const char*)PacketLen);
    return packet;
}

void DeletePacket(Packet *packet)
{
    if (packet != NULL){
        free(packet);
    }
}
*/

void SetDataLen(){
    PacketLen[PacketLenIdx] = '\0';
    CurrDataLen = atoi((const char*)PacketLen);
    CurrDataRecvd = 0;
}

void ResetParser(){
    memset(PacketSig, sizeof(PacketSig), 0);
    memset(PacketCommand, sizeof(PacketCommand), 0);
    memset(PacketData, sizeof(PacketData), 0);
    memset(PacketLen, sizeof(PacketLen), 0);
    PacketSigIdx = 0;
    PacketCommandIdx = 0;
    PacketDataIdx = 0;
    PacketLenIdx = 0;
    
    CurrDataLen = 0;
    state = Ready0;
    CurrDataRecvd = 0;
}

void AddPacketSigData(unsigned char byte){
    if (PacketSigIdx < MAX_PACKETSIG){
        PacketSig[PacketSigIdx++] = byte;
    }
}

void AddPacketCommandData(unsigned char byte){
    if (PacketCommandIdx < MAX_PACKETCOMMAND){
        PacketCommand[PacketCommandIdx++] = byte;
    }
}

void AddPacketData(unsigned char byte){
    if (PacketDataIdx < MAX_PACKETDATA){
        PacketData[PacketDataIdx++] = byte;
        CurrDataRecvd++;
    }
}

void AddPacketLenData(unsigned char byte){
    if (PacketLenIdx < MAX_PACKETLEN){
        PacketLen[PacketLenIdx++] = byte;
    }
}

BOOL ProcessStreamByte(unsigned char byte, ProcessPacketFunc processPacket)
{
    BOOL result = TRUE;
    // receiving ready sequence
    if (state == Ready0 && byte == '['){
        state = Ready1;
    }
    else if (state == Ready1 && byte == '-'){
        state = Ready2;
    }
    else if (state == Ready2 && byte == '-'){
        state = Ready3;
    }
    else if (state == Ready3 && byte == '\r'){
        state = Ready4;
    }
    else if (state == Ready4 && byte == '\n'){
        state = Ready5;
    }
    
    
    // receiving header
    else if (state == Ready5 && byte == 'P'){
        state = RH1;
        AddPacketSigData(byte);
    }
    else if (state == RH1 && byte == 'K'){
        state = RH2;
        AddPacketSigData(byte);
    }
    else if (state == RH2 && byte == 'T'){
        state = RH3;
        AddPacketSigData(byte);
    }
    else if (state == RH3 && byte == '.'){
        state = RH4;
        AddPacketSigData(byte);
    }
    else if (state == RH4 && byte == 'V'){
        state = RH5;
        AddPacketSigData(byte);
    }
    
    // receiving version of protocol
    else if ((state == RH5 || state == RH6) && byte >= '0' && byte <= '9'){
        state = RH6;
        AddPacketSigData(byte);
    }
    
    // receving packet command
    else if (state == RH6 && (byte == ' ' || byte == '\t')){
        state = RH7;
    }
    else if ((state == RH7 || state == RH8) && byte >= 'A' && byte <= 'Z'){
        state = RH8;
        AddPacketCommandData(byte);
    }
    else if (state == RH8 && byte == '\r'){
        state = RH9;
    }
    else if (state == RH9 && byte == '\n'){
        state = RH10;
    }
    
    // receiving data length
    else if (state == RH10 && byte == 'L'){
        state = LEN1;
    }
    else if (state == LEN1 && byte == 'E'){
        state = LEN2;
    }
    else if (state == LEN2 && byte == 'N'){
        state = LEN3;
    }
    else if (state == LEN3 && byte == ':'){
        state = LEN4;
    }
    else if (state == LEN4 && (byte == ' ' || byte == '\t' )){
        state = state;
    }
    else if ((state == LEN4 || state == LEN5) && byte >= '0' && byte <= '9'){
        state = LEN5;
        AddPacketLenData(byte);
    }
    else if (state == LEN5 && byte == '\r'){
        state = LEN6;
    }
    else if (state == LEN6 && byte == '\n'){
        state = RecvData;
        SetDataLen();
        if (CurrDataRecvd == 0 && CurrDataLen == 0){
            state = Ready0;
            Packet *newPacket = CreatePacket();
            processPacket(newPacket);
        }
    }
    
    // receiving data
    else if (state == RecvData && CurrDataRecvd < CurrDataLen){
        AddPacketData(byte);
        if (CurrDataRecvd == CurrDataLen){
            state = Ready0;
            Packet *newPacket = CreatePacket();
            processPacket(newPacket);
        }
    }
    else if (state == RecvData && CurrDataRecvd == CurrDataLen){
        state = Ready0;
        Packet *newPacket = CreatePacket();
        processPacket(newPacket);
    }
    
    // White space is insignificant
    else if (byte == ' ' || byte == '\t'){
        state = state; // state doesn't change
    }
    
    // unexpected byte in the stream. application should stop processing
    else{
        result = FALSE;
    }
    
    return result;
}

PacketBuffer* CreatePacketBuffer(const char* command, unsigned char* data, unsigned int dataLength)
{
    PacketBuffer *packetBuffer = NULL;
    
    if (dataLength == 0 && data != NULL){
        // if dataLength is supplied but no data is supplied this is an error
        return NULL;
    }
    
    if (dataLength > 0 && data == NULL){
        // dataLength is greater than 0 but no buffer was supplied
        return NULL;
    }
    
    if (dataLength > MAX_PACKETDATA){
        // more data supplied than is supported by this protocol
        return NULL;
    }
    
    if (command == NULL){
        // command parameter must always be specified
        return NULL;
    }
    
    char tempSig[MAX_PACKETSIG + MAX_PACKETCOMMAND + 1];
    char tempDataLen[MAX_PACKETLEN];
    const char* delimiter = "[--\r\n";
    sprintf(tempSig, "PKT.V1 %s\r\n", command);
    sprintf(tempDataLen, "LEN: %d\r\n", dataLength);
    size_t delimiterLen = strlen(delimiter);
    size_t sigLen = strlen(tempSig);
    size_t dataLenLen = strlen(tempDataLen);
    size_t totalStreamLen =
        delimiterLen +
        sigLen +
        dataLenLen +
        dataLength;

    unsigned char* stream = (unsigned char*)malloc(totalStreamLen);
    memcpy(stream, delimiter, delimiterLen);
    memcpy(stream + delimiterLen, tempSig, sigLen);
    memcpy(stream + delimiterLen + sigLen, tempDataLen, dataLenLen);
    if (dataLength > 0){
        memcpy(stream + delimiterLen + sigLen + dataLenLen, data, dataLength);
    }

    packetBuffer = (PacketBuffer*)malloc(sizeof(PacketBuffer));
    if (packetBuffer != NULL){
        packetBuffer->Data = malloc(totalStreamLen);
        if (packetBuffer->Data != NULL){
            packetBuffer->Data = stream;
            packetBuffer->DataLength = (unsigned int)totalStreamLen;
        }
        else{
            packetBuffer->Data = NULL;
            packetBuffer->DataLength = 0;
        }
    }
    return packetBuffer;
}

void DeletePacketBuffer(PacketBuffer *packetBuffer)
{
    if (packetBuffer != NULL){
        if (packetBuffer->Data != NULL){
            free(packetBuffer->Data);
        }
        free(packetBuffer);
    }
}

