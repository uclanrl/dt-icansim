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

/*
   @file ican_common_packet.h
   @brief defined ICAN packet formats
	  	enum packetType: defined all packet types that can be used
	  	    tyInterest--InterestPacket
	  	    tyData--DataPacket
	  	    tyGeoFloodInterest--GeoFloodInterestPacket
	  	    tyActiveInterest--ActiveFloodInterestPacket
	  	    tyGeoData--GeoRouteDataPacket
	  	    tyGeoInterest--GeoRouteInterestPacket
	  	    tyBfrGeoInterest--BfrInterestPacket
                    tyDtnRequest, 
                    tyDtnCacheSummary, 
                    tyFragment, 
                    tyDtnRts, 
                    tyDtnCts, 
                    tyDtnAck, 
                    tyDtnInterest

	  	    tyBeacon--BeaconPkt, see ican_beacon.h
	  	    tyNodeAdv--NodeAdvPacket, see ican_bfr_common.h
	  	    tyPartitionAdv--PartitionAdvPacket, see ican_bfr_common.h

        IcanHeader: all packets must carry ICAN Header, which carries the packet type indicator
        ICNPacket: all ICN Packets must inherit this type
        InterestPacket: basic interest
        DataPacket: basic data packet without payload

        GeoRouteDataPacket: data packet with destination and lasthop geo coordinates
        GeoFloodInterestPacket: interest packets carring last hop location
        BfrInterestPacket: interest packet of BFR
        GeoRouteInterestPacket: georouting interest
        ActiveFloodInterestPacket: active flooding interest

    @Author: Yu-Ting Yu
    @Date: 2013/6/15
*/


#ifndef ICAN_COMMON_PACKET_H
#define ICAN_COMMON_PACKET_H

#include "ican_geocoordinate.h"
#include "bloom_filter.hpp"
#include "ncutil.h"

enum packetType {tyInterest, tyData, tyNodeAdv, tyPartitionAdv, tyBfrGeoInterest, tyGeoFloodInterest, tyBeacon, tyActiveInterest, tyGeoData, tyGeoInterest, tyAck, tyDtnRequest, tyDtnCacheSummary, tyFragment, tyCoding,tyDtnRts, tyDtnCts, tyDtnAck, tyDtnInterest};

extern std::string pktTypeString[];


typedef clocktype freshnessIndexUnit ;

bool IsDtnPacket(packetType pType);
bool IsIcnPacket(packetType pType);
bool IsIcnInterest(packetType pType);
bool IsIcnData(packetType pType);
bool IsAck(packetType pType);
void PrintSendLog(Node* node, Message* const  msg);
void PrintRecvLog(Node* node, Message* const  msg);

typedef struct IcanHeader
{
    packetType pktType;
    virtual ~IcanHeader() {}
    IcanHeader() {}
    IcanHeader(IcanHeader const & o)
        :pktType(o.pktType) {}
} IcanHeader;

struct Packet
{
    virtual Packet& operator = (Packet const & o)
    {
        return *this;
    };
    virtual ~Packet() {}
    virtual void PrintPacket() = 0;
};

struct DTNPacket:public Packet
{

    unsigned srcName;  //note: get source name by node->nodeIndex

    DTNPacket():srcName(999999)
    {
    }
    DTNPacket(DTNPacket const & o)
    {
        srcName = o.srcName;
    }
    virtual DTNPacket& operator = (DTNPacket const & o)
    {
        Packet::operator = (o);
        srcName = o.srcName;
        return *this;
    }
    virtual ~DTNPacket() {}

    virtual void PrintPacket()
    {
        std::cout
                 <<"src node: "<<srcName +1<<std::endl;
    }

    virtual bool Initialized()
    {
        if(srcName -999999 == 0) return false;
        else return true;
    }
};

struct DTNRTSPacket: public DTNPacket{
    bool isNc;
    unsigned destName;
    char objectName[HANDSHAKENAMELEN+1];
    
    virtual ~DTNRTSPacket() {}

