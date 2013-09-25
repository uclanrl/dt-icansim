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

#include <cfloat>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <limits.h>

#include "api.h"
#include "network_ip.h"
#include "partition.h"
#include "icansim.h"

//----------------------------Qualnet Interface----------------------------

//-----------------------------------------------------------------------------
// FUNCTION     IcanHandleProtocolPacket()
// PURPOSE      Handle packets sent to ICAN protocol
//-----------------------------------------------------------------------------
void IcanHandleProtocolPacket(
    Node *node,
    Message *msg,
    Address srcAddr_in,
    Address destAddr_in,
    int ttl,
    int interfaceIndex
)
{

    IcanData* pIcanDs =
        reinterpret_cast<IcanData*>(
            NetworkIpGetRoutingProtocol(node, ROUTING_PROTOCOL_ICAN, NETWORK_IPV4));

    NodeAddress srcAddr = GetIPv4Address(srcAddr_in);
    NodeAddress destAddr = GetIPv4Address(destAddr_in);

    if(NetworkIpIsMyIP(node, srcAddr))
    {
        MESSAGE_Free (node, msg);
        return; //if this is a self-initiated packets, do not react.
    }

   
    IcanForwarding(node, msg, FALSE, srcAddr);
    return;
}

//-----------------------------------------------------------------------------
// FUNCTION     IcanHandleProtocolEvent()
// PURPOSE      ICAN event dispatcher
// PARAMETERS   Node *node
//                  Pointer to node.
//              Message *msg
//                  Event message
//-----------------------------------------------------------------------------
void IcanHandleProtocolEvent (
    Node *node,
    Message *msg)
{

    NodeAddress srcAddr, destAddr;
    TosType priority;
    unsigned char IpProto;
    unsigned int ttl;

    IcanData* pIcanDs =
        reinterpret_cast<IcanData*>(
            NetworkIpGetRoutingProtocol(node, ROUTING_PROTOCOL_ICAN, NETWORK_IPV4));

    switch (MESSAGE_GetEvent(msg))
    {
    case MSG_ROUTING_ICAN_BROADCASTPACKET: //This is the case when a packet is broadcast
    {
        Message** pkt = (Message**) MESSAGE_ReturnInfo(msg);

        //update stat
        pIcanDs->m_stat.totalPacketsOut++; 
        pIcanDs->m_stat.totalBytesOut+=MESSAGE_ReturnPacketSize((*pkt)); 
        //update stats
        IcanHeader* icanHeader = (IcanHeader*) MESSAGE_ReturnPacket((*pkt));

        PrintSendLog(node, *pkt);
        
        if(IsDtnPacket(icanHeader->pktType)){
                pIcanDs->m_stat.totalDtnPacketOut++;
                pIcanDs->m_stat.totalDtnBytesOut+=MESSAGE_ReturnPacketSize((*pkt));                     
        }
        MESSAGE_Free(node, *pkt);
        MESSAGE_Free(node, msg);
        break;
    }

    default:
    {
        bool freedByOthers = false;
        freedByOthers = pIcanDs->m_dtnManager->EventHandler(msg)
       			  || pIcanDs->m_appManager->EventHandler(msg);
        if(!freedByOthers)
        {
			std::cout<<msg->eventType<<std::endl;
            ReportError("Unknown routing_ICAN event");
        }
    }
    
    return;
    }
}

