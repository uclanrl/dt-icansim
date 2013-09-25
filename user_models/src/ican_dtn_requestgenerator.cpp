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
#include "ican_dtn_requestgenerator.h"

CDtnRequestGenerator::CDtnRequestGenerator(Node* node, const NodeInput* nodeInput, int interfaceIndex, 
    CDtnInterestManager* interest_manager_, CDtnInterestGenerator* interest_generator_, 
    CDtnCacheSummaryStore* cachesummary_store_,  CDtnCacheSummaryGenerator* cachesummary_gen_)
    :m_node(node), m_pInterestManager(interest_manager_), m_pInterestGenerator(interest_generator_), m_pCacheSummaryStore(cachesummary_store_)
    , m_pCacheSummaryGen(cachesummary_gen_)
{
    //initialize seed
    RANDOM_SetSeed(m_seed,node->globalSeed,node->nodeId,ROUTING_PROTOCOL_ICAN, interfaceIndex);
    
	//initiate bloomfilter parameters
    BOOL retVal;
    //read expected number of item
    int nItem;
    IO_ReadInt(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "BF-NUMITEM",
        &retVal,
        &nItem);
    if (retVal == FALSE || nItem < 0)
    {
        char errorString[MAX_STRING_LENGTH];
        sprintf(errorString,
                "Wrong BF-NUMITEM configuration format!\n"
               );
        ERROR_ReportError(errorString);
    }
    //read expected false postive rate
    double fpRate;
    IO_ReadDouble(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "BF-FPRATE",
        &retVal,
        &fpRate);
    if (retVal == FALSE || nItem < 0)
    {
        char errorString[MAX_STRING_LENGTH];
        sprintf(errorString,
                "Wrong BF-FPRATE configuration format!\n"
               );
        ERROR_ReportError(errorString);
    }
    //read salt seed
    int seed;
    IO_ReadInt(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "BF-SALT",
        &retVal,
        &seed);
    if (retVal == FALSE || nItem < 0)
    {
        char errorString[MAX_STRING_LENGTH];
        sprintf(errorString,
                "Wrong BF-SALT configuration format!\n"
               );
        ERROR_ReportError(errorString);
    }

    //calculate BF parameters
    bfparameters.projected_element_count    = nItem;
    bfparameters.false_positive_probability = fpRate;
    bfparameters.random_seed                = seed;
    bfparameters.compute_optimal_parameters();
}

CDtnRequestGenerator::~CDtnRequestGenerator()
{
}

Message* CDtnRequestGenerator::MakeDtnRequest()
{  
    //Create request bloomfilter
    bloom_filter nodeBf(bfparameters);

    bloom_filter myBf = m_pInterestGenerator->MakeDtnInterestBloomfilter();
    nodeBf |= myBf;  

    std::list<unsigned> nodeList = GetAllNodeIndex(m_node);
    
    for (std::list<unsigned>::iterator it = nodeList.begin(); it != nodeList.end(); it++){
        unsigned neighborIndex = (*it);

        bloom_filter friendBf = m_pInterestManager->GetInterest(neighborIndex); //friend's request bf

        //if I have received whatever my friend wants, skip this interest
        bloom_filter intersectionBf = m_pCacheSummaryGen->GetDtnCacheSummary();
        intersectionBf &= friendBf;
        if(intersectionBf == friendBf){
            continue;
        }

        if(m_pCacheSummaryStore->HasCacheSummary(neighborIndex)){
            bloom_filter friendCacheSummary = m_pCacheSummaryStore->GetCacheSummary(neighborIndex);
            //if friend has receive whatever it wants, skip this interest
            intersectionBf = friendBf;
            intersectionBf &= friendCacheSummary;
            if(intersectionBf == friendBf){//means friend has everything requested
                continue;
            }
        }

        nodeBf |= friendBf;
    }
        

    if(nodeBf.IsEmpty()){
        //do not send interest packet if the node is not a subscriber for anything
        return NULL;
    }

    //create interest
    DtnRequestPacket dtnPkt;

    dtnPkt.srcName = m_node->nodeIndex;
    dtnPkt.requestBf = nodeBf;
 
    packetType pType = tyDtnRequest;
    
    Message* dtnRequestPkt = GeneratePacketWithRawData(m_node, &dtnPkt, pType, 0, nodeBf.GetRawTableSize(), nodeBf.table()); 	//virtual payload size 0

    return dtnRequestPkt;
}

void CDtnRequestGenerator::PrintDebugInfo()
{
    std::cout<<"DTNRequestGenerator DebugInfo"<<std::endl;
    std::cout<<std::endl;
}
