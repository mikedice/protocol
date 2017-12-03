//
//  main.c
//  protocol
//
//  Created by Michael Dice on 12/1/17.
//  Copyright © 2017 Michael Dice. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

void ProcessPacket(Packet* packet){
    printf("processing packet\n");
    printf("Sig: %s\n", packet->Sig);
    printf("Command: %s\n", packet->Command);
    printf("DataLen: %d\n", packet->DataLength);
    printf("\n");
    DeletePacket(packet); // done with the packet
}

void TestStream(char* stream){
    size_t len = strlen(stream);
    for (size_t i = 0; i < len; i++){
        ProcessStreamByte(stream[i], &ProcessPacket);
    }
}
int main(int argc, const char * argv[]) {
    ResetParser();
    char *test1 = "[--\r\n"
    "PKT.V1 HELLO\r\n"
    "LEN: 6\r\n"
    "abc123";
    TestStream(test1);
    
    ResetParser();
    char *test2 = "[--\r\n"
    "PKT.V1 HELLOREPLY\r\n"
    "LEN: 11\r\n"
    "hello reply";
    TestStream(test2);

    ResetParser();
    char *test3 = "[--\r\n"
    "PKT.V1 PING\r\n"
    "LEN: 0\r\n";
    TestStream(test3);
    
    PacketBuffer* packetBuffer = CreatePacketBuffer("NODATA", NULL, 0);
    ResetParser();
    TestStream((char*)packetBuffer->Data);
    DeletePacketBuffer(packetBuffer);

    const char* buffer = "you are a boob";
    packetBuffer = CreatePacketBuffer("MYPACKET", (unsigned char*)buffer, (unsigned int)strlen(buffer));
    ResetParser();
    TestStream((char*)packetBuffer->Data);
    DeletePacketBuffer(packetBuffer);
    
    return 0;
}