    DTNRTSPacket(unsigned _srcName, unsigned _destName, std::string _objectName, bool _isNC) : destName(_destName), isNc(_isNC) {
	this->srcName = _srcName;
        memset(this->objectName, 0, sizeof(char)*HANDSHAKENAMELEN+1);
        strncpy(this->objectName, _objectName.c_str(), sizeof(this->objectName));
    }
    
    DTNRTSPacket(DTNRTSPacket const & o):DTNPacket(o), destName(o.destName), isNc(o.isNc) {
        memset(this->objectName, 0, sizeof(char)*HANDSHAKENAMELEN+1);
        strncpy(this->objectName, o.objectName, sizeof(this->objectName));
    }

    virtual DTNRTSPacket& operator=(DTNRTSPacket const& o) {
	DTNPacket::operator = (o);
        this->isNc = o.isNc;
	this->destName = o.destName;
        memset(this->objectName, 0, sizeof(char)*HANDSHAKENAMELEN+1);
        strncpy(this->objectName, o.objectName, sizeof(this->objectName));
	return *this;
    }
    virtual void PrintPacket()
    {        
        std::string s(objectName);
        std::cout<<"isNC: "<<isNc<<" "
                    <<"Object Name: "<<s<<" "
                    <<"destName: "<<destName<<" "
        	    <<"srcName: "<<srcName<<std::endl;
    }

    std::string GetName()
    {
        std::string packetname(this->objectName);
        return packetname;
    }			
};

struct DTNCTSPacket: public DTNPacket{
    bool isNc;
    unsigned destName;
    char objectName[HANDSHAKENAMELEN+1];
    bool isAccept;
    bool isHaveBlock; //1 means i have this block already, 0 means other (say neighbors are sending this one..)
    bool isHaveObject; //1 means i have the full object
    bool isRejectNcMixing; //1 means non-innovative
    
    virtual ~DTNCTSPacket() {}

    DTNCTSPacket(unsigned _srcName, unsigned _destName, std::string _objectName, bool _isNC, bool _isAccept, bool _isHaveBlock, bool _isHaveObject, bool _isRejectNcMixing) : destName(_destName), isNc(_isNC), isAccept(_isAccept), isHaveBlock(_isHaveBlock), isHaveObject(_isHaveObject), isRejectNcMixing(_isRejectNcMixing) {
	this->srcName = _srcName;
        memset(this->objectName, 0, sizeof(char)*HANDSHAKENAMELEN+1);
        strncpy(this->objectName, _objectName.c_str(), sizeof(this->objectName));
    }
    
    DTNCTSPacket(DTNCTSPacket const & o):DTNPacket(o), destName(o.destName), isNc(o.isNc), isAccept(o.isAccept), isHaveBlock(o.isHaveBlock) , isHaveObject(o.isHaveObject), isRejectNcMixing(o.isRejectNcMixing){
        memset(this->objectName, 0, sizeof(char)*HANDSHAKENAMELEN+1);
        strncpy(this->objectName, o.objectName, sizeof(this->objectName));
    }

    virtual DTNCTSPacket& operator=(DTNCTSPacket const& o) {
	DTNPacket::operator = (o);
        this->isNc = o.isNc;
        this->isAccept = o.isAccept;
	this->destName = o.destName;
        this->isHaveBlock = o.isHaveBlock;
	this->isHaveObject = o.isHaveObject;
	this->isRejectNcMixing = o.isRejectNcMixing;
        memset(this->objectName, 0, sizeof(char)*HANDSHAKENAMELEN+1);
        strncpy(this->objectName, o.objectName, sizeof(this->objectName));
	return *this;
    }
    virtual void PrintPacket()
    {        
        std::string s(objectName);
        std::cout<<"isNC: "<<isNc<<" "
                    <<"Object Name: "<<s<<" "
                    <<"destName: "<<destName<<" "
        	    <<"srcName: "<<srcName<<" "
        	    <<"Accept? "<<isAccept<<" "
        	    <<"Have block?: "<<isHaveBlock<<" "
        	    <<"Have object?: "<<isHaveObject<<" "
		    <<"NC mixing reject?: "<<isRejectNcMixing<<" "
        	    <<std::endl;        
    }


