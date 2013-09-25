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

#include "ican_dtn_interestmanager.h"

CDtnInterestManager::CDtnInterestManager(Node* node, int interfaceIndex, const NodeInput* nodeInput):m_node(node)
{
    //initialize seed
    RANDOM_SetSeed(m_seed,node->globalSeed,node->nodeId,ROUTING_PROTOCOL_ICAN, interfaceIndex);    

    m_pInterestStore = new BloomFilterStore;

    BOOL retVal;

    IO_ReadTime(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "INTEREST-FORWARD-INTERVAL",
        &retVal,
        &m_interestForwardInterval);
    if(retVal == FALSE)
    {
	ReportError("INTEREST-FORWARD-INTERVAL");
    }
}

CDtnInterestManager::~CDtnInterestManager()
{
    delete m_pInterestStore;
}

bool CDtnInterestManager::EventHandler(Message* msg){
    switch (MESSAGE_GetEvent(msg))
    {
    case MSG_ROUTING_ICAN_DTN_INTERESTMANAGER_INITIATENEIGHBORINTEREST:{
        unsigned* nodeIndex = (unsigned*) MESSAGE_ReturnInfo(msg);
        
        SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTNMANAGER_QUEUEINTEREST, sizeof(*nodeIndex), nodeIndex);

        //schedule the next sent
        SetIcanEvent(m_node, m_interestForwardInterval, MSG_ROUTING_ICAN_DTN_INTERESTMANAGER_INITIATENEIGHBORINTEREST, sizeof(*nodeIndex), nodeIndex);                
        //TODO: do we want to stop the interest forwarding at some point or not?!
        
        MESSAGE_Free(m_node, msg);
        return true;        
    }

    default:
    {
        bool freedByOthers = false;        
        return freedByOthers;
    }    
    }
    return false;
}

void CDtnInterestManager::InsertInterest(unsigned nodeName, bloom_filter interestBf)
{
    bool scheduleInterest = false;
    if (m_pInterestStore->find(nodeName) == m_pInterestStore->end()){
        scheduleInterest = true;
    }

    (*m_pInterestStore)[nodeName] = interestBf;


    //if the interest has not been seen before,   schedule periodic timer for interest forwarding
    if(scheduleInterest){
        //TODO: we may want to wait, overhear data transmissions or 3-way handshake, detect congestion level etc
        SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTN_INTERESTMANAGER_INITIATENEIGHBORINTEREST, sizeof(nodeName), &nodeName);
    }
}

bloom_filter CDtnInterestManager::GetInterest(unsigned nodeName){
    return (*m_pInterestStore)[nodeName];
}

void CDtnInterestManager::PrintDebugInfo()
{
    /*
    PrintTime(m_node);
    set<std::string>::iterator it;
    std::cout<<"node "<<m_node->nodeIndex+1<<" subscribes to ";
    for(it = m_pInterestRegistry->begin(); it != m_pInterestRegistry->end(); it++)
        std::cout<<*it<<" ";
    std::cout<<std::endl;
    */
}
