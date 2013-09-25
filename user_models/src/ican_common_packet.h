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

#ifndef ICAN_COMMON_PACKET_H
#define ICAN_COMMON_PACKET_H

#include "bloom_filter.hpp"

enum packetType {tyDtnRequest, tyDtnCacheSummary, tyFragment, tyDtnRts, tyDtnCts, tyDtnAck, tyDtnInterest};

extern std::string pktTypeString[];

extern std::string IntToString(int i);

bool IsDtnPacket(packetType pType);
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

    unsigned srcName;  

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
    bool isHaveBlock; 
    bool isHaveObject; 
    bool isRejectNcMixing; 
    
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
    	std::string stringFragmentId = IntToString(fragmentid);
    	std::string stringSequenceNumber = IntToString(sequenceNumber);
    	std::string separatorName("/");
        std::string packetname = (this->parentObjectName)
        		+separatorName+stringFragmentId+separatorName+stringSequenceNumber;
        return packetname;
    }

    std::string GetFragmentName()
    {
    	std::string stringFragmentId = IntToString(fragmentid);
    	std::string separatorName("/");
        std::string packetname = (this->parentObjectName)
        		+separatorName+stringFragmentId;
        return packetname;
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
    }
    virtual ~DtnCacheSummaryPacket() {}
} DtnCacheSummaryPacket;

#endif
