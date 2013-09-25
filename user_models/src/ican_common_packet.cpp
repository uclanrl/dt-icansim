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


std::string pktTypeString[] = {"tyDtnRequest", "tyDtnCacheSummary", "tyFragment", "tyDtnRts", "tyDtnCts", "tyDtnAck", "tyDtnInterest"};

bool IsDtnPacket(packetType pType){
    if(pType == tyDtnRequest || pType == tyDtnCacheSummary || pType == tyFragment || pType ==tyDtnRts || pType ==tyDtnCts|| pType == tyDtnAck || pType ==tyDtnInterest) 
        return true;
    else return false;
}
void PrintSendLog(Node* node, Message* const  msg){
    
    IcanHeader* ccnHeader = (IcanHeader*) MESSAGE_ReturnPacket(msg);            
    packetType ptype = ccnHeader->pktType;

    std::cout<<GetTimeString(node)<<" "<<node->nodeIndex+1<<", ";

   if(ptype==tyDtnRts){
        DTNRTSPacket* pkt = (DTNRTSPacket*) (ccnHeader+1);
        std::cout<<pkt->srcName + 1
            <<", 0, SEND, "
            <<pktTypeString[ptype]
            <<", "
            <<pkt->GetName();
    }

    else if(ptype==tyDtnCts){
        DTNCTSPacket* pkt = (DTNCTSPacket*) (ccnHeader+1);
        std::cout<<pkt->srcName + 1
            <<", 0, SEND, "
            <<pktTypeString[ptype]
            <<", "
            <<pkt->GetName();
    }

    else if(ptype==tyDtnAck){
        DTNACKPacket* pkt = (DTNACKPacket*) (ccnHeader+1);
        std::cout<<pkt->srcName + 1
            <<", 0, SEND, "
            <<pktTypeString[ptype]
            <<", "
            <<pkt->GetName();
    }

      
    else if(ptype==tyFragment){
        FragmentationPacket* pkt = (FragmentationPacket*) (ccnHeader+1);
        std::cout<<pkt->srcName + 1
            <<", 0, SEND, "
            <<pktTypeString[ptype]
            <<", "
            <<pkt->GetPacketName();
    }
    else if(IsDtnPacket(ptype)){
        DTNPacket* pkt = (DTNPacket*) (ccnHeader+1);
        std::cout<< pkt->srcName + 1
            <<", 0, SEND, "
            <<pktTypeString[ptype];
    }

    else{
        std::cout<<" unknown"
                        <<", 0, SEND, "
                        <<pktTypeString[ptype];                        
    }
    
    std::cout<<", "<<MESSAGE_ReturnPacketSize(msg)<< " #"<<std::endl;
}

void PrintRecvLog(Node* node, Message* const  msg){    
    IcanHeader* ccnHeader = (IcanHeader*) MESSAGE_ReturnPacket(msg);            
    packetType ptype = ccnHeader->pktType;

    std::cout<<GetTimeString(node)<<" "<<node->nodeIndex+1<<", ";

   if(ptype==tyDtnRts){
        DTNRTSPacket* pkt = (DTNRTSPacket*) (ccnHeader+1);
        std::cout<<pkt->srcName + 1
            <<", 0, RECV, "
            <<pktTypeString[ptype]
            <<", "
            <<pkt->GetName();
    }

    else if(ptype==tyDtnCts){
        DTNCTSPacket* pkt = (DTNCTSPacket*) (ccnHeader+1);
        std::cout<<pkt->srcName + 1
            <<", 0, RECV, "
            <<pktTypeString[ptype]
            <<", "
            <<pkt->GetName();
    }

    else if(ptype==tyDtnAck){
        DTNACKPacket* pkt = (DTNACKPacket*) (ccnHeader+1);
        std::cout<<pkt->srcName + 1
            <<", 0, RECV, "
            <<pktTypeString[ptype]
            <<", "
            <<pkt->GetName();
    }

   else if(ptype==tyFragment){
        FragmentationPacket* pkt = (FragmentationPacket*) (ccnHeader+1);
        std::cout<<pkt->srcName + 1
            <<", 0, RECV, "
            <<pktTypeString[ptype]
            <<", "
            <<pkt->GetPacketName();
    }
    else if(IsDtnPacket(ptype)){
        DTNPacket* pkt = (DTNPacket*) (ccnHeader+1);
        std::cout<< pkt->srcName + 1
            <<", 0, RECV, "
            <<pktTypeString[ptype];
    }
    else{
        std::cout<<" unknown"
                        <<", 0, RECV, "
                        <<pktTypeString[ptype];                        
    }
    
    std::cout<<", "<<MESSAGE_ReturnPacketSize(msg)<< " #"<<std::endl;
}
