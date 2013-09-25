/*
* Copyright (c) 2013,  Network Research Lab, University of California, Los Angeles
* Coded by Yu-Ting Yu [yutingyu@cs.ucla.edu]
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
* in the documentation and/or other materials provided with the distribution.
* Neither the name of the University of California, Los Angeles nor the names of its contributors 
* may be used to endorse or promote products derived from this software without specific prior written permission.
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ICAN_COMMON_H
#define ICAN_COMMON_H

#define dbgprintf(msg) printf("LINE:%d, %s, %s\n",__LINE__, __FUNCTION__, __FILE__);printf(msg);printf("\n")
#define ReportError(str)  ICANReportError(str, __FILE__, __LINE__)

#define MAXPACKETSTRINGLEN 8
#define PARENTOBJECTNAMELEN 16
#define HANDSHAKENAMELEN 20


#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "clock.h"
#include "api.h"
#include <list>
#include <map>
#include <set>
#include <utility>

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include "network_ip.h"
#include "partition.h"

#include "ican_common_packet.h"
#include "ican_common_eventinfo.h"

#include "ican_geocoordinate.h"

#define DTNSUBSCRIBERID "DTNSUB"
#define DTNPUBLISHERID "DTNPUB"

enum faceId {app, wifi};

struct ResponseTimeInfo
{
    clocktype totalResponseTime;
    clocktype minResponseTime;
    clocktype maxResponseTime;
    int numResponse;
    ResponseTimeInfo():totalResponseTime(0), numResponse(0), minResponseTime(999999999999999999), maxResponseTime(0){}
    ResponseTimeInfo(ResponseTimeInfo const & origin): totalResponseTime(origin.totalResponseTime), numResponse(origin.numResponse), 
        minResponseTime(origin.minResponseTime), maxResponseTime(origin.maxResponseTime){}
};

typedef std::map<unsigned, bloom_filter> BloomFilterStore; //key: node ID, value: bloom_filter

typedef std::map<std::string, size_t> DtnDataStore; //<data object name, (parent) object size>

void SetIcanEvent(Node * node, clocktype cInterval, int nMsgType, size_t infoSize, void * pToInfo);
void SendIcanPacket(Node* node, clocktype cInterval, Message* pToInfo);
Message* SetIcanEventAndReturnEvent(Node* node, clocktype cInterval, int nMsgType, size_t infoSize, void* pToInfo);
void PrintTime(Node* node);
std::string GetTimeString(Node *node);
std::string IntToString(int i);

int StringToInt(std::string s);
bool IsInteger(std::string s);
std::string GetPrefixFromName(std::string& szName);
bool IsFullDataObjectName(std::string& szName);
clocktype GenerateRandomDelay(Node* node, clocktype maxBackoff, RandomSeed seed);

void BroadcastPacket(Node* node, Message* pkt, clocktype delay);

void ICANReportError(std::string s, const char* file, int   lineno);

GeoCoordinate GetLocation(Node* node, unsigned nodeIndex);
GeoCoordinate GetLocation(Node* node);
std::vector<Node*> GetMyNeighborList(Node* node, double tx_range);
std::list<unsigned> GetAllNodeIndex(Node* node);

//-----------------------------------------------------------------------------
// FUNCTION     GeneratePacket()
// PURPOSE     This template is for generating real interest packet or data packet (non-coded)
// Parameter    Node* node
//                      pointer to node
//                    T* packet
//                      the packet header information
//                    packetType pType
//                      packet type
//                    int nPktSize
//                      payload size
//-----------------------------------------------------------------------------
template <class T>
Message* GeneratePacket(Node* node, T* packet, packetType pType, int nPktSize)
{
    //generate interest
    NetworkRoutingProtocolType protocolType = ROUTING_PROTOCOL_ICAN;

    Message *newMsg = MESSAGE_Alloc(
                          node,
                          MAC_LAYER,
                          protocolType,
                          MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(
        node,
        newMsg,
        sizeof(T),
        TRACE_ICAN);

    T* pPkt = (T*) MESSAGE_ReturnPacket(newMsg);
    if(!pPkt)
    {
        char errorString[MAX_STRING_LENGTH];
        sprintf(errorString, "Allocate failure\n");
        ERROR_ReportError(errorString);
    }

    memcpy(pPkt, packet, sizeof(T));

#ifdef DEBUG_ICAN
    dbgprintf("Routing layer generating packet\t");
#endif

    //add packet type header
    MESSAGE_AddHeader(node, newMsg, sizeof(IcanHeader), TRACE_ICAN);
    IcanHeader *hdr = (IcanHeader*)newMsg->packet;
    hdr->pktType = pType;

    //add virtual payload
    MESSAGE_AddVirtualPayload(node, newMsg, nPktSize);

    return newMsg;
}


template <class T>
Message* GeneratePacketWithRawData(Node* node, T* packet, packetType pType, int nPktSize, int rawDataSize, const void* rawData)
{
    //generate interest
    NetworkRoutingProtocolType protocolType = ROUTING_PROTOCOL_ICAN;

    Message *newMsg = MESSAGE_Alloc(
                          node,
                          MAC_LAYER,
                          protocolType,
                          MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(
        node,
        newMsg,
        sizeof(T) + rawDataSize,
        TRACE_ICAN);

    T* pPkt = (T*) MESSAGE_ReturnPacket(newMsg);
    if(!pPkt)
    {
        char errorString[MAX_STRING_LENGTH];
        sprintf(errorString, "Allocate failure\n");
        ERROR_ReportError(errorString);
    }

    memcpy(pPkt, packet, sizeof(T));

    void* pRawData = (void*) (pPkt+1);
    memcpy(pRawData, rawData, rawDataSize);

    MESSAGE_AddHeader(node, newMsg, sizeof(IcanHeader), TRACE_ICAN);
    IcanHeader *hdr = (IcanHeader*)newMsg->packet;
    hdr->pktType = pType;

    //add virtual payload
    MESSAGE_AddVirtualPayload(node, newMsg, nPktSize);

    return newMsg;
}

#endif
