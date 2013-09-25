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
#include "ican_app_dtnpublisher.h"
#include <fstream>

CIcanDtnPublisher::CIcanDtnPublisher(Node* node, std::string szConfigName):m_node(node)
{
    //parse .app
    ParseConfig(szConfigName);
}

bool CIcanDtnPublisher::EventHandler(Node* node, Message* msg)
{
    switch (MESSAGE_GetEvent(msg))
    {

    case MSG_ROUTING_ICAN_DTNPUBLISHER_PUBLISH:
    {

        IcanDtnAppTimerInfo* sInfo = (IcanDtnAppTimerInfo*) MESSAGE_ReturnInfo(msg);
        if(!sInfo)
        {
            char errorString[MAX_STRING_LENGTH];
            sprintf(errorString, "%s, line %d: MSG_ROUTING_ICAN_DTNPUBLISHER_PUBLISH returns NULL info", __FILE__, __LINE__);
            ERROR_ReportError(errorString);
        }
        else
        {
            //publish object
            SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTNMANAGER_PUBLISH, sizeof(IcanDtnAppTimerInfo), sInfo);

            m_stat.totalDOPublished++;
        }

        MESSAGE_Free(node, msg);
        return true;
        break;
    }
    }
    return false;

}

void CIcanDtnPublisher::ParseConfig(std::string szConfigName)
{
    std::cout<<"Node "<<m_node->nodeIndex+1<<" parsing config "<<szConfigName<<std::endl;

    std::ifstream inputfile(szConfigName.c_str());
    if(inputfile)
    {
    	const int LINESIZE = 99999;
    	char line[LINESIZE];
    	memset(line,0,sizeof(char)*LINESIZE);
        while(inputfile.getline (line,LINESIZE)){
            char charObjectId[9999];
            int objectSize;
            char charPublishTime[9999];
            memset(charObjectId,0,sizeof(char)*9999);
            memset(charPublishTime,0,sizeof(char)*9999);
            sscanf (line," %s %d %s ", charObjectId, &objectSize, charPublishTime);

            std::string objectID(charObjectId);
            std::string publishTime(charPublishTime);

            std::cout<<"object ID: "<<objectID<<" "
                     <<"object size: "<<objectSize<<" "
                     <<"publishTime: "<<publishTime<<" "
                     <<std::endl;

            clocktype tPublishTime = (clocktype) TIME_ConvertToClock(publishTime.c_str());

            IcanDtnAppTimerInfo subinfo;
            strncpy(subinfo.namePrefix, objectID.c_str(), sizeof(subinfo.namePrefix));

            subinfo.size = objectSize;
            SetIcanEvent(m_node, tPublishTime, MSG_ROUTING_ICAN_DTNPUBLISHER_PUBLISH, sizeof(IcanDtnAppTimerInfo), &subinfo);

            memset(line,0,sizeof(char)*LINESIZE);
        }
    }
    else
    {
        ReportError("Publisher config error");
    }
    inputfile.close();


}

CIcanDtnPublisher::~CIcanDtnPublisher()
{
}

std::string CIcanDtnPublisher::GetPrefix()
{
    return DTNPUBLISHERID;
}

//-----------------------------------------------------------------------------
// FUNCTION    IsServer()
// PURPOSE      Decides whether this is the server app of a give name
// Parameter    std::string& szInterestName
//                      a full name
//-----------------------------------------------------------------------------
bool CIcanDtnPublisher::IsServer(const std::string& szInterestName)
{
    if(strcmp(szInterestName.c_str(), DTNPUBLISHERID)==0)
        return true;
    return false;
}


//-----------------------------------------------------------------------------
// FUNCTION     PrintDebugInfo()
// PURPOSE      print debug information
//-----------------------------------------------------------------------------
void CIcanDtnPublisher::PrintDebugInfo()
{
}

//-----------------------------------------------------------------------------
// FUNCTION     PrintStat()
// PURPOSE      print final statistics
// Parameter    Node* node
//                      pointer to node
//                    NetworkType networkType
//                      network type
//-----------------------------------------------------------------------------
void CIcanDtnPublisher::PrintStat(Node * node, NetworkType networkType)
{
    char buf[MAX_STRING_LENGTH];
    std::string AppName = "DTNPUB";

    // Print statistics
    sprintf(buf, "Total DOs published= %u", m_stat.totalDOPublished);
    IO_PrintStat(node, AppName.c_str(), "ICAN", ANY_DEST, 0, buf);
}

