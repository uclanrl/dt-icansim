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

#include "ican_dtnmanager.h"

CIcanDtnManager::CIcanDtnManager(Node* node, const NodeInput* nodeInput, DtnDataStore* dtnstore, int interfaceIndex):m_node(node), m_pDtnDataStore(dtnstore){

    memset(&m_stat, 0, sizeof(m_stat));

    //initialize seed
    RANDOM_SetSeed(m_seed,node->globalSeed,node->nodeId,ROUTING_PROTOCOL_ICAN, interfaceIndex);
    
    //read input config    
    BOOL retVal;

     IO_ReadInt(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "DTN-MAX-RTS-RETRY",
        &retVal,
        &m_maxRtsRetry);
    if(retVal == FALSE)
    {
	ReportError("DTN-MAX-RTS-RETRY is missing");
    }


    IO_ReadDouble(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "DTN-TRANSMISSION-RANGE",
        &retVal,
        &m_txrange);
    if(retVal == FALSE)
    {
	ReportError("DTN-TRANSMISSION-RANGE is missing");
    }


    IO_ReadTime(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "RTS-EXPIRE-INTERVAL",
        &retVal,
        &m_rtsExpireTime);
    if(retVal == FALSE)
    {
	ReportError("RTS-EXPIRE-INTERVAL");
    }

    IO_ReadTime(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "CTS-EXPIRE-INTERVAL",
        &retVal,
        &m_ctsExpireTime);
    if(retVal == FALSE)
    {
	ReportError("CTS-EXPIRE-INTERVAL");
    }

    IO_ReadTime(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "DATASENDSTATE-EXPIRE-INTERVAL",
        &retVal,
        &m_dataSendExpireTime);
    if(retVal == FALSE)
    {
	ReportError("DATASENDSTATE-EXPIRE-INTERVAL");
    }
 
    
    IO_ReadTime(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "DTN-INTEREST-INTERVAL",
        &retVal,
        &m_InterestInterval);
    if(retVal == FALSE)
	{
		ReportError("DTN-INTEREST-INTERVAL is missing");
    }

    IO_ReadTime(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "DTN-REQUEST-INTERVAL",
        &retVal,
        &m_RequestInterval);
    if(retVal == FALSE)
	{
		ReportError("DTN-REQUEST-INTERVAL is missing");
    }

    IO_ReadTime(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "DTN-CACHESUMMARY-INTERVAL",
        &retVal,
        &m_cacheSummaryInterval);
    if(retVal == FALSE)
	{
		ReportError("DTN-CACHESUMMARY-INTERVAL is missing");
    }


    IO_ReadTime(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "NODEDESCRIPTION-JITTER",
        &retVal,
        &m_nodeDescriptionJitter);
    if(retVal == FALSE)
	{
		ReportError("NODEDESCRIPTION-JITTER is missing");
    }

    IO_ReadTime(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "NODEDESCRIPTION-STARTING-JITTER",
        &retVal,
        &m_nodeDescriptionStartingJitter);
    if(retVal == FALSE)
	{
		ReportError("NODEDESCRIPTION-STARTING-JITTER is missing");
    }
    
    m_pInterestRegistry = new CDtnInterestGenerator(node, nodeInput, interfaceIndex);
    m_pCacheSummaryGen = new CDtnCacheSummaryGenerator(node, nodeInput, interfaceIndex, dtnstore);

    m_pCacheSummaryStore = new CDtnCacheSummaryStore(m_node, nodeInput, interfaceIndex);
    
    m_pInterestManager = new CDtnInterestManager(node, interfaceIndex, nodeInput);
    m_pDataRequestManager = new CDtnRequestManager(node, interfaceIndex, nodeInput);

    m_pDataRequestGenerator = new CDtnRequestGenerator(node, nodeInput, interfaceIndex, m_pInterestManager, 
        m_pInterestRegistry, m_pCacheSummaryStore, m_pCacheSummaryGen);

    //add random jitter for first interest, request, and 
    clocktype jitter = GenerateRandomDelay(m_node, m_nodeDescriptionStartingJitter, m_seed);
    SetIcanEvent(m_node, jitter, MSG_ROUTING_ICAN_DTNMANAGER_SENDDTNINTEREST, 0, NULL);            
    SetIcanEvent(m_node, jitter, MSG_ROUTING_ICAN_DTNMANAGER_SENDDTNCACHESUMMARY, 0, NULL);            
    SetIcanEvent(m_node, jitter, MSG_ROUTING_ICAN_DTNMANAGER_SENDDTNREQUEST, 0, NULL);            

    int sizeBlockInBlockUnit;
    IO_ReadInt(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "BLOCK-SIZE",
        &retVal,
        &sizeBlockInBlockUnit);
    if(retVal == FALSE)
    {
	ReportError("BLOCK-SIZE is missing");
    }

    IO_ReadBool(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "ICAN-NC",
        &retVal,
        &m_enableNc);
    if(retVal == FALSE)
    {
	ReportError("ICAN-NC is missing");    
    }    

    char buf[MAX_STRING_LENGTH];
    NetworkCodingOption ncoption;
        
    IO_ReadString(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "NETWORKCODING-OPTION",
        &retVal,
        buf);
    if(retVal == FALSE)
    {
        ncoption = nc_fullobjectonly;
    }
    else
    {
        if(strcmp(buf, "SOURCE-ONLY")==0)
        {
            ncoption = nc_sourceonly;
        }
        else if(strcmp(buf, "MIXING")==0){
            ncoption = nc_mixing;
        }
        else if(strcmp(buf, "FULL-OBJECT-ONLY")==0){
            ncoption = nc_fullobjectonly;
        }
        else
        {
            char errorString[MAX_STRING_LENGTH];
            sprintf(errorString,
                    "Wrong ICAN-NC-OPTION configuration format! Acceptable options:\n"
                    "PIT\n"
                   );
            ERROR_ReportError(errorString);
        }
    }

    m_networkcoding = new NetworkCoding(node,this->m_pCacheSummaryStore, ncoption, BLOCKUNIT*sizeBlockInBlockUnit,this->m_pDtnDataStore);
    
    m_fragmentation = new FragmentationManager(BLOCKUNIT*sizeBlockInBlockUnit,node, this->m_pCacheSummaryStore,this->m_pDtnDataStore);
    

    m_objectQueue = new CDtnObjectQueue(node, nodeInput, interfaceIndex);

    int dtnPktSize;
    IO_ReadInt(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "DTN-PACKET-SIZE",
        &retVal,
        &dtnPktSize);
    if(retVal == FALSE)
    {
	ReportError("DTN-PACKET-SIZE is missing");
    }
    m_fragmentPacketUtil = new fragmentationpacketutil(dtnPktSize, BLOCKUNIT*sizeBlockInBlockUnit, node);

    m_ncPacketUtil = new networkcodingpacketutil(dtnPktSize, (size_t) (BLOCKUNIT*sizeBlockInBlockUnit), node); 

    m_pHandshakeObjBuffer = new HandshakeObjectBuffer;
    m_pHandshakeDataWaitingState = new HandshakeWaitingState;
    m_pHandshakeAckWaitingState = new HandshakeWaitingState;

    m_pFileSizeBuffer = new FileSizeBuffer;     

}