    std::string GetName()
    {
        std::string packetname(this->objectName);
        return packetname;
    }		
};

struct DTNACKPacket: public DTNPacket{
    bool isNc;
    char objectName[HANDSHAKENAMELEN+1];
    bool hasFullObject;		
    
    virtual ~DTNACKPacket() {}

    DTNACKPacket(unsigned _srcName, std::string _objectName, bool _isNC, bool _hasFullObject) : isNc(_isNC), hasFullObject(_hasFullObject) {
	this->srcName = _srcName;
        memset(this->objectName, 0, sizeof(char)*HANDSHAKENAMELEN+1);
        strncpy(this->objectName, _objectName.c_str(), sizeof(this->objectName));
    }
    
    DTNACKPacket(DTNACKPacket const & o):DTNPacket(o), isNc(o.isNc), hasFullObject(o.hasFullObject) {
        memset(this->objectName, 0, sizeof(char)*HANDSHAKENAMELEN+1);
        strncpy(this->objectName, o.objectName, sizeof(this->objectName));
    }

    virtual DTNACKPacket& operator=(DTNACKPacket const& o) {
	DTNPacket::operator = (o);
        this->isNc = o.isNc;
	this->hasFullObject = o.hasFullObject;
        memset(this->objectName, 0, sizeof(char)*HANDSHAKENAMELEN+1);
        strncpy(this->objectName, o.objectName, sizeof(this->objectName));
	return *this;
    }
    virtual void PrintPacket()
    {        
        std::string s(objectName);
        std::cout<<"isNC: "<<isNc<<" "
                    <<"Object Name: "<<s<<" "
        	    <<"srcName: "<<srcName<<" "
        	    <<"hasFullObject: "<<hasFullObject<<" "
                    <<std::endl;;
    }

    std::string GetName()
    {
        std::string packetname(this->objectName);
        return packetname;
    }	
};


struct NetworkCodingPacket: public DTNPacket {
	char parentObjectName[PARENTOBJECTNAMELEN+1];
	int networkcodingblockid;
	size_t sizeParentObject;
	int sequenceNumber;
	unsigned targetNodeId;
	bool isMixed;

	virtual ~NetworkCodingPacket() {

	}


	NetworkCodingPacket() {
		memset(this->parentObjectName, 0, sizeof(char)*(PARENTOBJECTNAMELEN+1));
		this->networkcodingblockid = -1;
		this->sizeParentObject = -1;
		this->sequenceNumber = -1;
		this->targetNodeId = -1;
		this->isMixed = false;
	}

	NetworkCodingPacket(unsigned _srcName,
			std::string _parentObjectName, int _networkcodingblockid,int _sequenceNumber,
			unsigned _targetNodeId,size_t _sizeParentObject,bool _isMixed) : targetNodeId(_targetNodeId) {
		this->srcName = _srcName;
        memset(this->parentObjectName, 0, sizeof(char)*PARENTOBJECTNAMELEN+1);
        strncpy(this->parentObjectName, _parentObjectName.c_str(), sizeof(this->parentObjectName));
		this->networkcodingblockid = _networkcodingblockid;
		this->sequenceNumber = _sequenceNumber;
		this->sizeParentObject = _sizeParentObject;
		this->isMixed = _isMixed;
	}
	NetworkCodingPacket(NetworkCodingPacket const & o):DTNPacket(o) {
        memset(this->parentObjectName, 0, sizeof(char)*(PARENTOBJECTNAMELEN+1));
        strncpy(this->parentObjectName, o.parentObjectName, sizeof(this->parentObjectName));
		this->networkcodingblockid = o.networkcodingblockid;
		this->sequenceNumber = o.sequenceNumber;
		this->targetNodeId = o.targetNodeId;
		this->sizeParentObject = o.sizeParentObject;
		this->isMixed = o.isMixed;
	}
	virtual NetworkCodingPacket& operator=(NetworkCodingPacket const& o) {
		DTNPacket::operator = (o);
        memset(this->parentObjectName, 0, sizeof(char)*(PARENTOBJECTNAMELEN+1));
        strncpy(this->parentObjectName, o.parentObjectName, sizeof(this->parentObjectName));
		this->networkcodingblockid = o.networkcodingblockid;
		this->sequenceNumber = o.sequenceNumber;
		this->targetNodeId = o.targetNodeId;
		this->sizeParentObject = o.sizeParentObject;
		this->isMixed = o.isMixed;
		return *this;
	}
    virtual void PrintPacket()
    {
        std::cout<<"parentObjectName: "<<parentObjectName
        		<<"sizeParentObject: "<<sizeParentObject
        		<<"networkcodingblockid: "<<networkcodingblockid
        		<<"sequencenumber: "<<sequenceNumber
        		<<"targetnodeid:"<<targetNodeId
        		<<"isMixed:"<<isMixed<<std::endl;
    }

