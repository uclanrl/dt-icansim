/*
* Copyright (c) 2013,  Network Research Lab, University of California, Los Angeles
* Coded by Joshua Joy [jjoy@cs.ucla.edu]
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
#ifndef FRAGMENTATIONPACKETUTIL_H_
#define FRAGMENTATIONPACKETUTIL_H_

#include "ican_common.h"
#include "fragmentation.h"
#include <list>
#include <set>

//maintain mapping for fragmentid to fragmentpacket sequence numbers received
typedef std::set<int> fragmentatonpacketsequence_t;
typedef std::map<std::string,fragmentatonpacketsequence_t > fragmentationpackets_t;

struct FragmentationPacketUtilStat
{
    unsigned totalFragPacketSent;
    unsigned totalFragPacketProcessed;
    unsigned totalFragPacketReceived;
    unsigned totalBlockReconstructed;
    unsigned totalFragmentBytesSent;
};

class fragmentationpacketutil {
public:
	fragmentationpacketutil(size_t payloadSize, size_t blockSize,Node* sourceNode);
	virtual ~fragmentationpacketutil();
    bool EventHandler(Node* node,Message* msg);
    void sendPackets(std::list<Message*> packets,Node* sourceNode);
	std::list<Message*> createFragmentationPackets(
			Fragmentation_t fragmentation,Node* sourceNode);
	void handleSendFragment(
			Fragmentation_t fragmentation,Node* sourceNode);

	void receiveFragment(FragmentationPacket* fragmentationPacket);
	void notifyThreeWayHandleShake(std::string parentObjectName,int fragmentId);
        void Printstat(Node* node, NetworkType networkType);

private:
	Node* sourceNode;
	size_t blockSize;
	size_t numberPackets;
        size_t payloadSize;
	fragmentationpackets_t fragmentationpackets;
        FragmentationPacketUtilStat m_stat;
};

#endif /* FRAGMENTATIONPACKETUTIL_H_ */
