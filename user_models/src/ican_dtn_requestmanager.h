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
/** @file ICAN_DTN_REQUESTMANAGER_H
  * @brief stores and updates requests from other nodes
  */
  
#ifndef ICAN_DTN_REQUESTMANAGER_H
#define ICAN_DTN_REQUESTMANAGER_H

#include "ican_common.h"

class CDtnRequestManager {
public:
    CDtnRequestManager(Node* node, int interfaceIndex, const NodeInput* nodeInput);
    virtual ~CDtnRequestManager();
    void InsertRequest(unsigned nodeName, bloom_filter requestBf);
    bloom_filter GetRequest(unsigned nodeName);
    bool HasRequest(unsigned nodeName);
    bool EventHandler(Message* msg);
    void PrintDebugInfo();

private:
    RandomSeed m_seed;
    BloomFilterStore* m_pRequestStore;
    CDtnRequestManager(){}
    Node* m_node;
    //TODO request expiration
};

#endif