    std::string GetBlockName()
    {
    	std::string separatorName("/");
    	std::string stringBlockId = convertInt(networkcodingblockid);

        std::string packetname = (this->parentObjectName)
        		+separatorName+stringBlockId;
        return packetname;
    }

    std::string GetName()
    {
    	std::string separatorName("/");
    	std::string stringBlockId = convertInt(networkcodingblockid);
    	std::string stringSequenceNumber = convertInt(sequenceNumber);
        std::string packetname = (this->parentObjectName)
        		+separatorName+stringBlockId+separatorName+stringSequenceNumber;
        return packetname;
    }

};

struct FragmentationPacket: public DTNPacket {
	char parentObjectName[PARENTOBJECTNAMELEN+1];
	int fragmentid;
	int sequenceNumber;
	unsigned targetNodeId;
	size_t sizeParentObject;

	virtual ~FragmentationPacket() {}

	FragmentationPacket(unsigned _srcName,
			std::string _parentObjectName, int _fragmentid,int _sequenceNumber,
			unsigned _targetNodeId,size_t _sizeParentObject) : targetNodeId(_targetNodeId) {
		this->srcName = _srcName;
        memset(this->parentObjectName, 0, sizeof(char)*PARENTOBJECTNAMELEN+1);
        strncpy(this->parentObjectName, _parentObjectName.c_str(), sizeof(this->parentObjectName));
		this->fragmentid = _fragmentid;
		this->sequenceNumber = _sequenceNumber;
		this->sizeParentObject = _sizeParentObject;
	}
	FragmentationPacket(FragmentationPacket const & o):DTNPacket(o) {
        memset(this->parentObjectName, 0, sizeof(char)*PARENTOBJECTNAMELEN+1);
        strncpy(this->parentObjectName, o.parentObjectName, sizeof(this->parentObjectName));
		this->fragmentid = o.fragmentid;
		this->sequenceNumber = o.sequenceNumber;
		this->targetNodeId = o.targetNodeId;
		this->sizeParentObject = o.sizeParentObject;
	}
	virtual FragmentationPacket& operator=(FragmentationPacket const& o) {
		DTNPacket::operator = (o);
        memset(this->parentObjectName, 0, sizeof(char)*PARENTOBJECTNAMELEN+1);
        strncpy(this->parentObjectName, o.parentObjectName, sizeof(this->parentObjectName));
		this->fragmentid = o.fragmentid;
		this->sequenceNumber = o.sequenceNumber;
		this->targetNodeId = o.targetNodeId;
		this->sizeParentObject = o.sizeParentObject;
		return *this;
	}
    virtual void PrintPacket()
    {
        std::cout<<"parentObjectName: "<<parentObjectName<<std::endl;
        std::cout<<"fragmentid: "<<fragmentid
        		<<"sequencenumber: "<<sequenceNumber
        		<<"targetnodeid:"<<targetNodeId
        		<<"sizeParentObject:"<<sizeParentObject<<std::endl;
    }
    std::string GetPacketName()
    {
    	std::string stringFragmentId = convertInt(fragmentid);
    	std::string stringSequenceNumber = convertInt(sequenceNumber);
    	std::string separatorName("/");
        std::string packetname = (this->parentObjectName)
        		+separatorName+stringFragmentId+separatorName+stringSequenceNumber;
        return packetname;
    }