CIcanDtnManager::~CIcanDtnManager(){

    //delete any class instance created
    //e.g. delete m_ncManager;
    
    delete m_pInterestRegistry;
    delete m_pCacheSummaryGen;

    delete m_pCacheSummaryStore;
    delete m_pInterestManager;

    if(m_fragmentation){
        delete m_fragmentation;
        m_fragmentation = NULL;
    }

    if(m_networkcoding){
        delete this->m_networkcoding;
        this->m_networkcoding = NULL;
    }

    delete m_objectQueue;
    m_objectQueue = NULL;

    delete m_pHandshakeObjBuffer;
    m_pHandshakeObjBuffer = NULL;

    delete m_pHandshakeDataWaitingState;
    m_pHandshakeDataWaitingState = NULL;

    delete m_pHandshakeAckWaitingState;
    m_pHandshakeAckWaitingState = NULL;

    delete m_pDataRequestManager;
    m_pDataRequestManager = NULL;

    delete m_pDataRequestGenerator;
    m_pDataRequestGenerator = NULL;

    delete m_ncPacketUtil;
    m_ncPacketUtil = NULL;

    delete m_pFileSizeBuffer;
    m_pFileSizeBuffer = NULL;
}

void CIcanDtnManager::PrintDebugInfo(){	
	std::cout<<"DTN Manager Debug Info:"<<std::endl;

        if(m_pDtnDataStore){
            if(m_pDtnDataStore->size() > 0)
                std::cout<<"node "<<m_node->nodeIndex+1<<" published ";

            DtnDataStore::const_iterator itr;
            for(itr = (*m_pDtnDataStore).begin(); itr != (*m_pDtnDataStore).end(); ++itr){
                cout << "Object: " << (*itr).first << " size- " << (*itr).second<<"     ";
            }
            std::cout<<std::endl;       
        }

        std::cout<<std::endl;
}

void CIcanDtnManager::NotifySubscriber(std::string objName, size_t objSize){
    //notify subscriber, subscriber should unregister object if needed
    IcanDtnAppTimerInfo receiveObjInfo;
    strncpy(receiveObjInfo.namePrefix, objName.c_str(), sizeof(receiveObjInfo.namePrefix));
    receiveObjInfo.size = objSize;
        
    SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTNSUBSCRIBER_RECEIVE, sizeof(receiveObjInfo), &receiveObjInfo);
}
    
