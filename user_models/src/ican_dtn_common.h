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

#ifndef ICAN_DTN_COMMON_H
#define ICAN_DTN_COMMON_H


#include "ican_common.h"
#include "bloom_filter.hpp"
#include "coding/nc.h"

enum NetworkCodingOption {nc_sourceonly, nc_mixing, nc_fullobjectonly};

struct Fragmentation_t{
	char parentObjectName[PARENTOBJECTNAMELEN+1];

	int fragmentid;
	size_t sizeBytesParentObject;
	unsigned targetNodeId;

	virtual ~Fragmentation_t() {

	}

    Fragmentation_t(){
    	memset(this->parentObjectName, 0, sizeof(char)*PARENTOBJECTNAMELEN+1);
    	this->fragmentid = 0;
    	this->sizeBytesParentObject = 0;
    	this->targetNodeId = 0;
    }
	Fragmentation_t(std::string _parentObjectName, int _fragmentid,
			size_t _sizeBytesParentObject,unsigned _targetNodeId) : targetNodeId(_targetNodeId) {
 		memset(this->parentObjectName, 0, sizeof(char)*PARENTOBJECTNAMELEN+1);
        strncpy(this->parentObjectName, _parentObjectName.c_str(), sizeof(this->parentObjectName));

		this->fragmentid = _fragmentid;
		this->sizeBytesParentObject = _sizeBytesParentObject;
	}
	Fragmentation_t(Fragmentation_t const&  o) {
		memset(this->parentObjectName, 0, sizeof(char)*PARENTOBJECTNAMELEN+1);
        strncpy(this->parentObjectName, o.parentObjectName, sizeof(this->parentObjectName));

		this->fragmentid = o.fragmentid;
		this->sizeBytesParentObject = o.sizeBytesParentObject;
		this->targetNodeId = o.targetNodeId;
	}
	virtual Fragmentation_t& operator=(Fragmentation_t const& o) {
		memset(this->parentObjectName, 0, sizeof(char)*PARENTOBJECTNAMELEN+1);
	    strncpy(this->parentObjectName, o.parentObjectName, sizeof(this->parentObjectName));
	
		this->fragmentid = o.fragmentid;
		this->sizeBytesParentObject = o.sizeBytesParentObject;
		this->targetNodeId = o.targetNodeId;
		return *this;
	}

        bool operator==(Fragmentation_t & n ) const {
	       	if( strcmp(this->parentObjectName, n.parentObjectName)==0 &&
    	   		this->fragmentid == n.fragmentid &&
    	   		this->targetNodeId == n.targetNodeId &&
        	    this->sizeBytesParentObject == n.sizeBytesParentObject){
	            return true;
  	        }
            return false;
        }

	bool objectEqual(Fragmentation_t & n ) const{
		if( strcmp(this->parentObjectName, n.parentObjectName)==0 &&
//    	   		this->fragmentid == n.fragmentid &&
        	    this->sizeBytesParentObject == n.sizeBytesParentObject){
	            return true;
  	        }
            return false;		
	}		

        std::string getName()
        {
            std::string packetname(this->parentObjectName);
            packetname = packetname + "/"+convertInt(fragmentid);

            return packetname;
        }

        std::string getBufferId()
        {
            std::string bufferid(this->parentObjectName);
            bufferid = bufferid+"/"+convertInt(fragmentid)+"/"+IntToString(targetNodeId);
            return bufferid;
        }


        void Print(){            
			std::string objName(parentObjectName);
           std::cout<< "ParentObjectName: "<<objName<<std::endl
            <<"fragment ID: "<<fragmentid<<std::endl
            <<"sizeParentObject:"<<sizeBytesParentObject<<std::endl
            <<"targetNodeId:"<<targetNodeId<<std::endl;
        }
};

struct NetworkCoding_t {
	char parentObjectName[PARENTOBJECTNAMELEN+1];
	long parentObjectFileSize;
	unsigned targetNodeId;
	int networkcodingblockid;
	bool isMixed;

