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

#ifndef ICAN_APPMANAGER_H
#define ICAN_APPMANAGER_H

#include "ican_common.h"
#include "ican_app_client_interface.h"
#include "ican_app_dtnsubscriber.h"
#include "ican_app_dtnpublisher.h"
#include "ican_app_server_interface.h"
#include "limits.h"

typedef std::map<std::string, CIcanFtpClientInterface*> IcanFtpClientMap;
typedef std::vector<CIcanFtpServerInterface*> IcanFtpServerVector;

class CIcanAppManager
{
public:
	CIcanAppManager(Node* _node, const NodeInput *nodeInput, std::string expName_, DtnDataStore* dtnDataStore_, int pktSize_);
	virtual ~CIcanAppManager();
	bool HasClient(std::string s);
	bool HasServer(std::string szNamePrefix);
	bool EventHandler(Message* msg);
	void PrintStat(Node *node, NetworkType networkType);
	void ClientPacketHandler(std::string namePrefix, Message* msg);
	std::list<std::string> GetServerList();
	
private:
	CIcanAppManager(){}
	Node* m_node;
	int m_clientMaxRtxTimes;
	clocktype m_clientRtxTimeout;	

	IcanFtpClientMap m_clients;
        IcanFtpServerVector m_servers;

	std::string m_expName;		
	void ParseIcanAppConfig(Node* node, NodeInput& fileInput);

	DtnDataStore* m_dtnDataObjectStore;
	int m_nPktSize;
};

#endif