    std::string GetFragmentName()
    {
    	std::string stringFragmentId = convertInt(fragmentid);
    	std::string separatorName("/");
        std::string packetname = (this->parentObjectName)
        		+separatorName+stringFragmentId;
        return packetname;
    }

};

struct AckPacket:public Packet
{
    char szNonce [MAXPACKETSTRINGLEN];
    AckPacket(std::string s)
    {
        memset(szNonce, 0, sizeof(szNonce));
        strncpy(szNonce, s.c_str(), sizeof(szNonce));
    }
    AckPacket(AckPacket const & o)
    {
        memset(szNonce, 0, sizeof(szNonce));
        strncpy(szNonce, o.szNonce, sizeof(szNonce));
    }
    virtual AckPacket& operator = (AckPacket const & o)
    {
        Packet::operator = (o);
        memset(szNonce, 0, sizeof(szNonce));
        strncpy(szNonce, o.szNonce, sizeof(szNonce));
        return *this;
    }
    virtual ~AckPacket() {}
    std::string GetNonce()
    {
        std::string s(szNonce, strlen(szNonce));
        return s;
    }
    void SetNonce(std::string s)
    {
        strncpy(szNonce, s.c_str(), sizeof(szNonce));
    }

    virtual void PrintPacket()
    {
        std::string nonce(szNonce, strlen(szNonce));
        std::cout<<"nonce: "<<nonce<<std::endl;
    }
};

struct ICNPacket:public Packet
{
    char szNonce [MAXPACKETSTRINGLEN];
    unsigned lastHopName;
    unsigned srcName;

    ICNPacket():lastHopName(999999), srcName(999999) 
    {
        memset(szNonce, 0, sizeof(szNonce));
    }
    ICNPacket(ICNPacket const & o)
    {
        memset(szNonce, 0, sizeof(szNonce));
        strncpy(szNonce, o.szNonce, sizeof(szNonce));
        lastHopName = o.lastHopName;
        srcName = o.srcName;
    }
    virtual ICNPacket& operator = (ICNPacket const & o)
    {
        Packet::operator = (o);
        memset(szNonce, 0, sizeof(szNonce));
        strncpy(szNonce, o.szNonce, sizeof(szNonce));
        lastHopName = o.lastHopName;
        srcName = o.srcName;
        return *this;
    }
    virtual ~ICNPacket() {}
    std::string GetNonce()
    {
        std::string s(szNonce, strlen(szNonce));
        return s;
    }
    void SetNonce(std::string s)
    {
        strncpy(szNonce, s.c_str(), sizeof(szNonce));
    }

    virtual void PrintPacket()
    {
        std::string nonce(szNonce, strlen(szNonce));
        std::cout<<"nonce: "<<nonce<<std::endl
                 <<"last Hop: "<<lastHopName+1<<std::endl
                 <<"src: "<<srcName +1<<std::endl;
    }

    virtual bool Initialized()
    {
        if(lastHopName -999999 ==0) return false;
        if(srcName -999999 == 0) return false;
        else return true;
    }
};

typedef struct DtnRequestPacket:public DTNPacket
{
    bloom_filter requestBf; //NOTE: must carry the bittable as rawdata when sending DTN Interest	

    DtnRequestPacket():DTNPacket()
    {        
    }
    DtnRequestPacket(DtnRequestPacket const & o):DTNPacket(o)
    {      
        requestBf= o.requestBf;
    }
    DtnRequestPacket& operator = (DtnRequestPacket const & o)
    {
        DTNPacket::operator = (o);
        requestBf = o.requestBf;
        
        return *this;
    }

    virtual void PrintPacket()
    {
        std::cout<<"===============DTN Request Packet============="<<std::endl;
        DTNPacket::PrintPacket();
        //std::cout  <<"id: "<<id<<std::endl;
    }
    virtual ~DtnRequestPacket() {}
} DtnRequestPacket;