	virtual ~NetworkCoding_t() {

	}
	NetworkCoding_t() {
		memset(this->parentObjectName, 0, sizeof(char)*PARENTOBJECTNAMELEN+1);
		this->parentObjectFileSize = -1;
		this->targetNodeId = -1;
		this->networkcodingblockid = -1;
		this->isMixed = false;
	}
	NetworkCoding_t(std::string _parentObjectName,
			long _parentObjectFileSize,
			unsigned _targetNodeId,int _networkcodingblockid, bool _isMixed ) {
		memset(this->parentObjectName, 0, sizeof(char)*(PARENTOBJECTNAMELEN+1));
        strncpy(this->parentObjectName, _parentObjectName.c_str(), sizeof(this->parentObjectName));

		this->parentObjectFileSize = _parentObjectFileSize;
		this->targetNodeId = _targetNodeId;
		this->networkcodingblockid = _networkcodingblockid;
		this->isMixed = _isMixed;
	}
	NetworkCoding_t(NetworkCoding_t const & o) {
		memset(this->parentObjectName, 0, sizeof(char)*(PARENTOBJECTNAMELEN+1));
	        strncpy(this->parentObjectName, o.parentObjectName, sizeof(this->parentObjectName));
	
		this->parentObjectFileSize = o.parentObjectFileSize;
		this->targetNodeId = o.targetNodeId;
		this->networkcodingblockid = o.networkcodingblockid;
		this->isMixed = o.isMixed;
	}
	virtual NetworkCoding_t& operator=(NetworkCoding_t const& o) {

		memset(this->parentObjectName, 0, sizeof(char)*(PARENTOBJECTNAMELEN+1));
	        strncpy(this->parentObjectName, o.parentObjectName, sizeof(this->parentObjectName));

		this->parentObjectFileSize = o.parentObjectFileSize;
		this->targetNodeId = o.targetNodeId;
		this->networkcodingblockid = o.networkcodingblockid;
		this->isMixed = o.isMixed;
		return *this;
	}

	bool operator==(NetworkCoding_t & n ) const {
		//TODO: how to check nc coefficients? or maybe we can simply skip it?
		if( strcmp(this->parentObjectName, n.parentObjectName)==0 &&
			this->parentObjectFileSize == n.parentObjectFileSize &&
			this->networkcodingblockid == n.networkcodingblockid &&
			this->targetNodeId == n.targetNodeId &&
			this->isMixed == n.isMixed){
			return true;
		}
		return false;
    }

	bool objectEqual(NetworkCoding_t & n ) const {
		//TODO: how to check nc coefficients? or maybe we can simply skip it?
		if( strcmp(this->parentObjectName, n.parentObjectName)==0 &&
			this->parentObjectFileSize == n.parentObjectFileSize &&
//			this->networkcodingblockid == n.networkcodingblockid &&
			this->isMixed == n.isMixed){
			return true;
		}
		return false;
    }

	bool operator<(const NetworkCoding_t &other) const { return this->networkcodingblockid < other.networkcodingblockid; }

	std::string getName() {
    	std::string separatorName("/");
    	std::string stringBlockId = convertInt(networkcodingblockid);

        std::string objectName =
        		(this->parentObjectName)+separatorName+stringBlockId;
        return objectName;
	}

	std::string getBlockIdOnly() {
		std::string stringBlockId = convertInt(networkcodingblockid);
		return stringBlockId;
	}

	std::string getBufferId() {
    	std::string separatorName("/");
    	std::string stringBlockId = convertInt(networkcodingblockid);
    	std::string stringTargetNodeId = convertInt(targetNodeId);

        std::string objectName =
        		(this->parentObjectName)+separatorName+stringBlockId+separatorName+stringTargetNodeId;
        return objectName;
	}

	bool isEmpty(){
		if(this->networkcodingblockid == -1 || this->networkcodingblockid == 0){
			return true;
		}
		return false;
	}


	void Print(){            
		std::string objName(parentObjectName);
           std::cout<< "ParentObjectName: "<<objName<<std::endl
            <<"NC ID: "<<networkcodingblockid<<std::endl
            <<"sizeParentObject:"<<parentObjectFileSize<<std::endl
            <<"targetNodeId:"<<targetNodeId<<std::endl;
        }
};



#endif

