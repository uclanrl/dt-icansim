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
#ifndef ICAN_COMMON_EVENTINFO_H
#define ICAN_COMMON_EVENTINFO_H

typedef struct IcanDtnAppTimerInfo
{
    char namePrefix[MAXPACKETSTRINGLEN];
    int size;
    IcanDtnAppTimerInfo() {}
    IcanDtnAppTimerInfo(IcanDtnAppTimerInfo const & o)
        :size(o.size)
    {
        strncpy(namePrefix, o.namePrefix, sizeof(namePrefix));
    }

    virtual ~IcanDtnAppTimerInfo(){}	
} IcanDtnAppTimerInfo;

typedef struct IcanDtnAckInfo
{
    char blockname[HANDSHAKENAMELEN];
    bool isNc;
    IcanDtnAckInfo() {}

    IcanDtnAckInfo(std::string _blockname, bool _isNc):isNc(_isNc){
 	memset(this->blockname, 0, sizeof(char)*HANDSHAKENAMELEN+1);
        strncpy(this->blockname, _blockname.c_str(), sizeof(this->blockname));
    }	
    IcanDtnAckInfo(IcanDtnAckInfo const & o)
        :isNc(o.isNc)
    {
        strncpy(blockname, o.blockname, sizeof(blockname));
    }

    virtual ~IcanDtnAckInfo(){}		
} IcanDtnAckInfo;

typedef struct IcanClientInfo
{
    char namePrefix[MAXPACKETSTRINGLEN];
    bool firstInterest;
    IcanClientInfo() {}
    IcanClientInfo(IcanClientInfo const & o)
        :firstInterest(o.firstInterest)
    {
        strncpy(namePrefix, o.namePrefix, sizeof(namePrefix));
    }

    virtual ~IcanClientInfo(){}				
} IcanClientInfo;

struct IcanAppRetxInfo
{
    char namePrefix[MAXPACKETSTRINGLEN];
    char szName[MAXPACKETSTRINGLEN];
    int nRtx; //number of transmissions
    IcanAppRetxInfo()
    {
        memset(namePrefix, 0, sizeof(namePrefix));
        memset(szName, 0, sizeof(szName));
    }
    IcanAppRetxInfo(IcanAppRetxInfo const & o)
        :nRtx(o.nRtx)
    {
        memset(namePrefix, 0, sizeof(namePrefix));
        memset(szName, 0, sizeof(szName));
        strncpy(namePrefix, o.namePrefix, sizeof(namePrefix));
        strncpy(szName, o.szName, sizeof(szName));
    }

	virtual ~IcanAppRetxInfo(){}
};


typedef struct IcanPitEntryExpireInfo
{
    char szName[MAXPACKETSTRINGLEN];
    char szReqId[MAXPACKETSTRINGLEN];
    clocktype expireTime;
    IcanPitEntryExpireInfo()
    {
        memset(szName, 0, sizeof(szName));
        memset(szReqId, 0, sizeof(szReqId));
    }
    IcanPitEntryExpireInfo(IcanPitEntryExpireInfo const & o)
        :expireTime(o.expireTime)
    {
        memset(szName, 0, sizeof(szName));
        memset(szReqId, 0, sizeof(szReqId));
        strncpy(szName, o.szName, sizeof(szName));
        strncpy(szReqId, o.szReqId, sizeof(szReqId));
    }

	virtual ~IcanPitEntryExpireInfo(){}
} PitEntryExpireInfo;

typedef struct IcanPitEntryReplayInfo
{
    char szName[MAXPACKETSTRINGLEN];
    char szReqId[MAXPACKETSTRINGLEN];
    char szNonce[MAXPACKETSTRINGLEN];
    int currentRtx;
    clocktype expireTime;
    IcanPitEntryReplayInfo()
    {
        memset(szName, 0, sizeof(szName));
        memset(szReqId, 0, sizeof(szReqId));
        memset(szNonce, 0, sizeof(szNonce));
    }
    IcanPitEntryReplayInfo(IcanPitEntryReplayInfo const & o)
        : currentRtx(o.currentRtx), expireTime(o.expireTime)
    {
        memset(szName, 0, sizeof(szName));
        memset(szReqId, 0, sizeof(szReqId));
        memset(szNonce, 0, sizeof(szNonce));
        strncpy(szName, o.szName, sizeof(szName));
        strncpy(szReqId, o.szReqId, sizeof(szReqId));
        strncpy(szNonce, o.szNonce, sizeof(szNonce));
    }
	virtual ~IcanPitEntryReplayInfo(){}
} PitEntryReplayInfo;

 
typedef struct BackoffEventInfo{
    std::string szNonce;
    bool isFromApp;
    bool isReplay;
	virtual ~BackoffEventInfo(){}
}BackoffEventInfo;

#endif
