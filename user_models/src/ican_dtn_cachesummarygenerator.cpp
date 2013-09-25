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
#include "ican_dtn_cachesummarygenerator.h"

CDtnCacheSummaryGenerator::CDtnCacheSummaryGenerator(Node* node, const NodeInput* nodeInput, int interfaceIndex, DtnDataStore* datastore):m_node(node), m_dtnDataStore(datastore)
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

CDtnCacheSummaryGenerator::~CDtnCacheSummaryGenerator()
{
        
}

Message* CDtnCacheSummaryGenerator::MakeDtnCacheSummary()
{
    //create cache summary BF
    
    bloom_filter nodeBf = GetDtnCacheSummary();

    //create interest
    DtnCacheSummaryPacket dtnPkt;
    
    dtnPkt.srcName = m_node->nodeIndex;
    dtnPkt.cacheBf = nodeBf;
 
    packetType pType = tyDtnCacheSummary;
    
    //make Qualnet packet
    Message* pkt = GeneratePacketWithRawData(m_node, &dtnPkt, pType, 0, nodeBf.GetRawTableSize(), nodeBf.table()); 	//virtual payload size 0

    return pkt;
}

bloom_filter CDtnCacheSummaryGenerator::GetDtnCacheSummary()
{
    //create cache summary BF
    
    bloom_filter nodeBf(bfparameters);

    for(DtnDataStore::const_iterator itr = (*m_dtnDataStore).begin(); itr != (*m_dtnDataStore).end(); ++itr){ 
        nodeBf.insert((*itr).first);                    
    }    

    return nodeBf;
}


void CDtnCacheSummaryGenerator::PrintDebugInfo()
{ 
}
