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

#include "ican_common.h"
#include "ican_common_packet.h"
#include "icansim.h"


//-----------------------------------------------------------------------------
// FUNCTION     SetIcanEvent()
// PURPOSE      create event sent to ROUTING_ICAN
// Parameter    Node* node
//                      pointer to node
//                    clocktype cInterval
//                      delay time to send event
//                    int nMsgType
//                      Event
//                    size_t infoSize
//                      size of info field
//                    void* pToInfo
//                      pointer to info field to append
//-----------------------------------------------------------------------------
void SetIcanEvent(Node* node, clocktype cInterval, int nMsgType, size_t infoSize, void* pToInfo)
{
    Message *msg = MESSAGE_Alloc(
                       node, NETWORK_LAYER, ROUTING_PROTOCOL_ICAN, nMsgType);

    if(infoSize > 0 && (pToInfo!=NULL))
    {
        //append neccessary information
        MESSAGE_InfoAlloc(
            node,
            msg,
            infoSize);
        memcpy(MESSAGE_ReturnInfo(msg), pToInfo, infoSize);
    }

    MESSAGE_Send(node, msg, cInterval);
}



//-----------------------------------------------------------------------------
// FUNCTION     SendIcanPacket()
// PURPOSE      send packet via ROUTING_ICAN
// Parameter    Node* node
//                      pointer to node
//                    clocktype cInterval
//                      delay time to send event
//              Message* pToInfo
//                    The packet that will be sent out (Note that this packet will be freed after calling this function)
//---------------------------------------------------------------------------
void SendIcanPacket(Node* node, clocktype cInterval, Message* packetToSend)
{   
    IcanHeader* ccnHdr = (IcanHeader*) MESSAGE_ReturnPacket(packetToSend);
        //duplicate packet and send events for stat purpose
    Message* pkt = MESSAGE_Duplicate(node, packetToSend);

    Message *msg = MESSAGE_Alloc(
                       node, NETWORK_LAYER, ROUTING_PROTOCOL_ICAN, MSG_ROUTING_ICAN_BROADCASTPACKET);
    MESSAGE_InfoAlloc(
        node,
        msg,
        sizeof(Message*));
    memcpy(MESSAGE_ReturnInfo(msg), (&pkt), sizeof(Message*));
    MESSAGE_Send(node, msg, cInterval);

    //send packet to network by broadcast
    BroadcastPacket(node, packetToSend, cInterval);
}

//-----------------------------------------------------------------------------
// FUNCTION     SetIcanEventAndReturnEvent()
// PURPOSE      create event sent to ROUTING_ICAN and return the event pointer
// Parameter    Node* node
//                      pointer to node
//                    clocktype cInterval
//                      delay time to send event
//                    int nMsgType
//                      Event
//                    size_t infoSize
//                      size of info field
//                    void* pToInfo
//                      pointer to info field to append
//-----------------------------------------------------------------------------
Message* SetIcanEventAndReturnEvent(Node* node, clocktype cInterval, int nMsgType, size_t infoSize, void* pToInfo)
{
    Message *msg = MESSAGE_Alloc(
                       node, NETWORK_LAYER, ROUTING_PROTOCOL_ICAN, nMsgType);

    if(infoSize > 0 && (pToInfo!=NULL))
    {
        //append neccessary information
        MESSAGE_InfoAlloc(
            node,
            msg,
            infoSize);
        memcpy(MESSAGE_ReturnInfo(msg), pToInfo, infoSize);
    }

    MESSAGE_Send(node, msg, cInterval);
    return msg;
}

