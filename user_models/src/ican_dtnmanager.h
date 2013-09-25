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

#ifndef ICAN_DTNMANAGER_H
#define ICAN_DTNMANAGER_H

#include "ican_common.h"
#include "ican_dtn_interestgenerator.h"
#include "ican_dtn_cachesummarygenerator.h"
#include "ican_dtn_interestmanager.h"
#include "fragmentation.h"
#include "ican_dtn_common.h"
#include "ican_dtn_objectqueue.h"
#include "fragmentationpacketutil.h"
#include "ican_dtn_requestmanager.h"
#include "ican_dtn_requestgenerator.h"
#include "ican_dtn_cachesummarystore.h"

#define DTNINTERESTMATCHINGDELAY 30*MILLI_SECOND 
#define BLOCKUNIT 1024 //in bytes

enum bufferedObjectType{bot_unknown, bot_fragmentation, bot_ncblock};

typedef struct ObjectBufferEntry{//convenience struct for buffered objects to ease the 3-way handshake work
    bufferedObjectType objType;
    Fragmentation_t fragmentMetadata;
    int numRetry;

    ObjectBufferEntry():objType(bot_unknown), numRetry(0)
    {}

    ObjectBufferEntry(Fragmentation_t frag_):objType(bot_fragmentation), fragmentMetadata(frag_), numRetry(0)
    {
        
    }

    ObjectBufferEntry(ObjectBufferEntry const&  o):objType(o.objType), fragmentMetadata(o.fragmentMetadata){
    }
    virtual ObjectBufferEntry& operator=(ObjectBufferEntry const& o) {
        objType = o.objType;
        fragmentMetadata = o.fragmentMetadata;
        return *this;
    }

    int getRetry(){
	return numRetry;
	}
    void incrementRetry(){
	numRetry++;
	}	
		
    virtual ~ObjectBufferEntry(){

    }

    void Print(){
        std::cout<<"========BUFFERENTRY========="<<std::endl
        <<"objType: "<<objType<<std::endl;
	if(objType == bot_fragmentation)		
	        fragmentMetadata.Print();
       std::cout<<std::endl;
    }
                
} ObjectBufferEntry;

typedef std::map<std::string, ObjectBufferEntry> HandshakeObjectBuffer;
typedef std::map<std::string, unsigned> HandshakeWaitingState; //keeps track of the name I'm waiting to receive <name, datasrc>
typedef std::map<std::string, size_t> FileSizeBuffer;

struct IcanDtnStat
{
    //packet type stat
    unsigned totalInterestOut;
    unsigned totalInterestBytesOut;
    unsigned totalInterestIn;
    unsigned totalInterestBytesIn; 
    unsigned totalRequestOut; 
    unsigned totalRequestBytesOut; 
    unsigned totalRequestIn;
    unsigned totalRequestBytesIn;
    unsigned totalCacheSummaryOut;
    unsigned totalCacheSummaryBytesOut;
    unsigned totalCacheSummaryIn;
    unsigned totalCacheSummaryBytesIn;
    unsigned totalRtsIn;
    unsigned totalRtsBytesIn;
    unsigned totalRtsOut;
    unsigned totalRtsBytesOut;
    unsigned totalCtsAcceptIn;
    unsigned totalCtsAcceptBytesIn;
    unsigned totalCtsAcceptOut;
    unsigned totalCtsAcceptBytesOut;
    unsigned totalCtsRejectIn;
    unsigned totalCtsRejectBytesIn;
    unsigned totalCtsRejectOut;
    unsigned totalCtsRejectBytesOut;
    unsigned totalAckIn;
    unsigned totalAckBytesIn;
    unsigned totalAckOut;
    unsigned totalAckBytesOut;
    unsigned totalFragIn;
    unsigned totalFragBytesIn;
   
};

class CIcanDtnManager{

public:
    CIcanDtnManager(Node* m_node, const NodeInput* nodeInput, DtnDataStore* dtnDataStore, int interfaceIndex);
    ~CIcanDtnManager();
    void PrintDebugInfo();
    bool EventHandler(Message* msg);
    void PacketHandler(Node* node, Message* msg);
    void PrintStat(Node* node,   NetworkType networkType);

    void SendRTS(std::string objectId, unsigned targetId, bool isNc);
    void SendACK(std::string objectId, bool isNc);
    void HandleRTS(DTNRTSPacket rtspkt);
    void HandleCTS(DTNCTSPacket ctspkt);
    void HandleACK(DTNACKPacket ackpkt); 
    
private:
    IcanDtnStat m_stat;
    
    RandomSeed m_seed;
    Node* m_node;
    RandomSeed seed;
    DtnDataStore* m_pDtnDataStore; //passed from routing_ican

    CDtnInterestGenerator* m_pInterestRegistry;
    CDtnCacheSummaryGenerator* m_pCacheSummaryGen;
    CDtnInterestManager* m_pInterestManager;
    CDtnRequestManager* m_pDataRequestManager;
    CDtnRequestGenerator* m_pDataRequestGenerator;

    CDtnCacheSummaryStore* m_pCacheSummaryStore;
        
    //add hooks here
    //e.g.
    FragmentationManager* m_fragmentation;

    clocktype m_InterestInterval;
    clocktype m_cacheSummaryInterval;	
    clocktype m_RequestInterval;
    clocktype m_nodeDescriptionJitter;
    clocktype m_nodeDescriptionStartingJitter;	
    clocktype m_rtsExpireTime;
    clocktype m_ctsExpireTime;
    clocktype m_dataSendExpireTime;
    
    CDtnObjectQueue* m_objectQueue;

    fragmentationpacketutil* m_fragmentPacketUtil;
    HandshakeObjectBuffer* m_pHandshakeObjBuffer;
    HandshakeWaitingState* m_pHandshakeDataWaitingState;
    HandshakeWaitingState* m_pHandshakeAckWaitingState;

    double m_txrange;

    BOOL m_enableNc;		

    FileSizeBuffer* m_pFileSizeBuffer;

    int m_maxRtsRetry;

    CIcanDtnManager(){}
    void RequestMatching(unsigned nodeIndex);
    void RetrieveNextFragment(std::string fragName, unsigned targetNodeIndex);
   void SendNeighborInterestPacket(unsigned neighborNodeIndex);
    void RequestMatchingByObject(unsigned nodeIndex, std::string objName);
    void NotifySubscriber(std::string objName, size_t objSize);
    size_t GetFileSizeByObjectName(std::string fullName);
	

};


#endif

