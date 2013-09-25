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

#include "ican_dtn_requestmanager.h"

CDtnRequestManager::CDtnRequestManager(Node* node, int interfaceIndex, const NodeInput* nodeInput):m_node(node)
{
    //initialize seed
    RANDOM_SetSeed(m_seed,node->globalSeed,node->nodeId,ROUTING_PROTOCOL_ICAN, interfaceIndex);    

    m_pRequestStore = new BloomFilterStore;
}

CDtnRequestManager::~CDtnRequestManager()
{
    delete m_pRequestStore;
    m_pRequestStore = NULL;
}

bool CDtnRequestManager::EventHandler(Message* msg){
    switch (MESSAGE_GetEvent(msg))
    {
    default:
    {
        bool freedByOthers = false;        
        return freedByOthers;
    }    
    }
    return false;
}

void CDtnRequestManager::InsertRequest(unsigned nodeName, bloom_filter requestBf)
{
    (*m_pRequestStore)[nodeName] = requestBf;

    std::cout<<m_node->nodeIndex+1<<" insert request from "<<nodeName+1<<std::endl;
}

bool CDtnRequestManager::HasRequest(unsigned nodeName){
    if(m_pRequestStore->find(nodeName) != m_pRequestStore->end()){
        return true;
    }
    return false;
}

bloom_filter CDtnRequestManager::GetRequest(unsigned nodeName){
    return (*m_pRequestStore)[nodeName];
}

void CDtnRequestManager::PrintDebugInfo()
{
 
}
