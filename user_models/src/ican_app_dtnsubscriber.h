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

#ifndef ICAN_APP_DTNSUBSCRIBER_H
#define ICAN_APP_DTNSUBSCRIBER_H

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "clock.h"
#include "ican_common.h"
#include "api.h"
#include "ican_app_client_interface.h"

typedef std::set<std::string> RegisteredNameSet; //keeps track of the name I have registered

struct DtnSubscriberStat
{
    int totalDORegistered;
    int totalDOReceived;
    
    DtnSubscriberStat():totalDORegistered(0), totalDOReceived(0){}
    DtnSubscriberStat(DtnSubscriberStat const & o):totalDORegistered(o.totalDORegistered), 
        totalDOReceived(o.totalDOReceived) {}
    ~DtnSubscriberStat() {}
};

class CIcanDtnSubscriber: public CIcanFtpClientInterface
{
public:
    CIcanDtnSubscriber(Node* node, std::string szConfigName, std::string szExpName);
    CIcanDtnSubscriber(Node* node, std::string szExpName);
    ~CIcanDtnSubscriber();
    void PrintDebugInfo();
    bool EventHandler(Node* node, Message* msg);
    void DataPacketHandler(Node* node, Message* msg);
    void PrintStat(Node* node,   NetworkType networkType);
    bool IsTransferComplete(Node* node);

private:
    CIcanDtnSubscriber();
    RandomSeed seed;
    Node* m_node;

    std::string m_expName;
    
    std::string m_szAppId;
    std::string m_szPrefix;    //NOTE: objects are named sequentially as prefix1, prefix2, ...etc, blocks are named prefix1/1, prefix1/2...etc
    int m_nNumObj;
    std::vector<clocktype> m_tStartTime; //start time for each object
    std::vector<clocktype> m_tEndTime;

    DtnSubscriberStat m_stat;

    RegisteredNameSet* m_pRegisteredObject;

    void RegisterInterest();
    void ParseConfig(std::string szConfigName);

    std::map<std::string, clocktype> m_RegisterTimeKeeper;
    std::map<std::string, clocktype> m_delayKeeper;
};

#endif

