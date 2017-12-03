//
//  parser.h
//  protocol
//
//  Created by Mike Dice on 12/2/17.
//  Copyright © 2017 Mike Dice. All rights reserved.
//

#ifndef parser_h
#define parser_h

#ifdef __cplusplus
extern "C"
{
#endif

/********* Spec ********************

 packet with data of length 5
 [--<newline>
PKT.V1 HELLO<newline>
LEN: 5<newline>
ADSFG
 
Packet with no data
[--<newline>
PKT.V1 HELLO<newline>
LEN: 0<newline>

***********************************/

#define MAX_PACKETDATA 1024
#define MAX_PACKETSIG 32
#define MAX_PACKETCOMMAND 64
#define MAX_PACKETLEN 32

#ifndef BOOL_DEFINED
#define BOOL unsigned int
#endif
    
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
    
// Data structure used to hold command and data
typedef struct {
    unsigned char Sig[MAX_PACKETSIG-1];
    unsigned char Command[MAX_PACKETCOMMAND-1];
    unsigned char Data[MAX_PACKETDATA];
    unsigned int DataLength;
} Packet;
    
typedef struct{
    unsigned char *Data;
    unsigned int DataLength;
} PacketBuffer;

// Application callback function that gets called once a packet becomes
// ready for the application to process
typedef void (*ProcessPacketFunc)(Packet* packet);

// Application must call ResetParser to enable the parser to start consuming
// next available packet. Typically call at beginning of application and
// immediately after the application has processed a packet
void ResetParser(void);
    
// Bytes are passed to the protocol processor one at a time by the application.
// THe application is responsible for obtaining those byte from a transport stream
// that it maintains, such as serial port, tcp channel or bluetooth stream.
// If function returns FALSE then application should call ResetParser because
// a byte not understood by the protocol was received.
BOOL ProcessStreamByte(unsigned char byte, ProcessPacketFunc processPacket);
    
// When application is done with a packet call this function to remove the Packet
// data structure from memory
void DeletePacket(Packet *packet);
    
// To send a new packet, start by calling CreatePacket to allocate a new packet.
// This will create a packet properly formatted per protocol definition. If no data
// is to be passed then pass NULL for the data parameter and 0 for dataLength
PacketBuffer* CreatePacketBuffer(const char* command, unsigned char* data, unsigned int dataLength);

// Application must delete packet buffers after they are returned
void DeletePacketBuffer(PacketBuffer *packetBuffer);



#ifdef __cplusplus
}
#endif

#endif /* parser_h */
