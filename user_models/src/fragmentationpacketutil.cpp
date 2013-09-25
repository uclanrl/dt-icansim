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
#include "fragmentationpacketutil.h"

fragmentationpacketutil::fragmentationpacketutil(size_t _payloadSize, size_t _blockSize,
		Node* _sourceNode):payloadSize(_payloadSize),blockSize(_blockSize),sourceNode(_sourceNode) {

        memset(&m_stat, 0, sizeof(m_stat));
        
	this->numberPackets = _blockSize / _payloadSize;
}

fragmentationpacketutil::~fragmentationpacketutil() {

}


void fragmentationpacketutil::Printstat(Node* node, NetworkType networkType) {

    char statbuf[MAX_STRING_LENGTH];
    sprintf(statbuf, "Block Reconstructed = %d", m_stat.totalBlockReconstructed);
    IO_PrintStat(node, "FragUtil", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "Fragment packets sent = %d", m_stat.totalFragPacketSent);
    IO_PrintStat(node, "FragUtil", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "Fragment packets received = %d", m_stat.totalFragPacketReceived);
    IO_PrintStat(node, "FragUtil", "ICAN", ANY_DEST, 0, statbuf);
    
    sprintf(statbuf, "Fragment packets processed = %d", m_stat.totalFragPacketProcessed);
    IO_PrintStat(node, "FragUtil", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "FragmentPacket bytes sent = %d", m_stat.totalFragmentBytesSent);
    IO_PrintStat(node, "FragUtil", "ICAN", ANY_DEST, 0, statbuf);
}

bool fragmentationpacketutil::EventHandler(Node* sourceNode,Message* message) {
    switch (MESSAGE_GetEvent(message))
    {
    case MSG_ROUTING_ICAN_DTN_FRAGMENTPACKETUTIL_SENDFRAGMENT:{
    	Fragmentation_t* fragment = (Fragmentation_t*) MESSAGE_ReturnInfo(message);
        //fragment->Print();

    	handleSendFragment(*fragment,sourceNode);

        MESSAGE_Free(sourceNode, message);
        return true;
    }
    case MSG_ROUTING_ICAN_DTN_FRAGMENTPACKETUTIL_RECEIVEFRAGMENT: {
    	FragmentationPacket* fragmentationpacket = (FragmentationPacket*)MESSAGE_ReturnInfo(message);
    	//fragmentationpacket->PrintPacket();

    	receiveFragment(fragmentationpacket);

    	MESSAGE_Free(sourceNode, message);
    	return true;
    }
    }


	return false;
}

void fragmentationpacketutil::handleSendFragment(
		Fragmentation_t fragmentation,Node* sourceNode) {
	std::list<Message*>  packets = createFragmentationPackets(fragmentation,sourceNode);
	sendPackets(packets,sourceNode);
}

void fragmentationpacketutil::sendPackets(std::list<Message*> packets,
		Node* sourceNode) {
	std::list<Message*>::const_iterator iter;
        int delay = 0; //TODO: add jitter?
	for(iter=packets.begin(); iter!=packets.end(); iter++)	{
		Message* packet = (*iter);

                m_stat.totalFragPacketSent++;
                this->m_stat.totalFragmentBytesSent += MESSAGE_ReturnPacketSize(packet);
                //send packet
                SendIcanPacket(sourceNode, delay * MILLI_SECOND, packet);
	}
}

std::list<Message*> fragmentationpacketutil::createFragmentationPackets(
		Fragmentation_t fragmentation,Node* sourceNode) {
	std::list<Message*> packets;

	unsigned sourceNodeId = sourceNode->nodeIndex;

	int payLoad = payloadSize;
	int requiredNumberOfPackets = (this->blockSize + payLoad - 1)/payLoad;

	//sequenceid generate all packets based on block size
	for(int sequenceNumber=1;sequenceNumber<=requiredNumberOfPackets;sequenceNumber++) {
		FragmentationPacket fragmentationPacket(sourceNodeId,
				fragmentation.parentObjectName,fragmentation.fragmentid,sequenceNumber,
				fragmentation.targetNodeId,fragmentation.sizeBytesParentObject);

		packetType pType = tyFragment;

		Message* qualnetFragmentationPacket = GeneratePacket(sourceNode,&fragmentationPacket,pType,payLoad);

		packets.push_back(qualnetFragmentationPacket);
	}

	return packets;
}

void fragmentationpacketutil::receiveFragment(FragmentationPacket* fragmentationPacket) {
        m_stat.totalFragPacketReceived++;
        
	std::string parentObjectName = fragmentationPacket->parentObjectName;
	int fragmentid = fragmentationPacket->fragmentid;
	std::string stringFragmentId =IntToString(fragmentid);
	std::string key = parentObjectName+"/"+stringFragmentId;

	//check if we already started receiving fragments for this object
	if( this->fragmentationpackets.find(key) == this->fragmentationpackets.end() ) {
		//create set and insert
		fragmentatonpacketsequence_t fragmentatonpacketsequence;
		this->fragmentationpackets[key] = fragmentatonpacketsequence;
	}

	//update set
	fragmentatonpacketsequence_t fragmentatonpacketsequence = this->fragmentationpackets[key];
        int sequencenumber = fragmentationPacket->sequenceNumber;
        if(fragmentatonpacketsequence.find(sequencenumber)!=fragmentatonpacketsequence.end()){
            //means I already have this fragment
            //skip processing
            return;
        }
        m_stat.totalFragPacketProcessed++;
        
	fragmentatonpacketsequence.insert(sequencenumber); 
	this->fragmentationpackets[key] = fragmentatonpacketsequence;

	if( this->numberPackets == fragmentatonpacketsequence.size()) {                    
		size_t sizeParentObject = fragmentationPacket->sizeParentObject;
		std::string parentObjName(fragmentationPacket->parentObjectName);
		Fragmentation_t fragmentation(parentObjName, fragmentationPacket->fragmentid,
				sizeParentObject,fragmentationPacket->targetNodeId);

                m_stat.totalBlockReconstructed++;
		SetIcanEvent(this->sourceNode, 0, MSG_ROUTING_ICAN_DTNMANAGER_FRAGMENTRECONSTRUCTED, sizeof(fragmentation), &fragmentation);
	}
}

void fragmentationpacketutil::notifyThreeWayHandleShake(std::string parentObjectName,int fragmentId) {
	//FIXME
	//configurable option to expire fragment packet cache when duplicating handshake
}