typedef struct DtnInterestPacket:public DTNPacket{
    unsigned interestSrcName;
    bloom_filter interestBf; //NOTE: must carry the bittable as rawdata when sending DTN Interest	

    DtnInterestPacket():DTNPacket()
    {        
    }
    DtnInterestPacket(DtnInterestPacket const & o):DTNPacket(o), interestSrcName(o.interestSrcName)
    {      
        interestBf= o.interestBf;
    }
    DtnInterestPacket& operator = (DtnInterestPacket const & o)
    {
        DTNPacket::operator = (o);
        interestBf = o.interestBf;
        interestSrcName = o.interestSrcName;
        
        return *this;
    }

    virtual void PrintPacket()
    {
        std::cout<<"===============DTN Interest Packet============="<<std::endl;
        DTNPacket::PrintPacket();
        std::cout  <<"interest src: "<<interestSrcName+1<<std::endl;
    }
    virtual ~DtnInterestPacket() {}
    
} DtnInterestPacket;

typedef struct DtnCacheSummaryPacket:public DTNPacket
{
    bloom_filter cacheBf; //NOTE: must carry the bittable as rawdata when sending DTN cache summary

    DtnCacheSummaryPacket():DTNPacket()
    {        
    }
    DtnCacheSummaryPacket(DtnCacheSummaryPacket const & o):DTNPacket(o)
    {      
        cacheBf= o.cacheBf;
    }
    DtnCacheSummaryPacket& operator = (DtnCacheSummaryPacket const & o)
    {
        DTNPacket::operator = (o);
        cacheBf = o.cacheBf;
        
        return *this;
    }

    virtual void PrintPacket()
    {
        std::cout<<"===============DTN Cache Summary Packet============="<<std::endl;
        DTNPacket::PrintPacket();
        //std::cout  <<"id: "<<id<<std::endl;
    }
    virtual ~DtnCacheSummaryPacket() {}
} DtnCacheSummaryPacket;

typedef struct InterestPacket:public ICNPacket
{
    char szInterestName [MAXPACKETSTRINGLEN];
    char szReqAppId[MAXPACKETSTRINGLEN];

    InterestPacket():ICNPacket()
    {
        memset(szInterestName, 0, sizeof(szInterestName));
        memset(szReqAppId, 0, sizeof(szReqAppId));
    }
    InterestPacket(InterestPacket const & o):ICNPacket(o)
    {
        memset(szInterestName, 0, sizeof(szInterestName));
        memset(szReqAppId, 0, sizeof(szReqAppId));
        strncpy(szInterestName, o.szInterestName, sizeof(szInterestName));
        strncpy(szReqAppId, o.szReqAppId, sizeof(szReqAppId));
    }
    InterestPacket& operator = (InterestPacket const & o)
    {
        ICNPacket::operator = (o);
        memset(szInterestName, 0, sizeof(szInterestName));
        memset(szReqAppId, 0, sizeof(szReqAppId));
        strncpy(szInterestName, o.szInterestName, sizeof(szInterestName));
        strncpy(szReqAppId, o.szReqAppId, sizeof(szReqAppId));
        return *this;
    }

    std::string GetName()
    {
        std::string s(szInterestName, strlen(szInterestName));
        return s;
    }

    virtual void PrintPacket()
    {
        std::cout<<"===============Interest Packet============="<<std::endl;
        ICNPacket::PrintPacket();
        std::string name(szInterestName, strlen(szInterestName));
        std::string req(szReqAppId, strlen(szReqAppId));
        std::cout <<"interest name: "<<name<<std::endl
                  <<"requester id: "<<req<<std::endl;
    }
    virtual ~InterestPacket() {}
} InterestPacket;


