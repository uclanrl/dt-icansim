/*
 * networkcodingpacketutil.cpp
 *
 *  Created on: Aug 9, 2013
 *      Author: josh
 */

#include "networkcodingpacketutil.h"

networkcodingpacketutil::networkcodingpacketutil(size_t _payloadSize, size_t _blockSize,
		Node* _sourceNode):payloadSize(_payloadSize),blockSize(_blockSize),sourceNode(_sourceNode) {

	memset(&m_stat, 0, sizeof(m_stat));

	this->numberPackets = _blockSize / _payloadSize;
}

networkcodingpacketutil::~networkcodingpacketutil() {

}

void networkcodingpacketutil::Printstat(Node* node, NetworkType networkType) {

    char statbuf[MAX_STRING_LENGTH];
    sprintf(statbuf, "Block Reconstructed = %d", m_stat.totalBlockReconstructed);
    IO_PrintStat(node, "CodingUtil", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "Coding packets sent = %d", m_stat.totalCodingPacketSent);
    IO_PrintStat(node, "CodingUtil", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "Coding packets received = %d", m_stat.totalCodingPacketReceived);
    IO_PrintStat(node, "CodingUtil", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "Coding packets processed = %d", m_stat.totalCodingPacketProcessed);
    IO_PrintStat(node, "CodingUtil", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "NetworkCodingPacket bytes sent = %d", m_stat.totalCodingBytesSent);
    IO_PrintStat(node, "CodingUtil", "ICAN", ANY_DEST, 0, statbuf);


}

bool networkcodingpacketutil::EventHandler(Node* node,Message* message) {
    switch (MESSAGE_GetEvent(message))
    {
    case MSG_ROUTING_ICAN_DTN_NETWORKCODINGPACKETUTIL_SENDNCBLOCK:{
    	NetworkCoding_t* networkcoding = (NetworkCoding_t*) MESSAGE_ReturnInfo(message);

    	handleSendNetworkcoding(*networkcoding,sourceNode);

        MESSAGE_Free(sourceNode, message);
        return true;
    }
    case MSG_ROUTING_ICAN_DTN_NETWORKCODINGPACKETUTIL_RECEIVEBLOCK: {
    	NetworkCodingPacket* networkcodingpacket = (NetworkCodingPacket*)MESSAGE_ReturnInfo(message);
    	//networkcodingpacket->PrintPacket();

    	receiveNetworkCodingPacket(networkcodingpacket);

    	MESSAGE_Free(sourceNode, message);
    	return true;
    }
    }


	return false;
}

void networkcodingpacketutil::sendPackets(std::list<Message*> packets,Node* sourceNode) {
	std::list<Message*>::const_iterator iter;
	for(iter=packets.begin(); iter!=packets.end(); iter++)	{
		Message* packet = (*iter);
        //send packet
		int delay = 0;

        m_stat.totalCodingPacketSent++;
        this->m_stat.totalCodingBytesSent+=MESSAGE_ReturnPacketSize(packet);
        SendIcanPacket(sourceNode, delay, packet);
	}
}

std::list<Message*> networkcodingpacketutil::createNetworkCodingPackets(
		NetworkCoding_t networkcoding,Node* sourceNode) {
	std::list<Message*> packets;

	unsigned sourceNodeId = sourceNode->nodeIndex;

	int requiredNumberOfPackets = (this->blockSize + this->payloadSize - 1)/this->payloadSize;

	//cout<<"requiredNumberOfPackets: "<<requiredNumberOfPackets<<std::endl;
	//sequenceid generate all packets based on block size
	for(int sequenceNumber=1;sequenceNumber<=requiredNumberOfPackets;sequenceNumber++) {
		NetworkCodingPacket networkCodingPacket(sourceNodeId,
				networkcoding.parentObjectName,networkcoding.networkcodingblockid,sequenceNumber,
				networkcoding.targetNodeId,networkcoding.parentObjectFileSize,networkcoding.isMixed);

		packetType pType = tyCoding;

		Message* qualnetFragmentationPacket = GeneratePacket(sourceNode,&networkCodingPacket,pType,this->payloadSize);

		//packets.push_back(qualnetFragmentationPacket);
		int delay = 0;

        this->m_stat.totalCodingPacketSent++;
        this->m_stat.totalCodingBytesSent+=MESSAGE_ReturnPacketSize(qualnetFragmentationPacket);
        SendIcanPacket(sourceNode, delay, qualnetFragmentationPacket);
	}

	return packets;
}

void networkcodingpacketutil::handleSendNetworkcoding(
		NetworkCoding_t networkcoding,Node* sourceNode) {

	createNetworkCodingPackets(networkcoding,sourceNode);
	//sendPackets(packets,sourceNode);
}

void networkcodingpacketutil::receiveNetworkCodingPacket(NetworkCodingPacket* networkcodingPacket) {
	this->m_stat.totalCodingPacketReceived++;

	std::string parentObjectName = networkcodingPacket->parentObjectName;
	int networkcodingblockid = networkcodingPacket->networkcodingblockid;
	std::string stringNetworkCodingBlockId =convertInt(networkcodingblockid);
	std::string key = parentObjectName+":"+stringNetworkCodingBlockId;
//	printf("key=%s\n",key.c_str());
	//check if we already started receiving fragments for this object
	if( this->networkcodingpackets.find(key) == this->networkcodingpackets.end() ) {
		//create set and insert
		networkcodingpacketsequence_t networkcodingpacketsequence;
		networkcodingpacketsequence.clear();
		this->networkcodingpackets[key] = networkcodingpacketsequence;
	}

	//update set
	networkcodingpacketsequence_t _networkcodingpacketsequence = this->networkcodingpackets[key];
    int sequencenumber = networkcodingPacket->sequenceNumber;
    if(_networkcodingpacketsequence.find(sequencenumber)!=_networkcodingpacketsequence.end()){
        //means I already have this coded packet
        //skip processing
        return;
    }
    m_stat.totalCodingPacketProcessed++;
    _networkcodingpacketsequence.insert(sequencenumber);
	this->networkcodingpackets[key] = _networkcodingpacketsequence;
//	printf("numberPackets=%d networkcodingpacketsequencesize=%d\n",this->numberPackets,_networkcodingpacketsequence.size());
	if( this->numberPackets == _networkcodingpacketsequence.size()) {

		size_t sizeParentObject = networkcodingPacket->sizeParentObject;
		NetworkCoding_t networkcoding(networkcodingPacket->parentObjectName,
				sizeParentObject,networkcodingPacket->targetNodeId,
				networkcodingPacket->networkcodingblockid,networkcodingPacket->isMixed);

		printf("block name=%s sourcenodeid=%d\n",networkcodingPacket->GetBlockName().c_str(), networkcodingPacket->srcName+1);

		m_stat.totalBlockReconstructed++;
		SetIcanEvent(this->sourceNode, 0, MSG_ROUTING_ICAN_DTNMANAGER_NETWORKCODINGRECONSTRUCTED, sizeof(NetworkCoding_t), &networkcoding);
	}
}

void networkcodingpacketutil::notifyThreeWayHandleShake(std::string parentObjectName,int fragmentId) {

}
