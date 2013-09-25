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

#include "ican_dtn_objectqueue.h"


bool CDtnObjectQueue::EventHandler(Message* msg){
    switch (MESSAGE_GetEvent(msg))
    {
    case MSG_ROUTING_ICAN_DTN_OBJECTQUEUE_SENDNEXT:{
        TakeNextObject();

        SetIcanEvent(m_node, m_sendRate, MSG_ROUTING_ICAN_DTN_OBJECTQUEUE_SENDNEXT, 0, NULL);
        
        MESSAGE_Free(m_node, msg);       
        return true;
        
    }
   
    default:
    {
        return false;
    }
    
    }
    return false;
}

CDtnObjectQueue::CDtnObjectQueue(Node* node, const NodeInput* nodeInput, int interfaceIndex):m_node(node), wasEmpty(true)
{
    m_objectQueue = new ObjectQueue;

    BOOL retVal;

    IO_ReadTime(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "DTN-DATA-SENDING-RATE",
        &retVal,
        &m_sendRate);
    if(retVal == FALSE)
    {
	ReportError("DTN-DATA-SENDING-RATE");
    }
    
    SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTN_OBJECTQUEUE_SENDNEXT, 0, NULL);
}

CDtnObjectQueue::~CDtnObjectQueue()
{
    delete m_objectQueue;
    m_objectQueue = NULL;
}

void CDtnObjectQueue::InsertInterestDataObjectToQueue(unsigned nodeIndex){
    ObjectMetadata metadata(interest, nodeIndex);
    
    //check duplicate as the interests may be inserted periodically          
    for (ObjectQueue::iterator it = m_objectQueue->begin(); it != m_objectQueue->end(); ++it) {
        if(metadata==*it){
            return;
        }
    }
    m_objectQueue->push_back(metadata); 

    if(m_objectQueue->size() == 1 && wasEmpty){
        //meaning the queue was empty, should send the packet immediately
        wasEmpty = false;
        TakeNextObject();        
    }
}

void CDtnObjectQueue::InsertFragmentToQueue(Fragmentation_t frag){ 

    ObjectMetadata metadata(fragment, frag);
    
    //check duplicate as the interests may be inserted periodically          
    for (ObjectQueue::iterator it = m_objectQueue->begin(); it != m_objectQueue->end(); ++it) {
        if(metadata==*it){
            std::cout<<"found duplicate"<<std::endl;
            return;
        }
    }
    m_objectQueue->push_back(metadata); 

    if(m_objectQueue->size() == 1 && wasEmpty){
        //meaning the queue was empty, should send the packet immediately
        wasEmpty = false;
        TakeNextObject();
    }
}

void CDtnObjectQueue::TakeNextObject(){
    if(m_objectQueue->size() == 0){
        wasEmpty = true;
        return;
    }
    

    ObjectMetadata metadata = m_objectQueue->front();

    //interpret metadata
    if(metadata.IsEmpty()) ReportError("DtnObjectQueue: empty metadata in queue");

    switch(metadata.objectType){
        case interest:
        {
            SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTNMANAGER_SENDNEIGHBORINTEREST, sizeof(metadata.interestNodeIndex), &(metadata.interestNodeIndex));
            break;
        }

        case fragment:
        {
            //3-way handshake
            SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTNMANAGER_FRAGMENTHANDSHAKESTART, sizeof(*metadata.fragmentMetadata), metadata.fragmentMetadata);
            break;
        }
        default:{
            ReportError("unknown object type in dtnobjectqueue");
        }        
    }
    
    m_objectQueue->pop_front();    
}


void CDtnObjectQueue::PrintDebugInfo()
{
    for (ObjectQueue::iterator it = m_objectQueue->begin(); it != m_objectQueue->end(); ++it) {
        it->Print();
    }

    std::cout<<std::endl;
}