typedef struct DataPacket:public ICNPacket
{
    char szInterestName[MAXPACKETSTRINGLEN];
    short nSize; //packet size
    DataPacket():ICNPacket(), nSize(-1)
    {
        memset(szInterestName, 0, sizeof(szInterestName));
    }
    DataPacket(DataPacket const & o)
        :ICNPacket(o), nSize(o.nSize)
    {
        memset(szInterestName, 0, sizeof(szInterestName));
        strncpy(szInterestName, o.szInterestName, sizeof(szInterestName));
    }
    DataPacket& operator =(DataPacket const & o)
    {
        ICNPacket::operator = (o);
        nSize = o.nSize;
        memset(szInterestName, 0, sizeof(szInterestName));
        strncpy(szInterestName, o.szInterestName, sizeof(szInterestName));
        return *this;
    }
    virtual ~DataPacket() {}
    bool IsEmpty()
    {
        if(nSize ==-1 ) return true;
        else return false;
    }

    std::string GetName()
    {
        std::string s(szInterestName, strlen(szInterestName));
        return s;
    }
    virtual void PrintPacket()
    {
        std::cout<<"===============Data Packet============="<<std::endl;
        ICNPacket::PrintPacket();
        std::string name(szInterestName, strlen(szInterestName));
        std::cout
                <<"data name: "<<name<<std::endl
                <<"size: "<<nSize<<std::endl;
    }
} DataPacket;

typedef struct GeoRouteDataPacket:public DataPacket
{
    GeoCoordinate dest;
    GeoCoordinate lastHop;

    GeoRouteDataPacket():DataPacket() {}
    GeoRouteDataPacket(DataPacket const & o):DataPacket(o)
    {
        //Note that lastHop and dest are not initialized yet.
    }

    GeoRouteDataPacket(GeoRouteDataPacket const & o)
        :DataPacket(o), dest(o.dest), lastHop(o.lastHop)
    {
    }
    GeoRouteDataPacket& operator =(GeoRouteDataPacket const & o)
    {
        DataPacket::operator = (o);
        dest = o.dest;
        lastHop = o.lastHop;
        return *this;
    }
    virtual ~GeoRouteDataPacket() {}
    virtual bool Initialized()
    {
        if(dest.Initialized() && lastHop.Initialized()) return true;
        else return ICNPacket::Initialized();
    }

    virtual void PrintPacket()
    {
        std::cout<<"===============GeoData Packet============="<<std::endl;
        DataPacket::PrintPacket();
        std::cout
                <<"dest: "<<dest.toString()<<std::endl
                <<"lastHop: "<<lastHop.toString()<<std::endl;

    }
} GeoRouteDataPacket;


typedef struct GeoFloodInterestPacket:public InterestPacket
{
    GeoCoordinate lastHopLoc;

    GeoFloodInterestPacket():InterestPacket() {}
    GeoFloodInterestPacket(InterestPacket const & o):InterestPacket(o)
    {
        //Note that lastHopLoc and lastHopName are not initialized yet.
    }
    GeoFloodInterestPacket(GeoFloodInterestPacket const & o):InterestPacket(o),
        lastHopLoc(o.lastHopLoc)
    {

    }

    GeoFloodInterestPacket& operator = (GeoFloodInterestPacket const & o)
    {
        InterestPacket::operator = (o);
        lastHopLoc = o.lastHopLoc;
        return *this;
    }
    virtual ~GeoFloodInterestPacket() {}

    virtual void PrintPacket()
    {
        std::cout<<"===============GeoFloodInterest Packet============="<<std::endl;
        InterestPacket::PrintPacket();
        std::cout
                <<"lastHop: "<<lastHopLoc.toString()<<std::endl;
    }

    virtual bool initialized()
    {
        if(lastHopLoc.IsEmpty()) return false;
        else return ICNPacket::Initialized();
    }
} GeoFloodInterestPacket;