//-----------------------------------------------------------------------------
// FUNCTION     IcanInit()
// PURPOSE      ICAN initialization, read inputs, initializations
//-----------------------------------------------------------------------------
void IcanInit(
    Node *node,
    IcanData **ncdsPtr,
    const NodeInput *nodeInput,
    int interfaceIndex)
{
    char buf[MAX_STRING_LENGTH];
    BOOL retVal;
 
    IcanData* pIcanDs = new IcanData();
    (*ncdsPtr) = pIcanDs;

    srand(node->globalSeed); 
    
    //general sim settings    
    IO_ReadString(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "EXPERIMENT-NAME",
        &retVal,
        pIcanDs->m_expName);
    if (retVal == FALSE)
    {
        ReportError("EXPERIMENT-NAME is missing");
    }

  
    IO_ReadInt(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "ICAN-PACKETSIZE",
        &retVal,
        &pIcanDs->m_nPktSize);
    if (retVal == FALSE || pIcanDs->m_nPktSize < 0)
    {
        ReportError("Wrong ICAN-PACKETSIZE configuration format!");
    }    

   //initialize random seed
    RANDOM_SetSeed(pIcanDs->lossRateSeed,node->globalSeed,node->nodeId,ROUTING_PROTOCOL_ICAN, interfaceIndex);
    RANDOM_SetSeed(pIcanDs->rsAppSeed,node->globalSeed,node->nodeId,ROUTING_PROTOCOL_ICAN, interfaceIndex);

    std::string sExpName(pIcanDs->m_expName);
    //initialize data store
   pIcanDs->m_dtnDataObjectStore = new DtnDataStore;

    pIcanDs->m_appManager = new CIcanAppManager(node, nodeInput, sExpName,  pIcanDs->m_dtnDataObjectStore, pIcanDs->m_nPktSize);
    
   pIcanDs->m_dtnManager = new CIcanDtnManager(node, nodeInput, pIcanDs->m_dtnDataObjectStore, interfaceIndex);    

}


//-----------------------------------------------------------------------------
// FUNCTION     IcanFinalize()
// PURPOSE      ICAN protocol finalization, print stats
//-----------------------------------------------------------------------------
void IcanFinalize(
    Node *node,
    NetworkType networkType)
{
    IcanData* pIcanDs =
        reinterpret_cast<IcanData*>(
            NetworkIpGetRoutingProtocol(node, ROUTING_PROTOCOL_ICAN, NETWORK_IPV4));

    int offset;
    char buf[MAX_STRING_LENGTH];

    // Print statistics

    //packet type stat

    sprintf(buf, "Total DTN packets sent = %d", pIcanDs->m_stat.totalDtnPacketOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN bytes sent = %d", pIcanDs->m_stat.totalDtnBytesOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);


    //All nodes
    sprintf(buf, "Total packets in = %u", pIcanDs->m_stat.totalPacketsIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total packets out = %u", pIcanDs->m_stat.totalPacketsOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total packets bytes out = %u", pIcanDs->m_stat.totalBytesOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

   pIcanDs->m_dtnManager->PrintStat(node, networkType);

    pIcanDs->m_appManager->PrintStat(node, networkType);
    
   delete pIcanDs->m_dtnDataObjectStore;

    delete pIcanDs->m_dtnManager;
   delete pIcanDs->m_appManager;
        
    delete pIcanDs;    
}


//-----------------------------Common functions---------------------------------


//-----------------------------Forwarding functions---------------------------

//-----------------------------------------------------------------------------
// FUNCTION     IcanForwarding()
// PURPOSE     This is the start point of the main forwarding process
// Parameter    Node* node
//                      pointer to node
//                    Message* pkt
//                      packet
//                    bool fIsFromApp
//                      indicates if this packet is received from App layer
//-----------------------------------------------------------------------------
void IcanForwarding(Node* node, Message* pkt, bool fIsFromApp, NodeAddress lastHop)
{
    IcanData* pIcanDs =
        reinterpret_cast<IcanData*>(
            NetworkIpGetRoutingProtocol(node, ROUTING_PROTOCOL_ICAN, NETWORK_IPV4));

    //packet is received from network
    if(!fIsFromApp)
    {
			
       PrintRecvLog(node, pkt);
                
        //update stat
        pIcanDs->m_stat.totalPacketsIn++;
   }


    //Check ICAN header, dispatch packet
    IcanHeader* icanHdr = (IcanHeader*) MESSAGE_ReturnPacket(pkt);

   //DTN hook
    if(IsDtnPacket(icanHdr->pktType)){
        pIcanDs->m_dtnManager->PacketHandler(node, pkt);
    }      

   else{
        MESSAGE_Free(node, pkt);
        ReportError("Unknown packet type");
        return;
    }        
}


