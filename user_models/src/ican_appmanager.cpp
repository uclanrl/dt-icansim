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
   
#include "ican_appmanager.h"

CIcanAppManager::~CIcanAppManager()
{
}

CIcanAppManager::CIcanAppManager(Node* node, const NodeInput *nodeInput, std::string expName, DtnDataStore* dtnDataStore_, int pktSize_):
    m_node(node), m_expName(expName), m_dtnDataObjectStore(dtnDataStore_), m_nPktSize(pktSize_)
{
    //read input config    
    BOOL retVal;

    //parse ican app config
    NodeInput icanInput;
    IO_ReadCachedFile(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "ICAN-CONFIG-FILE",
        &retVal,
        &icanInput);

    icanInput.maxNumLines = 0;
    if (retVal == FALSE)
    {
        ReportError("ICAN-CONFIG-FILE is not found.");
    }
    else
    {
        ParseIcanAppConfig(node, icanInput);
    }

    //all nodes must have an instance of dtn subscriber
    std::string szNamePrefix = DTNSUBSCRIBERID;
    if(m_clients.find(szNamePrefix) == m_clients.end()){                  
        m_clients[szNamePrefix] = new CIcanDtnSubscriber(node, m_expName);                    
    }    

}

bool CIcanAppManager::HasClient(std::string szNamePrefix){
    if(m_clients.find(szNamePrefix) == m_clients.end()){   
        return false;
    }
    else
        return true;
}

bool CIcanAppManager::HasServer(std::string szName){
   for(int i=0; i<m_servers.size(); ++i)
   {
        if(m_servers[i]->IsServer(szName)){
            return true;
        }
   }
   return false;
}

std::list<std::string> CIcanAppManager::GetServerList(){
	std::list<std::string> serverList;
	for(int i=0;i<m_servers.size();++i){
		std:string szPrefix = m_servers[i]->GetPrefix();
		serverList.push_back(szPrefix);			
	}
	return serverList;
}
void CIcanAppManager::ClientPacketHandler(std::string szPrefix, Message* msg){
    if (m_clients.find(szPrefix) != m_clients.end())
    {
        MESSAGE_RemoveHeader(m_node, msg, sizeof(IcanHeader), TRACE_ICAN);
        m_clients[szPrefix]->DataPacketHandler(m_node, msg);
    }		
    else{
        ReportError("Client not found");
    }
}

bool CIcanAppManager::EventHandler(Message* msg){
    bool handled = false;

    switch (MESSAGE_GetEvent(msg))
    {
       
    default:
    {        
        //pass to dtnsubscriber and dtnpublisher
        if(m_clients.find(DTNSUBSCRIBERID) != m_clients.end()){
               handled = handled || m_clients[DTNSUBSCRIBERID]->EventHandler(m_node, msg);
        }
        if(!handled){
            for(int i=0; i<m_servers.size(); ++i)
            {
                if(m_servers[i]->IsServer(DTNPUBLISHERID)){
                    handled = handled || m_servers[i]->EventHandler(m_node, msg);    
                }
            }
        }
        return handled;
    }
    }

    return false;
}

void CIcanAppManager::PrintStat(Node *node, NetworkType networkType){
    for(IcanFtpClientMap::iterator it = m_clients.begin(); it != m_clients.end(); it++)
    {
        it->second->PrintStat(m_node, networkType);
    }

    for(IcanFtpServerVector::iterator it = m_servers.begin(); it != m_servers.end(); it++)
    {
        (*it)->PrintStat(m_node, networkType);
    }    
}

void CIcanAppManager::ParseIcanAppConfig(Node* node, NodeInput& fileInput)
{

    int clientNum=0;
    int serverNum=0;

    bool parsedDtnPublisher = false;

#ifdef DEBUG_ICAN
    printf("\nParseIcanAppConfig\n");
    printf("numLine: %d\n", fileInput.numLines);
#endif

    bool parseError = FALSE;

    for(int i=0; i<fileInput.numLines; i++)
    {

#ifdef DEBUG_ICAN
        printf("Line %d: %s\n", i, fileInput.inputStrings[i]);
#endif

        char szReadNodeId[MAX_STRING_LENGTH];
        sscanf(fileInput.inputStrings[i], "%s", szReadNodeId);
        int nReadNodeId = atoi(szReadNodeId);

        if(node->nodeIndex+1 == nReadNodeId)  //Node Id match, start parsing app info
        {
            char szNodeType[MAX_STRING_LENGTH];
            char szOpt1[MAX_STRING_LENGTH];
            char szOpt2[MAX_STRING_LENGTH];
            char szOpt3[MAX_STRING_LENGTH];
            char szOpt4[MAX_STRING_LENGTH];
            char szOpt5[MAX_STRING_LENGTH];
            char szOpt6[MAX_STRING_LENGTH];

            int numValues = sscanf(fileInput.inputStrings[i],
                                   "%s %s %s %s %s %s %s %s",
                                   szReadNodeId,
                                   szNodeType,
                                   szOpt1,
                                   szOpt2,
                                   szOpt3,
                                   szOpt4,
                                   szOpt5,
                                   szOpt6);
            if(numValues != 3 && numValues!=4  && numValues!=7 && numValues!=8)
            {
                parseError = TRUE;
                break;
            }            
            else if(strcmp(szNodeType, "DTNSUB")==0){
                 if(numValues != 3)
                {
                    parseError = TRUE;
                    break;
                }
                
                std::string szNamePrefix = DTNSUBSCRIBERID;
                if(m_clients.find(szNamePrefix) == m_clients.end()){
                    std::string szConfigName(szOpt1, strlen(szOpt1));                    
                    std::string expName(m_expName);
                    
                    m_clients[szNamePrefix] = new CIcanDtnSubscriber(node, szConfigName, expName);                    
                    clientNum++;
                }
                else{
                    ReportError("found duplicate DTNSUB");
                }
            }
             else if(strcmp(szNodeType, "DTNPUB")==0){
                 if(numValues != 3)
                {
                    parseError = TRUE;
                    break;
                }

                else{
                    if(parsedDtnPublisher)
                        ReportError("found duplicate DTNPUB");
                    else{
                    
                        serverNum++;
                        std::string szConfigName(szOpt1, strlen(szOpt1));         
                        CIcanFtpServerInterface* dataServer = new CIcanDtnPublisher(node, szConfigName);
                        m_servers.push_back(dataServer);
                        parsedDtnPublisher = true;
                    }
                }
            }
            
       }
        
        if (parseError)
        {
            char errorString[MAX_STRING_LENGTH];
            sprintf(errorString,
                    "Wrong ICAN configuration format! Acceptable format:\n"
                    "<Node> DTNSUB <subscriberconfigpath> \n"
                    "<Node> DTNPUB <publisherconfigpath> \n"
                  );
            ERROR_ReportError(errorString);
        }
    }
}


