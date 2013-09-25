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

#include "ican_dtn_interestgenerator.h"

CDtnInterestGenerator::CDtnInterestGenerator(Node* node, const NodeInput* nodeInput, int interfaceIndex):m_node(node)
{
    //initialize seed
    RANDOM_SetSeed(m_seed,node->globalSeed,node->nodeId,ROUTING_PROTOCOL_ICAN, interfaceIndex);
    
    m_pInterestRegistry = new DtnInterestRegistry;

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

CDtnInterestGenerator::~CDtnInterestGenerator()
{
    delete m_pInterestRegistry;
}

Message* CDtnInterestGenerator::MakeDtnInterest()
{
    //Create node description (interest) bloomfilter
    bloom_filter nodeBf(bfparameters);

    for(DtnInterestRegistry::iterator it = m_pInterestRegistry->begin(); it != m_pInterestRegistry->end(); it++){
        //std::cout<<*it<<std::endl;
        nodeBf.insert(*it);
    }

    if(nodeBf.IsEmpty()){
        //do not send interest packet if the node is not a subscriber for anything
        return NULL;
    }

    //create interest
    DtnInterestPacket dtnPkt;

    dtnPkt.srcName = m_node->nodeIndex;
    dtnPkt.interestBf = nodeBf;
    dtnPkt.interestSrcName = m_node->nodeIndex;
 
    packetType pType = tyDtnInterest;
    
    //make Qualnet packet
    Message* dtnInterestPkt = GeneratePacketWithRawData(m_node, &dtnPkt, pType, 0, nodeBf.GetRawTableSize(), nodeBf.table()); 	//virtual payload size 0

    return dtnInterestPkt;
}

bloom_filter CDtnInterestGenerator::MakeDtnInterestBloomfilter(){
    //Create node description (interest) bloomfilter
    bloom_filter nodeBf(bfparameters);

    for(DtnInterestRegistry::iterator it = m_pInterestRegistry->begin(); it != m_pInterestRegistry->end(); it++){
        //std::cout<<*it<<std::endl;
        nodeBf.insert(*it);
    }

    return nodeBf;
}

void CDtnInterestGenerator::RegisterInterest(std::string name)
{
    m_pInterestRegistry->insert(name);
}

void CDtnInterestGenerator::UnregisterInterest(std::string name)
{
    DtnInterestRegistry::iterator it;
    it = m_pInterestRegistry->find(name);
    if(it != m_pInterestRegistry->end())
        m_pInterestRegistry->erase(it);
}

void CDtnInterestGenerator::PrintDebugInfo()
{
    PrintTime(m_node);
    set<std::string>::iterator it;
    std::cout<<"node "<<m_node->nodeIndex+1<<" subscribes to ";
    for(it = m_pInterestRegistry->begin(); it != m_pInterestRegistry->end(); it++)
        std::cout<<*it<<" ";
    std::cout<<std::endl;
}
