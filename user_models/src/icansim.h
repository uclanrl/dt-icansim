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


#ifndef ICANSIM_H
#define ICANSIM_H
#include <list>
#include <map>
#include <utility>

#include "ican_common.h"
#include "ican_dtnmanager.h"
#include "ican_appmanager.h"


//----------------------------Stats definitions--------------------------------


struct IcanStat
{
    // All nodes
    unsigned totalPacketsIn;
   unsigned totalPacketsOut;
    unsigned totalBytesOut;    

    //packet type stat
    unsigned totalDtnPacketOut;    
    unsigned totalDtnBytesOut;
};

//----------------------------Data structure definitions and data types------------------------


typedef struct IcanDataStruct
{

    RandomSeed rsAppSeed;
    IcanStat m_stat;
 
    int m_nPktSize;
    DtnDataStore* m_dtnDataObjectStore;

    CIcanDtnManager* m_dtnManager;
    CIcanAppManager* m_appManager;

    char m_expName[MAX_STRING_LENGTH];

    RandomSeed lossRateSeed;

    IcanDataStruct()
    {
        memset(&m_stat, 0, sizeof(m_stat));
    }
} IcanData;

//---------------------Prototypes for interface functions in Routing_nc3n.cpp------------------

void IcanHandleProtocolPacket(
    Node *node,
    Message *msg,
    Address srcAddr,
    Address destAddr,
    int ttl,
    int interfaceIndex);

void IcanHandleProtocolEvent(Node *node, Message *msg);

void IcanInit(
    Node *node,
    IcanData**ncdsPtr,
    const NodeInput *nodeInput,
    int interfaceIndex);

void IcanFinalize(Node *node, NetworkType networkType);

//------------------------Common functions---------------------------
void IcanForwarding(Node * node, Message * pkt, bool fIsFromApp, NodeAddress lastHop);
#endif
