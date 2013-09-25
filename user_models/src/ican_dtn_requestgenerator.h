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
/** @file ican_dtn_interestgenerator.h
  * @brief (1) Manage registration of data objects to pull and (2) generate node interest (a summary of data objects a node is interested in)
  */
  
#ifndef ICAN_DTN_REQUESTGENRATOR_H
#define ICAN_DTN_REQUESTGENRATOR_H

#include "ican_common.h"
#include "bloom_filter.hpp"
#include "ican_dtn_interestmanager.h"
#include "ican_dtn_interestgenerator.h"
#include "ican_dtn_cachesummarystore.h"
#include "ican_dtn_cachesummarygenerator.h"

class CDtnRequestGenerator {
public:
    CDtnRequestGenerator(Node* node, const NodeInput* nodeInput, int interfaceIndex, 
        CDtnInterestManager* interest_manager_, CDtnInterestGenerator* interest_generator_, CDtnCacheSummaryStore* cacheSummary_Store_,
        CDtnCacheSummaryGenerator* m_pCacheSummaryGen);

    virtual ~CDtnRequestGenerator();
    Message* MakeDtnRequest();

    void PrintDebugInfo();
private:
    RandomSeed m_seed;
    CDtnRequestGenerator(){}
    Node* m_node;
    bloom_parameters bfparameters;
    CDtnInterestManager* m_pInterestManager;
    CDtnInterestGenerator* m_pInterestGenerator;
    CDtnCacheSummaryStore* m_pCacheSummaryStore;
    CDtnCacheSummaryGenerator* m_pCacheSummaryGen; 
};

#endif
