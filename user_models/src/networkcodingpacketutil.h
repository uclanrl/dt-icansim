/*
 * networkcodingpacketutil.h
 *
 *  Created on: Aug 9, 2013
 *      Author: josh
 */

#ifndef NETWORKCODINGPACKETUTIL_H_
#define NETWORKCODINGPACKETUTIL_H_

#include "networkcoding.h"
#include "ican_dtn_common.h"
#include "ican_common_packet.h"

//maintain mapping for networkcoding block to networkcodingpacket sequence numbers received
typedef std::set<int> networkcodingpacketsequence_t;
typedef std::map<std::string,networkcodingpacketsequence_t > networkcodingpackets_t;

struct NetworkCodingPacketUtilStat
{
    unsigned totalCodingPacketSent;
    unsigned totalCodingPacketProcessed;
    unsigned totalCodingPacketReceived;
    unsigned totalBlockReconstructed;
    unsigned totalCodingBytesSent;
};

class networkcodingpacketutil {
public:
	networkcodingpacketutil(size_t _payloadSize,size_t blockSize,Node* sourceNode);
	virtual ~networkcodingpacketutil();
    bool EventHandler(Node* node,Message* msg);
    void sendPackets(std::list<Message*> packets,Node* sourceNode);
	std::list<Message*> createNetworkCodingPackets(
			NetworkCoding_t networkcoding,Node* sourceNode);
	void handleSendNetworkcoding(
			NetworkCoding_t networkcoding,Node* sourceNode);

	void receiveNetworkCodingPacket(NetworkCodingPacket* networkcodingPacket);
	void notifyThreeWayHandleShake(std::string parentObjectName,int fragmentId);
        void Printstat(Node* node, NetworkType networkType);
    
private:
	Node* sourceNode;
	size_t blockSize;
	size_t numberPackets;
	size_t payloadSize;
	networkcodingpackets_t networkcodingpackets;

	NetworkCodingPacketUtilStat m_stat;
};

#endif /* NETWORKCODINGPACKETUTIL_H_ */
