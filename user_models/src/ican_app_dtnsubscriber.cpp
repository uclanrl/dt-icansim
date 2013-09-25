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
#include "ican_app_dtnsubscriber.h"
#include <fstream>

CIcanDtnSubscriber::~CIcanDtnSubscriber()
{
    delete m_pRegisteredObject;
}

CIcanDtnSubscriber::CIcanDtnSubscriber(Node* node, std::string szExpName):m_node(node), m_expName(szExpName)
{
    m_pRegisteredObject = new RegisteredNameSet;
    
}


CIcanDtnSubscriber::CIcanDtnSubscriber(Node* node, std::string szConfigName, std::string szExpName):m_node(node), m_expName(szExpName)
{
    m_pRegisteredObject = new RegisteredNameSet;
    //parse .app
    ParseConfig(szConfigName);
}

//-----------------------------------------------------------------------------
// FUNCTION     PrintDebugInfo()
// PURPOSE      print debug information
//-----------------------------------------------------------------------------
void CIcanDtnSubscriber::PrintDebugInfo()
{

}

//-----------------------------------------------------------------------------
// FUNCTION     EventHandler()
// PURPOSE      Handle App Client events
// PARAMETER    Node* node
//                     Message* msg
//                  The event message
//-----------------------------------------------------------------------------
bool CIcanDtnSubscriber::EventHandler(Node* node, Message* msg)
{
    switch (MESSAGE_GetEvent(msg))
    {
     
    case MSG_ROUTING_ICAN_DTNSUBSCRIBER_SUBSCRIBE:
    {
        IcanDtnAppTimerInfo* sInfo = (IcanDtnAppTimerInfo*) MESSAGE_ReturnInfo(msg);
        if(!sInfo)
        {
            char errorString[MAX_STRING_LENGTH];
            sprintf(errorString, "%s, line %d: MSG_ROUTING_ICAN_DTNSUBSCRIBER_SUBSCRIBE returns NULL info", __FILE__, __LINE__);
            ERROR_ReportError(errorString);
        }
        else
        {
#ifdef DEBUG_ICAN_APP
            PrintTime(node);
            dbgprintf("receives MSG_ROUTING_ICAN_DTNSUBSCRIBER_SUBSCRIBE");
#endif
            //register object
            SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTNMANAGER_SUBSCRIBE, sizeof(IcanDtnAppTimerInfo), sInfo);
            std::string objName(sInfo->namePrefix);
            m_pRegisteredObject->insert(objName);
            
            m_stat.totalDORegistered++;
        }

        MESSAGE_Free(node, msg);
        return true;
        break;
    }
    case MSG_ROUTING_ICAN_DTNSUBSCRIBER_RECEIVE:
    {
        IcanDtnAppTimerInfo* sInfo = (IcanDtnAppTimerInfo*) MESSAGE_ReturnInfo(msg);
        std::string receivedObjName(sInfo->namePrefix);
        
        m_stat.totalDOReceived++;

        //If I have previously registered this object, unregister this object
        if(m_pRegisteredObject->find(receivedObjName)!=m_pRegisteredObject->end()){
            //calculate delay
            clocktype startTime = m_RegisterTimeKeeper[receivedObjName];
            clocktype delay = getSimTime(node) - startTime;
            m_delayKeeper[receivedObjName] = delay;

            m_RegisterTimeKeeper.erase(receivedObjName);

            //unregister
            m_pRegisteredObject->erase(receivedObjName);                
            SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTNMANAGER_UNSUBSCRIBE, sizeof(*sInfo), sInfo);
        }
        
        MESSAGE_Free(node, msg);
        return true;
        break;
    }
    
    }    

    return false;
}

//-----------------------------------------------------------------------------
// FUNCTION     RegisterInterest()
// PURPOSE      Register Interest to DTNManager
//-----------------------------------------------------------------------------
void CIcanDtnSubscriber::RegisterInterest()
{
    //register interest
}


//-----------------------------------------------------------------------------
// FUNCTION    DataPacketHandler(Node* node, Message* msg)
// PURPOSE     Does nothing for DTN since the application does not need to handle interest transport
// PARAMETER  Node* node
//                      pointer to node
//                  Message* msg
//                      Received data packet (format: DataPktHeader|payload)
//-----------------------------------------------------------------------------
void CIcanDtnSubscriber::DataPacketHandler(Node* node, Message* msg)
{   
    MESSAGE_Free(node, msg);
}