bool CIcanDtnManager::EventHandler(Message* msg){
    switch (MESSAGE_GetEvent(msg))
    {
    case MSG_ROUTING_ICAN_DTNMANAGER_SENDACK:{

        IcanDtnAckInfo* ackinfo = (IcanDtnAckInfo*) MESSAGE_ReturnInfo(msg);
        bool isNc = ackinfo->isNc;
        std::string blockName(ackinfo->blockname);

        SendACK(blockName, isNc);
        
        MESSAGE_Free(m_node, msg);
    	return true;
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_NETWORKCODINGRECONSTRUCTED: {
    	NetworkCoding_t* codingMetadata = (NetworkCoding_t*) MESSAGE_ReturnInfo(msg);
        
        std::string fragmentName = codingMetadata->getName();
        std::cout<<"node "<<m_node->nodeIndex+1<<": "<<fragmentName<<" has been reconstructed"<<std::endl;

        std::string objName = GetPrefixFromName(fragmentName);

        if(m_pDtnDataStore->find(objName)==m_pDtnDataStore->end()){
            //add the fragment id to datastore only when the full object is not found in datastore           
            (*m_pDtnDataStore)[fragmentName] = codingMetadata->parentObjectFileSize; //NOTE: must store parent object size for fragment
        }

        //update file size buffer
        (*m_pFileSizeBuffer)[objName] = codingMetadata->parentObjectFileSize;

        //pass this fragment to fragmentation
        m_networkcoding->receiveCodedBlock(*codingMetadata);

        if(m_pHandshakeDataWaitingState->find(fragmentName)!=m_pHandshakeDataWaitingState->end()){            
            //send ACK only if I'm waiting for this data...       
            //remove data waiting flag
            m_pHandshakeDataWaitingState->erase(fragmentName);

            IcanDtnAckInfo ackinfo(fragmentName, true);
            SetIcanEvent(m_node, 1*MICRO_SECOND, MSG_ROUTING_ICAN_DTNMANAGER_SENDACK, sizeof(ackinfo), &ackinfo);
        }

        //TODO: neighbor discovery
        std::vector<Node*> neighborList = GetMyNeighborList(m_node, m_txrange);
        for(vector<Node* >::iterator it = neighborList.begin();it != neighborList.end();it++){
            Node* neighbor = (*it);
            if(m_node->nodeIndex - neighbor->nodeIndex == 0){
                continue;
            }                                
            RequestMatchingByObject(neighbor->nodeIndex,  fragmentName);
        }
 
        MESSAGE_Free(m_node, msg);
    	return true;
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_NETWORKCODINGOBJECTCONSTRUCTED: {    																	
    	NetworkCoding_t* codingMetadata = (NetworkCoding_t*) MESSAGE_ReturnInfo(msg);

	std::string blockName(codingMetadata->getName());
	std::string objName = GetPrefixFromName(blockName);
	(*m_pDtnDataStore)[objName] = codingMetadata->parentObjectFileSize;

	//TODO clean up NC blocks from my datastore
	//

        NotifySubscriber(objName, codingMetadata->parentObjectFileSize);	

        PrintTime(m_node);
        std::cout<<"node "<<m_node->nodeIndex+1<<": FULL OBJECT "<<objName<<" has been reconstructed"<<std::endl;                		

        MESSAGE_Free(m_node, msg);
    	return true;
    }


    case MSG_ROUTING_ICAN_DTNMANAGER_FRAGMENTEDOBJECTCONSTRUCTED:{
        Fragmentation_t* fragmentMetadata = (Fragmentation_t*) MESSAGE_ReturnInfo(msg);

        //update datastore
	std::string fragmentName(fragmentMetadata->getName());
        std::string objName = GetPrefixFromName(fragmentName);
        (*m_pDtnDataStore)[objName] = fragmentMetadata->sizeBytesParentObject;

        //cleanup fragments
        m_fragmentation->cleanupFragmentsInDataStore(objName, fragmentMetadata->sizeBytesParentObject);
        
        NotifySubscriber(objName, fragmentMetadata->sizeBytesParentObject);	
       
        PrintTime(m_node);
        std::cout<<"node "<<m_node->nodeIndex+1<<": FULL OBJECT "<<objName<<" has been reconstructed"<<std::endl;        
        
        //TODO neighbor interest matching? probably need this for NC..
                
        MESSAGE_Free(m_node, msg);
        return true;        
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_FRAGMENTRECONSTRUCTED:{
        Fragmentation_t* fragmentMetadata = (Fragmentation_t*) MESSAGE_ReturnInfo(msg);

        std::string fragmentName = fragmentMetadata->getName();
        std::cout<<"node "<<m_node->nodeIndex+1<<": "<<fragmentName<<" has been reconstructed"<<std::endl;

        std::string objName = GetPrefixFromName(fragmentName);

        if(m_pDtnDataStore->find(objName)==m_pDtnDataStore->end()){
            //add the fragment id to datastore only when the full object is not found in datastore            
            (*m_pDtnDataStore)[fragmentName] = fragmentMetadata->sizeBytesParentObject; //NOTE: must store parent object size for fragment
        }

        (*m_pFileSizeBuffer)[objName] = fragmentMetadata->sizeBytesParentObject;

        //pass this fragment to fragmentation
        m_fragmentation->receiveFragment(*fragmentMetadata);

        if(m_pHandshakeDataWaitingState->find(fragmentName)!=m_pHandshakeDataWaitingState->end()){            
            //send ACK only if I'm waiting for this data...       
            //remove data waiting flag
            m_pHandshakeDataWaitingState->erase(fragmentName);


            IcanDtnAckInfo ackinfo(fragmentName, false);
            SetIcanEvent(m_node, 1*MICRO_SECOND, MSG_ROUTING_ICAN_DTNMANAGER_SENDACK, sizeof(ackinfo), &ackinfo);
        }

        //TODO: neighbor discovery
        std::vector<Node*> neighborList = GetMyNeighborList(m_node, m_txrange);
        for(vector<Node* >::iterator it = neighborList.begin();it != neighborList.end();it++){
            Node* neighbor = (*it);
            if(m_node->nodeIndex - neighbor->nodeIndex == 0){
                continue;
            }        
            
            
            RequestMatchingByObject(neighbor->nodeIndex,  fragmentName);
        }
        
        MESSAGE_Free(m_node, msg);
        return true;        
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_FRAGMENTHANDSHAKESTART:{
        Fragmentation_t* pMetadata = (Fragmentation_t*) MESSAGE_ReturnInfo(msg);

        ObjectBufferEntry entry(*pMetadata);
        if(m_pHandshakeObjBuffer->find(pMetadata->getBufferId())!=m_pHandshakeObjBuffer->end()){
            //means I have sent an RTS and awaiting for CTS
            //ignore this one
            std::cout<<m_node->nodeIndex+1<<": "<<pMetadata->getBufferId()<<" still in my buffer"<<std::endl;
        }
        else{
            (*m_pHandshakeObjBuffer)[pMetadata->getBufferId()] = entry;


            std::string metadataName = pMetadata->getName();
            SendRTS(metadataName, pMetadata->targetNodeId, false);
        }        
        MESSAGE_Free(m_node, msg);
        return true;        
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_NCBLOCKHANDSHAKESTART:{
        NetworkCoding_t* pMetadata = (NetworkCoding_t*) MESSAGE_ReturnInfo(msg);

        ObjectBufferEntry entry(*pMetadata);
        if(m_pHandshakeObjBuffer->find(pMetadata->getBufferId())!=m_pHandshakeObjBuffer->end()){
            //means I have sent an RTS and awaiting for CTS
            //ignore this one
            std::cout<<m_node->nodeIndex+1<<": "<<pMetadata->getBufferId()<<" still in my buffer"<<std::endl;
        }
        else{
            (*m_pHandshakeObjBuffer)[pMetadata->getBufferId()] = entry;

            std::string metadataName = pMetadata->getName();

            SendRTS(metadataName, pMetadata->targetNodeId, true);
        }        


        MESSAGE_Free(m_node, msg);
        return true;        
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_SENDNEIGHBORINTEREST:{
        unsigned* nodeIndex = (unsigned*) MESSAGE_ReturnInfo(msg);
        
        SendNeighborInterestPacket(*nodeIndex);
        
        MESSAGE_Free(m_node, msg);
        return true;        
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_QUEUEINTEREST:{
        unsigned* nodeIndex = (unsigned*) MESSAGE_ReturnInfo(msg);

        m_objectQueue->InsertInterestDataObjectToQueue(*nodeIndex);
                
        MESSAGE_Free(m_node, msg);
        return true;        
    }
        
    case MSG_ROUTING_ICAN_DTNMANAGER_PUBLISH:
    {
        IcanDtnAppTimerInfo* sInfo = (IcanDtnAppTimerInfo*) MESSAGE_ReturnInfo(msg);
        if(!sInfo)
        {
            char errorString[MAX_STRING_LENGTH];
            sprintf(errorString, "%s, line %d: MSG_ROUTING_ICAN_DTNMANAGER_PUBLISH returns NULL info", __FILE__, __LINE__);
            ERROR_ReportError(errorString);
        }
        else
        {
            //Publish object to data store
            std::string szObjectName(sInfo->namePrefix);
            int objectSize = sInfo->size;

            (*m_pDtnDataStore)[szObjectName] = objectSize;

            //store to file size buffer
            (*m_pFileSizeBuffer)[szObjectName] =objectSize;

			//pass to networkcoding
			m_networkcoding->RegisterObject(szObjectName);

#ifdef DEBUG_ICAN_DTN
            PrintTime(m_node);
            std::cout<<"node "<<m_node->nodeIndex+1<<" published ";

            std::map<std::string, int>::const_iterator itr;
            for(itr = (*m_pDtnDataStore).begin(); itr != (*m_pDtnDataStore).end(); ++itr){
                cout << "Object: " << (*itr).first << " size- " << (*itr).second<<"     ";
            }
            std::cout<<std::endl;                
#endif
        }

        MESSAGE_Free(m_node, msg);
        return true;
    }
     
    case MSG_ROUTING_ICAN_DTNMANAGER_SUBSCRIBE:
    {
        IcanDtnAppTimerInfo* sInfo = (IcanDtnAppTimerInfo*) MESSAGE_ReturnInfo(msg);
        if(!sInfo)
        {
            char errorString[MAX_STRING_LENGTH];
            sprintf(errorString, "%s, line %d: MSG_ROUTING_ICAN_DTNMANAGER_SUBSCRIBE returns NULL info", __FILE__, __LINE__);
            ERROR_ReportError(errorString);
        }
        else
        {
            //register interest
            std::string szObjectName(sInfo->namePrefix);            
            m_pInterestRegistry->RegisterInterest(szObjectName);
        }

        MESSAGE_Free(m_node, msg);
        return true;
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_UNSUBSCRIBE:
    {
        IcanDtnAppTimerInfo* sInfo = (IcanDtnAppTimerInfo*) MESSAGE_ReturnInfo(msg);
        if(!sInfo)
        {
            char errorString[MAX_STRING_LENGTH];
            sprintf(errorString, "%s, line %d: MSG_ROUTING_ICAN_DTNMANAGER_UNSUBSCRIBE returns NULL info", __FILE__, __LINE__);
            ERROR_ReportError(errorString);
        }
        else
        {
            //unregister interest
            std::string szObjectName(sInfo->namePrefix);            
            m_pInterestRegistry->UnregisterInterest(szObjectName);
        }

        MESSAGE_Free(m_node, msg);
        return true;
    }
    
    case MSG_ROUTING_ICAN_DTNMANAGER_SENDDTNINTEREST:        //send this node's interest and cache summary
    {        

        //generate interest packet
        Message* dtnInterestPkt = m_pInterestRegistry->MakeDtnInterest();        

        //send packet
        if(dtnInterestPkt!=NULL){ //do not send interest if I don't want anything
            m_stat.totalInterestOut++;
            m_stat.totalInterestBytesOut+=MESSAGE_ReturnPacketSize(dtnInterestPkt);

            clocktype jitter = GenerateRandomDelay(m_node, m_nodeDescriptionJitter, m_seed);
            
            SendIcanPacket(m_node, jitter, dtnInterestPkt);
        }        

        //schedule next node description
        SetIcanEvent(m_node, m_InterestInterval,  MSG_ROUTING_ICAN_DTNMANAGER_SENDDTNINTEREST, 0, NULL);        

        MESSAGE_Free(m_node, msg);
        return true;
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_SENDDTNCACHESUMMARY:{
        //generate cache summary packet
        Message* cacheSummaryPkt = m_pCacheSummaryGen->MakeDtnCacheSummary();
        if(cacheSummaryPkt==NULL){
           ReportError("cache summary should not be null");
        }

        m_stat.totalCacheSummaryOut++;
        m_stat.totalCacheSummaryBytesOut += MESSAGE_ReturnPacketSize(cacheSummaryPkt);        

        clocktype jitter = GenerateRandomDelay(m_node, m_nodeDescriptionJitter, m_seed);

        SendIcanPacket(m_node, jitter, cacheSummaryPkt);

        SetIcanEvent(m_node, m_cacheSummaryInterval,  MSG_ROUTING_ICAN_DTNMANAGER_SENDDTNCACHESUMMARY, 0, NULL);        

        MESSAGE_Free(m_node, msg);
        return true;
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_SENDDTNREQUEST:{
        Message* dtnRequestPkt = m_pDataRequestGenerator->MakeDtnRequest();
        if(dtnRequestPkt!=NULL){
            m_stat.totalRequestOut++;
            m_stat.totalRequestBytesOut+=MESSAGE_ReturnPacketSize(dtnRequestPkt);

            clocktype jitter = GenerateRandomDelay(m_node, m_nodeDescriptionJitter, m_seed);
            
            SendIcanPacket(m_node, jitter, dtnRequestPkt);
        }

        SetIcanEvent(m_node, m_RequestInterval,  MSG_ROUTING_ICAN_DTNMANAGER_SENDDTNREQUEST, 0, NULL);        

        MESSAGE_Free(m_node, msg);
        return true;
    }
    
    case MSG_ROUTING_ICAN_DTNMANAGER_INTERESTMATCHING:{
        unsigned* node = (unsigned*) MESSAGE_ReturnInfo(msg);
        RequestMatching(*node);
        
        MESSAGE_Free(m_node, msg);
        return true;        
    }

    case MSG_ROUTING_ICAN_DTNMANAGER_RTSTIMEOUT:{
        DTNRTSPacket* rtspkt = (DTNRTSPacket*) MESSAGE_ReturnInfo(msg);        

        //recover buffer index
        std::string objId(rtspkt->objectName);
        objId = objId + "/" + IntToString(rtspkt->destName);
        
        //remove from buffer
        if(m_pHandshakeObjBuffer->find(objId)!= m_pHandshakeObjBuffer->end()){
            PrintTime(m_node);
            std::cout<<m_node->nodeIndex+1<<": RTS TIMEOUT: "<<objId<<std::endl;

            if((*m_pHandshakeObjBuffer)[objId].getRetry()<m_maxRtsRetry){
                std::cout<<"resending RTS"<<std::endl;
                rtspkt->PrintPacket();
               (*m_pHandshakeObjBuffer)[objId].incrementRetry();
               Message* rtsmsg = GeneratePacket(m_node, rtspkt, tyDtnRts, 0);
               SendIcanPacket(m_node, 0, rtsmsg);

               m_stat.totalRtsOut++;
               m_stat.totalRtsBytesOut+=MESSAGE_ReturnPacketSize(rtsmsg);    

               //set up RTS expire timer
               SetIcanEvent(m_node, m_rtsExpireTime, MSG_ROUTING_ICAN_DTNMANAGER_RTSTIMEOUT, sizeof(rtspkt), &rtspkt);   

            }
            else{
                m_pHandshakeObjBuffer->erase(objId);
            }
        }
        
        MESSAGE_Free(m_node, msg);
        return true;        
    }
    case MSG_ROUTING_ICAN_DTNMANAGER_CTSTIMEOUT:{
        std::string* objId = (std::string*) MESSAGE_ReturnInfo(msg);
        
        //remove from waiting state
        if(m_pHandshakeDataWaitingState->find(*objId)!= m_pHandshakeDataWaitingState->end()){
            m_pHandshakeDataWaitingState->erase(*objId);
        }
        
        MESSAGE_Free(m_node, msg);
        return true;        
    }
    case MSG_ROUTING_ICAN_DTNMANAGER_DATASENDTIMEOUT:{
        std::string* objId = (std::string*) MESSAGE_ReturnInfo(msg);           
        
        //remove from ack waiting state
        if(m_pHandshakeAckWaitingState->find(*objId)!= m_pHandshakeAckWaitingState->end()){

            unsigned dest = (*m_pHandshakeAckWaitingState)[*objId];
            std::string object = GetPrefixFromName(*objId);
            std::cout<<m_node->nodeIndex+1<<": data "<<*objId<<" to "<<dest<<" has lost"<<std::endl;

            //TODO fix this: efficiency issue, always reschedule a data

            size_t objSize = (*m_pDtnDataStore)[*objId];
            
            if(!m_enableNc){                    
                if(!m_fragmentation->isDownloadCompleted(object, dest, objSize)){
                    //PrintTime(m_node);
                    Fragmentation_t fragMetadata =
                    m_fragmentation->createFragmentFromObject(object, dest, objSize);                        
    
                    //push to queue                    
                    this->m_objectQueue->InsertFragmentToQueue(fragMetadata);
                }                    
            }

            else{
                //Check if we still need to send NC blocks? check nullspace vector...etc
               NetworkCoding_t ncMetadata = m_networkcoding->createCodedBlockFromObject(object, dest, objSize);
               //ncMetadata.Print();
               if(!ncMetadata.isEmpty()) {
                   //push to queue
                   this->m_objectQueue->InsertNcblockToQueue(ncMetadata);
               }
            }

            
            m_pHandshakeAckWaitingState->erase(*objId);
        }
        
        MESSAGE_Free(m_node, msg);
        return true;        
    }
  

    default:
    {

        bool freedByOthers = m_pInterestManager->EventHandler(msg) || m_fragmentPacketUtil->EventHandler(m_node, msg) || m_objectQueue->EventHandler(msg)
        	|| m_ncPacketUtil->EventHandler(m_node, msg);

        //EventHandler hook goes here
        //e.g.
        //freedByOthers = m_ncManager->EventHandler(msg) || m_fragManager->EventHandler(msg);
        //whichever class handled this event must do MESSAGE_Free
        
        return freedByOthers;
    }
    
    }

    return false;
}

void CIcanDtnManager::SendNeighborInterestPacket(unsigned neighborNodeIndex){
    //TODO stat

    bloom_filter nodeBf = m_pInterestManager->GetInterest(neighborNodeIndex);
    
    //create interest
    DtnInterestPacket dtnPkt;

    dtnPkt.srcName = m_node->nodeIndex;
    dtnPkt.interestSrcName = neighborNodeIndex;
    dtnPkt.interestBf = nodeBf;
 
    packetType pType = tyDtnInterest;
    
    //make Qualnet packet
    Message* dtnInterestPkt = GeneratePacketWithRawData(m_node, &dtnPkt, pType, 0, nodeBf.GetRawTableSize(), nodeBf.table()); 	//virtual payload size 0

    SendIcanPacket(m_node, 0, dtnInterestPkt);
}

void CIcanDtnManager::RequestMatching(unsigned nodeIndex){
    //std::cout<<m_node->nodeIndex+1<<": request matching for neighbor "<<nodeIndex+1<<std::endl;
    
    bool hasCacheSummary = false;
    
    bloom_filter cacheSummary;
    if(m_pCacheSummaryStore->HasCacheSummary(nodeIndex)){
        cacheSummary = m_pCacheSummaryStore->GetCacheSummary(nodeIndex);
        hasCacheSummary = true;
    }

    if(!m_pDataRequestManager->HasRequest(nodeIndex)){
        //std::cout<<"don't have request from this neighbor"<<std::endl;
        return;
    }
    
    bloom_filter request = m_pDataRequestManager->GetRequest(nodeIndex);

    //matched data object name list
    std::map<std::string, size_t> matchedObject;

    if(m_pDtnDataStore){
        for(DtnDataStore::const_iterator itr = (*m_pDtnDataStore).begin(); itr != (*m_pDtnDataStore).end(); ++itr){
            //if the name is a fragment/nc block, match prefix            
            std::string datastoreObjName = (*itr).first;
            std::string objectName = datastoreObjName;

            if(!IsFullDataObjectName(datastoreObjName)){
                objectName = GetPrefixFromName(objectName);
            }

            if(matchedObject.find(objectName)==matchedObject.end())
                matchedObject[objectName] = (*itr).second;
        }

        for(std::map<std::string, size_t>::const_iterator itr = matchedObject.begin();itr!=matchedObject.end();++itr){            
            std::string objectName = (*itr).first;                    
            size_t objSize = (*itr).second;
            
            if(request.contains(objectName)){               
                if(!cacheSummary.contains(objectName) || !hasCacheSummary){                    
                    //found match                    
//                    std::cout << "node "<<m_node->nodeIndex+1<<" found match: "<<objectName<<" size "<<objSize<<std::endl;

                    if(!m_enableNc){                    
                        if(!m_fragmentation->isDownloadCompleted(objectName, nodeIndex, objSize)){
                            //PrintTime(m_node);
                            Fragmentation_t fragMetadata =
                        		m_fragmentation->createFragmentFromObject(objectName, nodeIndex, objSize);                        
    
                            //push to queue                    
                            this->m_objectQueue->InsertFragmentToQueue(fragMetadata);
                        }                    
                    }

                    else{
                        //Check if we still need to send NC blocks? check nullspace vector...etc
                        NetworkCoding_t ncMetadata = m_networkcoding->createCodedBlockFromObject(objectName, nodeIndex, objSize);
//                        printf("nodeid=%d\n",this->m_node->nodeId);
                        ncMetadata.Print();
                        if(!ncMetadata.isEmpty()) {
//                            std::cout<<"push to queue"<<std::endl;
                            //push to queue
                            this->m_objectQueue->InsertNcblockToQueue(ncMetadata);
                        }
                    }
                }
            }
        }        
    }
}

void CIcanDtnManager::RequestMatchingByObject(unsigned nodeIndex, std::string objName){   
//    std::cout<<"request matching by object for neighbor "<<nodeIndex+1<<std::endl;
    
    bool hasCacheSummary = false;    
    if(!m_pDataRequestManager->HasRequest(nodeIndex)){
        //std::cout<<"don't have request from this neighbor"<<std::endl;
        return;
    }
    bloom_filter request = m_pDataRequestManager->GetRequest(nodeIndex);                
    
    size_t objSize = (*m_pDtnDataStore)[objName];

    if(!IsFullDataObjectName(objName)){
        objName = GetPrefixFromName(objName);
    }
            
    if(request.contains(objName)){               
        if(!m_pCacheSummaryStore->HasObject(objName, nodeIndex)){                    
            //found match                    
            //std::cout << "node "<<m_node->nodeIndex+1<<" found match: "<<objName<<" size "<<objSize<<std::endl;					
            if(!m_enableNc){                   
                if(!m_fragmentation->isDownloadCompleted(objName, nodeIndex, objSize)){
                    //PrintTime(m_node);
                    Fragmentation_t fragMetadata =
                        		m_fragmentation->createFragmentFromObject(objName, nodeIndex, objSize);                        
    
                    //push to queue                    
                    this->m_objectQueue->InsertFragmentToQueue(fragMetadata);
                }                    
            }

            else{
                NetworkCoding_t ncMetadata = m_networkcoding->createCodedBlockFromObject(objName, nodeIndex, objSize);
//                printf("nodeid=%d\n",this->m_node->nodeId);
//                ncMetadata.Print();
                if(!ncMetadata.isEmpty()){
                    //std::cout<<"push to queue"<<std::endl;
                    //push to queue
                    this->m_objectQueue->InsertNcblockToQueue(ncMetadata);
                }
            }
       }
    }
}


void CIcanDtnManager::PacketHandler(Node* node, Message* msg){

#ifdef DEBUG_ICAN_DTN    
    PrintTime(m_node);
    std::cout<<"dtn manager RECEIVE packet"<<std::endl;
#endif


    IcanHeader* icanhdr = (IcanHeader*) MESSAGE_ReturnPacket(msg);
    if(IsDtnPacket(icanhdr->pktType)){
        switch(icanhdr->pktType){
        case tyDtnRequest:  
        {                        
            m_stat.totalRequestIn++;
            m_stat.totalRequestBytesIn+=MESSAGE_ReturnPacketSize(msg);
            
            //request is received from a neighbor
            //recover bloomfilter from dtninterest packet
            DtnRequestPacket* interest = (DtnRequestPacket*) (icanhdr+1);
            unsigned srcNodeId = interest->srcName;

            std::cout<<m_node->nodeIndex+1 <<" receive request from "<< srcNodeId+1<<std::endl;

            bloom_filter nodeBf(interest->requestBf, (cell_type*)(interest+1));

            //save interest BF 
            m_pDataRequestManager->InsertRequest(srcNodeId, nodeBf);

    		clocktype jitter = GenerateRandomDelay(m_node, DTNINTERESTMATCHINGDELAY, m_seed);
            //interest matching
            SetIcanEvent(m_node, jitter,  MSG_ROUTING_ICAN_DTNMANAGER_INTERESTMATCHING, sizeof(srcNodeId), &srcNodeId);        
            
            //must free message after processing
            MESSAGE_Free(node, msg);            
            return;
        }
        case tyDtnInterest:{
            m_stat.totalInterestIn++;
            m_stat.totalInterestBytesIn+=MESSAGE_ReturnPacketSize(msg);
                        
            //interest is received from a neighbor
            //recover bloomfilter from dtninterest packet
            DtnInterestPacket* interest = (DtnInterestPacket*) (icanhdr+1);
            unsigned srcNodeId = interest->interestSrcName;

            bloom_filter nodeBf(interest->interestBf, (cell_type*)(interest+1));

            //save interest BF 
            m_pInterestManager->InsertInterest(srcNodeId, nodeBf);

            //NOTE: we don't do interest matching when receiving interest, instead only do it when a REQUEST is received
            //Interest is the list of subscription a node has
            //DataRequest is the list of data a node is willing to receive/forward
            
            //must free message after processing
            MESSAGE_Free(node, msg);            
            return;
        }

        case tyDtnCacheSummary: 
        {
            m_stat.totalCacheSummaryIn++;
            m_stat.totalCacheSummaryBytesIn+=MESSAGE_ReturnPacketSize(msg);
            
            //interest is received from a neighbor
            //recover bloomfilter from dtninterest packet
            DtnCacheSummaryPacket* cacheSummary = (DtnCacheSummaryPacket*) (icanhdr+1);

            unsigned srcnodeId = cacheSummary->srcName;
            
            bloom_filter nodeBf(cacheSummary->cacheBf, (cell_type*)(cacheSummary+1));
    
            //save to cache summary store
            m_pCacheSummaryStore->InsertCacheSummary(srcnodeId, nodeBf);
                      
            MESSAGE_Free(node, msg);            
            return;
        }

        case tyFragment:{
            m_stat.totalFragIn++;
            m_stat.totalFragBytesIn+=MESSAGE_ReturnPacketSize(msg);
            
            FragmentationPacket* fragPacket = (FragmentationPacket*) (icanhdr+1);

            //Only pass to fragmentpacketutil when I don't have this fragment or object in my datastore
            std::string fragName = fragPacket->GetFragmentName();
            std::string objectName = GetPrefixFromName(fragName);
            if(m_pDtnDataStore->find(fragName)==m_pDtnDataStore->end()
                && m_pDtnDataStore->find(objectName) == m_pDtnDataStore->end()){
                        
                SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTN_FRAGMENTPACKETUTIL_RECEIVEFRAGMENT, sizeof(FragmentationPacket), fragPacket);
            }
            
            MESSAGE_Free(node, msg);            
            return;
            
        }
        case tyCoding:{
            m_stat.totalNcIn++;
            m_stat.totalNcBytesIn+=MESSAGE_ReturnPacketSize(msg);
            
            NetworkCodingPacket* fragPacket = (NetworkCodingPacket*) (icanhdr+1);

            //Only pass to fragmentpacketutil when I don't have this fragment or object in my datastore
            std::string fragName = fragPacket->GetName();
            std::string objectName = GetPrefixFromName(fragName);
            if(m_pDtnDataStore->find(fragName)==m_pDtnDataStore->end()
                && m_pDtnDataStore->find(objectName) == m_pDtnDataStore->end()){
                        
                SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTN_NETWORKCODINGPACKETUTIL_RECEIVEBLOCK, sizeof(NetworkCodingPacket), fragPacket);
            }
            
            MESSAGE_Free(node, msg);            
            return;
            
        }

 
        case tyDtnRts:{
            m_stat.totalRtsIn++;
            m_stat.totalRtsBytesIn+=MESSAGE_ReturnPacketSize(msg);
            
            DTNRTSPacket* rtspkt = (DTNRTSPacket*) (icanhdr+1);
            HandleRTS(*rtspkt);
            
            MESSAGE_Free(node, msg);            
            return;
        }

        case tyDtnCts:{           
            //TODO: update neighbors' nullspace vector...?
            DTNCTSPacket* ctspkt = (DTNCTSPacket*) (icanhdr+1);

            if(ctspkt->isAccept){                            
                m_stat.totalCtsAcceptIn++;
                m_stat.totalCtsAcceptBytesIn+=MESSAGE_ReturnPacketSize(msg);
            }
            else{
                m_stat.totalCtsRejectIn++;
                m_stat.totalCtsRejectBytesIn+=MESSAGE_ReturnPacketSize(msg);
            }
            
            HandleCTS(*ctspkt);
            
            MESSAGE_Free(node, msg);            
            return;
        }

        case tyDtnAck:{
            m_stat.totalAckIn++;
            m_stat.totalAckBytesIn+=MESSAGE_ReturnPacketSize(msg);
            
            DTNACKPacket* ackpkt = (DTNACKPacket*) (icanhdr+1);
            HandleACK(*ackpkt);
            
            MESSAGE_Free(node, msg);            
            return;
        }
        
         //case(other packet type) ...
        default:{
            //DTNManager should not receive any Non-DTN packet
            MESSAGE_Free(node, msg);
            ReportError("DTN manager receives unknown DTN packet");
            return;
        }
        }      
    }       
    MESSAGE_Free(node, msg);
    ReportError("DTN manager receives unknown packet");

}

void CIcanDtnManager::SendRTS(std::string objectId, unsigned targetId, bool isNc){
//    PrintTime(m_node);
    std::cout<<"Node "<<m_node->nodeIndex+1<<" sends RTS for "<<objectId<<" to node "<<targetId+1<<std::endl;

    //Make RTS packet
    DTNRTSPacket rtspkt(m_node->nodeIndex, targetId, objectId, isNc);
    Message* rtsmsg = GeneratePacket(m_node, &rtspkt, tyDtnRts, 0);
    SendIcanPacket(m_node, 0, rtsmsg);

    m_stat.totalRtsOut++;
    m_stat.totalRtsBytesOut+=MESSAGE_ReturnPacketSize(rtsmsg);    

    //set up RTS expire timer
    SetIcanEvent(m_node, m_rtsExpireTime, MSG_ROUTING_ICAN_DTNMANAGER_RTSTIMEOUT, sizeof(rtspkt), &rtspkt);   
}

void CIcanDtnManager::SendACK(std::string objectId, bool isNc){
    std::cout<<m_node->nodeIndex+1 <<" sends  ACK for "<<objectId<<std::endl;

    bool hasObject = false;
    std::string fullObjName = GetPrefixFromName(objectId);
    if(m_pDtnDataStore->find(fullObjName)!=m_pDtnDataStore->end()){                
        hasObject = true;
    }
	    
    DTNACKPacket ackpkt(m_node->nodeIndex, objectId, isNc, hasObject);
    Message* ackmsg = GeneratePacket(m_node, &ackpkt, tyDtnAck, 0);

    m_stat.totalAckOut++;
    m_stat.totalAckBytesOut+=MESSAGE_ReturnPacketSize(ackmsg);
    
    SendIcanPacket(m_node, 0, ackmsg);

    //TODO: if I'm receiving from many sender, how do they handle this ACK? Optimization?
}

void CIcanDtnManager::HandleRTS(DTNRTSPacket rtspkt){
    //TODO raise event to notify frag/NC for purging


    std::cout<<m_node->nodeIndex+1<<" receives RTS from "<<rtspkt.srcName+1<<" targeting "<<rtspkt.destName+1<<std::endl;
    
    //inpsect destination
    if((rtspkt.destName - m_node->nodeIndex)==0){//I'm the destination, decide if I want this packet               

                std::cout<<m_node->nodeIndex+1<<" should respond"<<std::endl;

                
		std::string objName(rtspkt.objectName);

		//TODO we may want to add more info to the packets and keep track of how many data I have requested from other neighbors?! Otherwise may accept redundant.
		std::string fullObjName = GetPrefixFromName(objName);
		if(m_pDtnDataStore->find(objName)==m_pDtnDataStore->end() && m_pDtnDataStore->find(fullObjName)==m_pDtnDataStore->end()){
			if(m_pHandshakeDataWaitingState->find(objName)!=m_pHandshakeDataWaitingState->end() &&
				(*m_pHandshakeDataWaitingState)[objName] != rtspkt.srcName
			){ 
				//means I  have been waiting for this data from others
	
				//reject with reject code <noblock, noobject>
				std::cout<<m_node->nodeIndex+1<<" send CTS packet rejecting duplicates"<<std::endl;
				DTNCTSPacket ctspkt(m_node->nodeIndex, rtspkt.srcName, rtspkt.objectName, rtspkt.isNc, false, false, false, false);
				Message* ctsmsg = GeneratePacket(m_node, &ctspkt, tyDtnCts, 0);
				SendIcanPacket(m_node, 0, ctsmsg);
				return;
			}
            
            		//NC mixing reject case (check coefficients) TODO: test
            		if(!m_networkcoding->acceptBlock(rtspkt.objectName)){
                            std::cout<<m_node->nodeIndex+1<<" send CTS packet rejecting non-innovative block from "<<rtspkt.srcName<<std::endl;
                            DTNCTSPacket ctspkt(m_node->nodeIndex, rtspkt.srcName, rtspkt.objectName, rtspkt.isNc, false, false, false, true);
			    Message* ctsmsg = GeneratePacket(m_node, &ctspkt, tyDtnCts, 0);
			    SendIcanPacket(m_node, 0, ctsmsg);
			    return;
                        }

			//keep state remembering I'm waiting for this data
			(*m_pHandshakeDataWaitingState)[objName] = rtspkt.srcName;            
			SetIcanEvent(m_node, m_ctsExpireTime, MSG_ROUTING_ICAN_DTNMANAGER_CTSTIMEOUT, sizeof(objName), &objName);   
			
			//Send Accept CTS            
			std::cout<<m_node->nodeIndex+1<<"send accepting CTS"<<std::endl;
			DTNCTSPacket ctspkt(m_node->nodeIndex, rtspkt.srcName, rtspkt.objectName, rtspkt.isNc, true, false, false, false);
			Message* ctsmsg = GeneratePacket(m_node, &ctspkt, tyDtnCts, 0);

			m_stat.totalCtsAcceptOut++;
			m_stat.totalCtsAcceptBytesOut+=MESSAGE_ReturnPacketSize(ctsmsg);

			SendIcanPacket(m_node, 0, ctsmsg);
		}
		else{
			//make reject CTS
			bool hasBlock = false;
                        bool hasObject = false;
			if(m_pDtnDataStore->find(fullObjName)!=m_pDtnDataStore->end()){                
                            hasObject = true;
			}
                        if(m_pDtnDataStore->find(objName)!=m_pDtnDataStore->end()){
                            hasBlock = true;
                        }

			std::cout<<m_node->nodeIndex+1<<"send CTS packet reject code <hasblock, hasobject>"<<hasBlock<<", "<<hasObject<<std::endl;                        

			DTNCTSPacket ctspkt(m_node->nodeIndex, rtspkt.srcName, rtspkt.objectName, rtspkt.isNc, false, hasBlock, hasObject, false);
            
			Message* ctsmsg = GeneratePacket(m_node, &ctspkt, tyDtnCts, 0);

			m_stat.totalCtsRejectOut++;
			m_stat.totalCtsRejectBytesOut+=MESSAGE_ReturnPacketSize(ctsmsg);
		
			SendIcanPacket(m_node, 0, ctsmsg);
		}
	}
}
void CIcanDtnManager::HandleCTS(DTNCTSPacket ctspkt){

    //check reject code, update cache summary
    if(ctspkt.isAccept == false){
        std::string fragName(ctspkt.objectName);
        std::string fullName = GetPrefixFromName(fragName);
/*
        if(ctspkt.isRejectNcMixing){
            //notify network coding //TODO: test
            if(m_enableNc){
                m_networkcoding->rejectBlock(fullName, ctspkt.srcName);
            }
        }

  */      
        if(ctspkt.isHaveObject== true){
            //add full object to cache summary       
            m_pCacheSummaryStore->UpdateCacheSummaryWithFullName(ctspkt.srcName,  fullName);                    
        }
        else if(ctspkt.isHaveBlock == true){
            std::string fragName(ctspkt.objectName);
            m_pCacheSummaryStore->UpdateCacheSummaryWithFragmentName(ctspkt.srcName,  fragName);        
        }

    }
   
    //check destination
    if((ctspkt.destName - m_node->nodeIndex)==0){
        PrintTime(m_node);
        std::cout<<"node "<<m_node->nodeIndex+1<<" receives CTS  ";

        //recover buffer index
        std::string objName(ctspkt.objectName);
        std::string objId = objName + "/" + IntToString(ctspkt.srcName);

        //if accept, raise event to send data
        if(ctspkt.isAccept == true){
            std::cout<<"ACCEPT "<<objName<<std::endl;
			if(m_pHandshakeObjBuffer->find(objId)!= m_pHandshakeObjBuffer->end()){
				//keep state remembering I'm waiting for ACK
				(*m_pHandshakeAckWaitingState)[objName] = ctspkt.srcName;
				SetIcanEvent(m_node, m_dataSendExpireTime, MSG_ROUTING_ICAN_DTNMANAGER_DATASENDTIMEOUT, sizeof(objName), &objName);   

				if(ctspkt.isNc == false){	
					Fragmentation_t metadata = (*m_pHandshakeObjBuffer)[objId].fragmentMetadata;
					SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTN_FRAGMENTPACKETUTIL_SENDFRAGMENT, sizeof(metadata), &metadata);
				}
				else{
					NetworkCoding_t metadata = (*m_pHandshakeObjBuffer)[objId].ncblockMetadata;
					SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTN_NETWORKCODINGPACKETUTIL_SENDNCBLOCK, sizeof(metadata), &metadata);						
				}
			}
			else{
				std::cout<<"!!!!WARNING: cannot find fragment/block in buffer. expired?";
			}
			
			//remove from RTS buffer
			m_pHandshakeObjBuffer->erase(objId);
       }
        else{
			if(ctspkt.isRejectNcMixing){
				//notify network coding //TODO: test
				std::string fullName = GetPrefixFromName(objName);
				if(m_enableNc){
					m_networkcoding->rejectBlock(fullName, ctspkt.srcName);
				}
			}

 

            std::cout<<"REJECT "<<objName<<std::endl;

            //remove from RTS buffer
            m_pHandshakeObjBuffer->erase(objId); 

            std::string fragName(ctspkt.objectName);

            if(ctspkt.isHaveObject) return;
            else{
		if(ctspkt.isNc){				
	    	    RetrieveNextNCBlock(fragName, ctspkt.srcName);
		}
		else{		
	            RetrieveNextFragment(fragName, ctspkt.srcName);
		}
            }
        }
    }   
}

void CIcanDtnManager::HandleACK(DTNACKPacket ackpkt){
    std::cout<<m_node->nodeIndex+1<<" receives ACK from node "<<ackpkt.srcName +1 <<std::endl;
    ackpkt.PrintPacket();
        
        std::string fragName(ackpkt.objectName);
        //Update this node's bloomfilter
        if(ackpkt.hasFullObject){
            //update with full object name
            std::string fullName = GetPrefixFromName(fragName);
            m_pCacheSummaryStore->UpdateCacheSummaryWithFullName(ackpkt.srcName,  fullName);                    
        }
        else{
            m_pCacheSummaryStore->UpdateCacheSummaryWithFragmentName(ackpkt.srcName,  fragName);    
        }

        std::string objectName = GetPrefixFromName(fragName);        

        if(m_pHandshakeAckWaitingState->find(fragName)!=m_pHandshakeAckWaitingState->end()){        // means I'm waiting for this ACK, should keep sending new fragment/blocks            
            //remove waiting state
            m_pHandshakeAckWaitingState->erase(fragName);        

            if(!ackpkt.hasFullObject){
                //Use parent object name or fragment name to get the full object size
                if(m_enableNc){
                    RetrieveNextNCBlock(fragName, ackpkt.srcName);
                }
                else{
                    RetrieveNextFragment(fragName, ackpkt.srcName);
                }
            }
        }
}

size_t CIcanDtnManager::GetFileSizeByObjectName(std::string objectName){
     size_t objSize;
     if(m_pFileSizeBuffer->find(objectName)!=m_pFileSizeBuffer->end()){
        objSize = (*m_pFileSizeBuffer)[objectName];
    }
    else{
        ReportError("Can't find object size");
    }

    return objSize;
}

void CIcanDtnManager::RetrieveNextFragment(std::string fragName, unsigned targetNodeIndex){
    //Use parent object name or fragment name to get the full object size
    std::string objectName = GetPrefixFromName(fragName);        
    size_t objSize = GetFileSizeByObjectName(objectName);
            
    //Take next fragment
    if(!m_fragmentation->isDownloadCompleted(objectName, targetNodeIndex, objSize)){
        Fragmentation_t fragMetadata =
            m_fragmentation->createFragmentFromObject(objectName, targetNodeIndex, objSize);            
        //push to queue                    
        this->m_objectQueue->InsertFragmentToQueue(fragMetadata);
    }
}
void CIcanDtnManager::RetrieveNextNCBlock(std::string fragName, unsigned targetNodeIndex){
    //Use parent object name or fragment name to get the full object size
    //
    std::string objectName = GetPrefixFromName(fragName);        
    size_t objSize = GetFileSizeByObjectName(objectName);
            
    //Take next block
    NetworkCoding_t fragMetadata =
            m_networkcoding->createCodedBlockFromObject(objectName, targetNodeIndex, objSize);
    printf("nodeid=%d\n",this->m_node->nodeId);
    fragMetadata.Print();
    if(!fragMetadata.isEmpty()){
        //push to queue                    
        this->m_objectQueue->InsertNcblockToQueue(fragMetadata);
    }
}


void CIcanDtnManager::PrintStat(Node* node,   NetworkType networkType){

    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "Total DTN Requests sent = %d", m_stat.totalRequestOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN Requests bytes sent = %d", m_stat.totalRequestBytesOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);
    
    sprintf(buf, "Total DTN Interests sent = %d", m_stat.totalInterestOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN Interests bytes sent = %d", m_stat.totalInterestBytesOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN CacheSummary sent = %d", m_stat.totalCacheSummaryOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN CacheSummary bytes sent = %d", m_stat.totalCacheSummaryBytesOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);    



    sprintf(buf, "Total DTN RTS sent = %d", m_stat.totalRtsOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN RTS bytes sent = %d", m_stat.totalRtsBytesOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   

    sprintf(buf, "Total DTN ACCEPT sent = %d", m_stat.totalCtsAcceptOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN ACCEPT bytes sent = %d", m_stat.totalCtsAcceptBytesOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   

    sprintf(buf, "Total DTN REJECT sent = %d", m_stat.totalCtsRejectOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN REJECT bytes sent = %d", m_stat.totalCtsRejectBytesOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   

    sprintf(buf, "Total DTN ACK sent = %d", m_stat.totalAckOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN ACK bytes sent = %d", m_stat.totalAckBytesOut);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   


    sprintf(buf, "Total DTN Requests received = %d", m_stat.totalRequestIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN Request bytes received = %d", m_stat.totalRequestBytesIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN Interests received = %d", m_stat.totalInterestIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN Interests bytes received = %d", m_stat.totalInterestBytesIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN CacheSummary received = %d", m_stat.totalCacheSummaryIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN CacheSummary bytes received = %d", m_stat.totalCacheSummaryBytesIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   

    sprintf(buf, "Total DTN RTS received = %d", m_stat.totalRtsIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN RTS bytes received = %d", m_stat.totalRtsBytesIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   

    sprintf(buf, "Total DTN ACCEPT received = %d", m_stat.totalCtsAcceptIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN ACCEPT bytes received = %d", m_stat.totalCtsAcceptBytesIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   

    sprintf(buf, "Total DTN REJECT received = %d", m_stat.totalCtsRejectIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN REJECT bytes received = %d", m_stat.totalCtsRejectBytesIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   

    sprintf(buf, "Total DTN ACK received = %d", m_stat.totalAckIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN ACK bytes received = %d", m_stat.totalAckBytesIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   

    sprintf(buf, "Total DTN FragmentPacket received = %d", m_stat.totalFragIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN FragmentPacket bytes received = %d", m_stat.totalFragBytesIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   

    sprintf(buf, "Total DTN NetworkCodingPacket received = %d", m_stat.totalNcIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DTN NetworkCodingPacket bytes received = %d", m_stat.totalNcBytesIn);
    IO_PrintStat(node, "Network", "ICAN", ANY_DEST, 0, buf);   


    m_fragmentation->Printstat(node, networkType);
    m_fragmentPacketUtil->Printstat(node, networkType);

    m_networkcoding->Printstat(node, networkType);
    m_ncPacketUtil->Printstat(node, networkType);
}