//-----------------------------------------------------------------------------
// FUNCTION     IsInteger()
// PURPOSE      decides if the input string is an integer
// Parameter    std::string s
//                      input string
//-----------------------------------------------------------------------------
bool IsInteger(std::string s)
{
    for(int i=0; i<s.length(); ++i)
    {
        if(!isdigit(s[i]))
            return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
// FUNCTION     StringToInt()
// PURPOSE      convert the input string to an integer
// Parameter    std::string s
//                      input string
//-----------------------------------------------------------------------------
int StringToInt(std::string s)
{
    return atoi(s.c_str());
}

unsigned ConvertNonceToUnsigned(std::string s)
{
    unsigned result=0;
    for(int i=0; i<s.length(); ++i)
    {
        char c = s[i];
        int num=atoi(&c);
        result = result*10+num;
    }
    return result;
}


//-----------------------------------------------------------------------------
// FUNCTION     PrintTime()
// PURPOSE      Print simulation time
// PARAMETERS   Node *node
//                  Pointer to node.
//-----------------------------------------------------------------------------
void PrintTime(Node *node)
{
    char clockStr[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(getSimTime(node), clockStr);
    printf("node %d: at %s\n",node->nodeIndex+1, clockStr);
}

//-----------------------------------------------------------------------------
// FUNCTION     GetTimeString()
// PURPOSE      Print simulation time to a string
// PARAMETERS   Node *node
//                  Pointer to node.
//-----------------------------------------------------------------------------
std::string GetTimeString(Node *node)
{
    char clockStr[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(getSimTime(node), clockStr);
    std::string s(clockStr);    
    return s;
}

//-----------------------------------------------------------------------------
// FUNCTION    IntToString()
// PURPOSE      convert the input integer to string
// Parameter    int i
//                      input integer
//-----------------------------------------------------------------------------
std::string IntToString(int i)
{
    std::stringstream ss;//create a stringstream
    ss << i;//add number to the stream
    return ss.str();
}

//-----------------------------------------------------------------------------
// FUNCTION     GetPrefixFromName()
// PURPOSE      returns the name prefix to indicate app
// Parameter    std::string& szName
//                      the interest name
//-----------------------------------------------------------------------------
std::string GetPrefixFromName(std::string& szName)
{
    std::string szPrefix(szName);
    size_t delim_pos;
    delim_pos = szPrefix.find("/");
    if (delim_pos != std::string::npos)
    {
        szPrefix = szPrefix.substr(0, delim_pos);
    }
    return szPrefix;
}

bool IsFullDataObjectName(std::string& szName){
    std::string szPrefix(szName);
    size_t delim_pos;
    delim_pos = szPrefix.find("/");
    if (delim_pos == std::string::npos)
    {
        return true;
    }
    else return false;
}

//-----------------------------------------------------------------------------
// FUNCTION     GenerateRandomDelay
// PURPOSE       generate a random delay time
// Parameter    Node* node
//                      pointer to node
//                    clocktype maxBackoff
//                      the maximum delay time
//                    RandomSeed seed
//                      random seed
//-----------------------------------------------------------------------------
clocktype GenerateRandomDelay(Node* node, clocktype maxBackoff, RandomSeed seed)
{
    clocktype delay = (clocktype) (RANDOM_erand(seed) * maxBackoff);
    return delay;
}


//-----------------------------------------------------------------------------
// FUNCTION     BroadcastPacket()
// PURPOSE     Package ICAN packet as an IP broadcast packet and send to MAC layer
// Parameter    Node* node
//                      pointer to node
//                    Message* pkt
//                      ICAN packet
//                    clocktype delay
//                      delay before sending to MAC
//-----------------------------------------------------------------------------
void BroadcastPacket(Node* node, Message* pkt, clocktype delay)
{

    IcanData* pIcanDs =
        reinterpret_cast<IcanData*>(
            NetworkIpGetRoutingProtocol(node, ROUTING_PROTOCOL_ICAN, NETWORK_IPV4));

    Address broadcastAddress;
    broadcastAddress.networkType=NETWORK_IPV4;
    broadcastAddress.interfaceAddr.ipv4=ANY_DEST;

    Address srcAddr;
    srcAddr.networkType = broadcastAddress.networkType;
    srcAddr.interfaceAddr.ipv4= NetworkIpGetInterfaceAddress(node, 0);

    int TTL=1;

    NetworkIpSendRawMessageToMacLayerWithDelay(
        node,
        pkt,
        srcAddr.interfaceAddr.ipv4,
        broadcastAddress.interfaceAddr.ipv4,
        IPTOS_PREC_INTERNETCONTROL, //priority
        IPPROTO_ROUTING_ICAN,
        TTL,
        DEFAULT_INTERFACE,
        broadcastAddress.interfaceAddr.ipv4,
        delay);

}

void ICANReportError(std::string s, const char* file, int   lineno)
{
    char errorString[MAX_STRING_LENGTH];
    sprintf(errorString, s.c_str());
    ERROR_WriteError(ERROR_ERROR, NULL, errorString, file, lineno);
}

GeoCoordinate GetLocation(Node* node, unsigned nodeIndex){
    PartitionData* partition = node->partitionData;    
    Node* currNode = partition->firstNode;

    GeoCoordinate gc;
    while(currNode != NULL){
        if(currNode->nodeIndex - nodeIndex == 0){
            gc.x = currNode->mobilityData->current->position.common.c1;
            gc.y = currNode->mobilityData->current->position.common.c2;
            break;
        }
        currNode = currNode->nextNodeData;
    }

    return gc;
}

GeoCoordinate GetLocation(Node* node){

    GeoCoordinate gc;
    gc.x = node->mobilityData->current->position.common.c1;
    gc.y = node->mobilityData->current->position.common.c2;
    return gc;
}

std::vector<Node*> GetMyNeighborList(Node* node, double tx_range){

    std::vector<Node*> neighborList;
    double my_x = node->mobilityData->current->position.common.c1;
    double my_y = node->mobilityData->current->position.common.c2;

        
    PartitionData* partition = node->partitionData;    
    Node* currNode = partition->firstNode;

    GeoCoordinate gc;
    while(currNode != NULL){
        gc.x = currNode->mobilityData->current->position.common.c1;
        gc.y = currNode->mobilityData->current->position.common.c2;

        if(gc.GetDistance(my_x, my_y) <= tx_range){
            neighborList.push_back(currNode);
        }
        
        currNode = currNode->nextNodeData;
    }

    return neighborList;
}

std::list<unsigned> GetAllNodeIndex(Node* node){
    std::list<unsigned> nodeList;
        
    PartitionData* partition = node->partitionData;    
    Node* currNode = partition->firstNode;

    while(currNode != NULL){
        nodeList.push_back(currNode->nodeIndex);
      
        currNode = currNode->nextNodeData;
    }

    return nodeList;
}