//-----------------------------------------------------------------------------
// FUNCTION    PrintStat(Node * node, NetworkType networkType)
// PURPOSE     Print statistics
// PARAMETER  Node* node
//                      pointer to node
//                  NetworkType networkType
//                      Network type
//-----------------------------------------------------------------------------
void CIcanDtnSubscriber::PrintStat(Node * node, NetworkType networkType)
{
    //Compute mean, min, max delay
    clocktype minDelay = 999999999999999999;
    clocktype maxDelay = 0;

    clocktype totalDelay = 0;
    clocktype numDelay = 0;
    
    if(m_delayKeeper.size()>0){
        //generate delay stat file
        std::string outputFileName = m_expName + "."+ IntToString(m_node->nodeIndex+1) +".dtnsubstat";
        std::ofstream fout(outputFileName.c_str());

        for(std::map<std::string, clocktype>::const_iterator itr = m_delayKeeper.begin(); itr != m_delayKeeper.end(); ++itr){
            std::string objName = (*itr).first;
            clocktype delay = (*itr).second;

            totalDelay += delay;
            numDelay ++;
            if(minDelay > delay) minDelay = delay;
            if(maxDelay < delay) maxDelay = delay;

            char clockStr[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(delay, clockStr);
            std::string delayStr(clockStr);
            
            fout<<objName<<", "<<delayStr<<std::endl;        
        }
   
        fout.close();
    }

    
    char buf[MAX_STRING_LENGTH];
    std::string AppName = "DTNSUB";

    // Print statistics
    sprintf(buf, "Total DOs registered= %u", m_stat.totalDORegistered);
    IO_PrintStat(node, AppName.c_str(), "ICAN", ANY_DEST, 0, buf);

    sprintf(buf, "Total DOs received = %u", m_stat.totalDOReceived);
    IO_PrintStat(node, AppName.c_str(), "ICAN", ANY_DEST, 0, buf);


    if(numDelay > 0){
        clocktype meanDelay = totalDelay / numDelay;

        char clockStr[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(meanDelay, clockStr);
        sprintf(buf, "Average Delay= %s", clockStr);
        IO_PrintStat(node, AppName.c_str(), "ICAN", ANY_DEST, 0, buf);

        TIME_PrintClockInSecond(minDelay, clockStr);
        sprintf(buf, "Min Delay= %s", clockStr);
        IO_PrintStat(node, AppName.c_str(), "ICAN", ANY_DEST, 0, buf);

        TIME_PrintClockInSecond(maxDelay, clockStr);        
        sprintf(buf, "Max Delay= %s", clockStr);
        IO_PrintStat(node, AppName.c_str(), "ICAN", ANY_DEST, 0, buf);

	TIME_PrintClockInSecond(totalDelay, clockStr);
	sprintf(buf, "Total Delay= %s", clockStr);
        IO_PrintStat(node, AppName.c_str(), "ICAN", ANY_DEST, 0, buf);

        sprintf(buf, "Number Delay= %d", numDelay);
        IO_PrintStat(node, AppName.c_str(), "ICAN", ANY_DEST, 0, buf);
    }
    
}

bool CIcanDtnSubscriber::IsTransferComplete(Node* node)
{
    //Does nothing..

    return false;
}

void CIcanDtnSubscriber::ParseConfig(std::string szConfigName){
    //config format: <data_object_ID    data_object_size    subscribe_time>
    std::cout<<"Node "<<m_node->nodeIndex+1<<" parsing config "<<szConfigName<<std::endl;

    std::ifstream inputfile(szConfigName.c_str());
    if(inputfile){           
    	const int LINESIZE = 99999;
    	char line[LINESIZE];
    	memset(line,0,sizeof(char)*LINESIZE);
    	while(inputfile.getline (line,LINESIZE)){
            char charObjectId[9999];
            int objectSize;
            char charSubscribeTime[9999];
            memset(charObjectId,0,sizeof(char)*9999);
            memset(charSubscribeTime,0,sizeof(char)*9999);
            sscanf (line," %s %d %s ", charObjectId, &objectSize, charSubscribeTime);

            std::string objectID(charObjectId);
            std::string subscribeTime(charSubscribeTime);

            std::cout<<"osbject ID: "<<objectID<<" "
                <<"object size: "<<objectSize<<" "
                <<"subscribeTime: "<<subscribeTime<<" "
                <<std::endl;

            clocktype tPublishTime = (clocktype) TIME_ConvertToClock(subscribeTime.c_str());

            IcanDtnAppTimerInfo subinfo;
            strncpy(subinfo.namePrefix, objectID.c_str(), sizeof(subinfo.namePrefix));

            m_RegisterTimeKeeper[objectID] = tPublishTime;
            
            subinfo.size = objectSize;
            SetIcanEvent(m_node, tPublishTime, MSG_ROUTING_ICAN_DTNSUBSCRIBER_SUBSCRIBE, sizeof(IcanDtnAppTimerInfo), &subinfo);

            memset(line,0,sizeof(char)*LINESIZE);
        }
    }
    else{
        ReportError("Subscriber config error");
    }
    inputfile.close();    
}