typedef struct BfrInterestPacket:public InterestPacket
{
    GeoCoordinate lastHop;
    GeoCoordinate nextDest;
    freshnessIndexUnit freshness;
    short nextDestLevel;

    BfrInterestPacket():InterestPacket() {}
    BfrInterestPacket(InterestPacket const & o):InterestPacket(o)
    {
        //Note that lastHop and nextDest are not initialized yet.
    }
    BfrInterestPacket(BfrInterestPacket const & o):InterestPacket(o),
        lastHop(o.lastHop), nextDest(o.nextDest), freshness(o.freshness), nextDestLevel(o.nextDestLevel)
    {
    }
    BfrInterestPacket& operator = (BfrInterestPacket const & o)
    {
        InterestPacket::operator = (o);
        lastHop = o.lastHop;
        nextDest = o.nextDest;
        freshness = o.freshness;
        nextDestLevel = o.nextDestLevel;
        return *this;
    }
    virtual ~BfrInterestPacket() {}

    virtual void PrintPacket()
    {
        std::cout<<"===============BfrGeoInterest Packet============="<<std::endl;
        InterestPacket::PrintPacket();
        std::cout
                <<"lastHop: "<<lastHop.toString()<<std::endl
                <<"nextDest: "<<nextDest.toString()<<std::endl
                <<"freshness: "<<freshness<<std::endl
                <<"nextDestLevel: "<<nextDestLevel<<std::endl;
    }

    virtual bool initialized()
    {
        if(lastHop.IsEmpty()) return false;
        if(nextDest.IsEmpty()) return false;
        return ICNPacket::Initialized();
    }
} BfrInterestPacket;

typedef struct GeoRouteInterestPacket:public InterestPacket
{
    GeoCoordinate lastHopLoc;
    GeoCoordinate DestLoc;
    GeoCoordinate srcLoc;
    unsigned destName;

    GeoRouteInterestPacket():InterestPacket() {}
    GeoRouteInterestPacket(InterestPacket const & o):InterestPacket(o)
    {
        //Note that all loc and node names are not initialized yet
    }
    GeoRouteInterestPacket(GeoRouteInterestPacket const & o):InterestPacket(o),
        lastHopLoc(o.lastHopLoc), DestLoc(o.DestLoc), srcLoc(o.srcLoc)
    {
    }
    GeoRouteInterestPacket& operator = (GeoRouteInterestPacket const & o)
    {
        InterestPacket::operator = (o);
        lastHopLoc = o.lastHopLoc;
        DestLoc = o.DestLoc;
        srcLoc = o.srcLoc;

        return *this;
    }
    virtual ~GeoRouteInterestPacket() {}

    virtual void PrintPacket()
    {
        std::cout<<"===============GeoInterest Packet============="<<std::endl;
        InterestPacket::PrintPacket();
        std::cout
                <<"lastHop: "<<lastHopLoc.toString()<<std::endl
                <<"dest: "<<destName + 1<<" "<<DestLoc.toString()<<std::endl
                <<"src: "<<srcLoc.toString()<<std::endl;
    }

    virtual bool initialized()
    {
        if(lastHopLoc.IsEmpty()) return false;
        if(DestLoc.IsEmpty()) return false;
        if(srcLoc.IsEmpty()) return false;
        return ICNPacket::Initialized();
    }
} GeoRouteInterestPacket;

//Active Interest Packet header format: ccnHdr | Active Int Header |
typedef unsigned nodeId;
typedef struct ActiveFloodInterestPacket:public InterestPacket
{
    nodeId nodename;
    short nodeListLength;

    ActiveFloodInterestPacket():InterestPacket() {}

    ActiveFloodInterestPacket(InterestPacket const & o, nodeId nname, short length):
        InterestPacket(o), nodename(nname), nodeListLength(length)
    {}

    ActiveFloodInterestPacket(ActiveFloodInterestPacket const & o):InterestPacket(o),
        nodename(o.nodename), nodeListLength(o.nodeListLength)
    {}

    ActiveFloodInterestPacket& operator=(ActiveFloodInterestPacket const & o)
    {
        InterestPacket::operator=(o);
        nodename = o.nodename;
        nodeListLength = o.nodeListLength;
    }

    virtual ~ActiveFloodInterestPacket() {}

    virtual void PrintPacket()
    {
        std::cout<<"===============Nc3nActiveInterst Header============="<<std::endl;
        InterestPacket::PrintPacket();
        std::cout
                <<"nodename: "<<nodename+1<<std::endl
                <<"node list length: "<<nodeListLength<<std::endl;
    }
} ActiveFloodInterestPacket;

#endif
