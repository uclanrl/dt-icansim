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

#ifndef NC3N_FRAGMENTATION_H
#define NC3N_FRAGMENTATION_H

#include "ican_common.h"
#include "ican_dtn_common.h"
#include "ican_dtn_cachesummarystore.h"

#define BLOCKSIZE 512

//keeps track of fragment ids received per object
typedef std::set<int> fragmentidset_t;
typedef std::map<std::string, fragmentidset_t> fragmentsreceived_t;

//keeps track of total fragments necessary per object
typedef std::map<std::string, int> fragmentstotal_t;

struct FragmentationStat
{
    unsigned totalObjectReconstructed;
    unsigned totalBlockReceived;
    unsigned totalBlockGenerated;
};

class FragmentationManager {
public:
    FragmentationManager(size_t blockSize, Node* node, CDtnCacheSummaryStore*  m_pCacheSummaryStore_, DtnDataStore* _dtnstore);
    bool EventHandler(Node* node,Message* msg);
    void Printstat(Node* node, NetworkType networkType);
    void receiveFragment(Fragmentation_t fragmentation);
    Fragmentation_t createFragmentFromObject(std::string parentObjectName, unsigned targetNodeIndex,size_t sizeBytesParentObject);
    bool isDownloadCompleted(std::string parentObjectName, unsigned targetNodeIndex,size_t sizeBytesParentObject);
    void cleanupFragmentsInDataStore(std::string parentObjectName,size_t sizeBytesParentObject);

private:
    fragmentidset_t getFragmentsToSendTarget(std::string parentObjectName, unsigned targetNodeIndex,size_t sizeBytesParentObject);

    //not called
    FragmentationManager();
    size_t blockSize;
    Node* m_node;
    fragmentsreceived_t fragmentsreceived;
    fragmentstotal_t fragmentstotal;
    CDtnCacheSummaryStore* m_pCacheSummaryStore;
    DtnDataStore* dtnstore;

    FragmentationStat m_stat;
};

#endif
