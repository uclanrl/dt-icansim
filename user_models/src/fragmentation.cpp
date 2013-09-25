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

#include "fragmentation.h"
#include <algorithm>

//do nothing shouldn't be called
FragmentationManager::FragmentationManager():blockSize(0),m_node(NULL), m_pCacheSummaryStore(NULL),  dtnstore(NULL) {
    memset(&m_stat, 0, sizeof(m_stat));    
}   

FragmentationManager::FragmentationManager(size_t _blockSize,Node* node,
		CDtnCacheSummaryStore* m_pCacheSummaryStore_,DtnDataStore* _dtnstore)
		:blockSize(_blockSize),m_node(node),  m_pCacheSummaryStore(m_pCacheSummaryStore_),dtnstore(_dtnstore){
    memset(&m_stat, 0, sizeof(m_stat));
} 

bool FragmentationManager::EventHandler(Node* node, Message* message) {
    /*
    switch(MESSAGE_GetEvent(message)) {
    }
    */

    return false;
}

void FragmentationManager::Printstat(Node* node, NetworkType networkType) {

    char statbuf[MAX_STRING_LENGTH];
    sprintf(statbuf, "DataObject Reconstructed = %d", m_stat.totalObjectReconstructed);
    IO_PrintStat(m_node, "Fragmentation", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "Fragment blocks generated = %d", m_stat.totalBlockGenerated);
    IO_PrintStat(m_node, "Fragmentation", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "Fragment blocks received = %d", m_stat.totalBlockReceived);
    IO_PrintStat(m_node, "Fragmentation", "ICAN", ANY_DEST, 0, statbuf);
}   

void FragmentationManager::receiveFragment(Fragmentation_t fragmentation) {

        m_stat.totalBlockReceived++;
    
	std::string parentObjectName(fragmentation.parentObjectName);
	int fragmentid = fragmentation.fragmentid;
	//check if we already started receiving fragments for this object
	//check if key not in map
	if( this->fragmentsreceived.find(parentObjectName) == this->fragmentsreceived.end() ) {
		//create set and insert
		fragmentidset_t fragmentidset;
		this->fragmentsreceived[parentObjectName] = fragmentidset;

		size_t sizeBytesParentObject = fragmentation.sizeBytesParentObject;
		//round up
		//http://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
		int totalNumberOfFragments = (sizeBytesParentObject + this->blockSize - 1)/this->blockSize;
		this->fragmentstotal[parentObjectName] = totalNumberOfFragments;
	}

	//inset fragmentid into set for parentobjectname
	fragmentidset_t fragmentidset = this->fragmentsreceived[parentObjectName];
	fragmentidset.insert(fragmentid);
	this->fragmentsreceived[parentObjectName] = fragmentidset;

	//check and see if have enough fragments and reconstructed object
	fragmentidset_t fragmentidset_check = this->fragmentsreceived[parentObjectName];
	int numberFragmentsReceivedSoFar = fragmentidset_check.size();
	int requiredNumberOfFragments = this->fragmentstotal[parentObjectName];
//	printf("numberFragmentsReceivedSoFar=%d requiredNumberOfFragments=%d\n", numberFragmentsReceivedSoFar,requiredNumberOfFragments);
	if( numberFragmentsReceivedSoFar == requiredNumberOfFragments) {
                m_stat.totalObjectReconstructed++;
        	SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTNMANAGER_FRAGMENTEDOBJECTCONSTRUCTED, sizeof(Fragmentation_t), &fragmentation);
	}
}

fragmentidset_t FragmentationManager::getFragmentsToSendTarget(std::string parentObjectName, unsigned targetNodeIndex,size_t sizeBytesParentObject) {
	int requiredNumberOfFragments = (sizeBytesParentObject + this->blockSize -1)/this->blockSize;

	//printf("blocksize=%d parentObjectName=%s sizeBytesParentObject=%d requiredNumberOfFragments=%d\n",	this->blockSize,parentObjectName.c_str(),sizeBytesParentObject,requiredNumberOfFragments);

	//iterate and check which fragments havent received yet
	//check the bloom filter of the target node
	fragmentidset_t fragmentsMissing;
	for(int x=1;x<=requiredNumberOfFragments;x++) {
		char buffer [33];
		snprintf(buffer, sizeof(buffer), "%d", x);
		std::string objectName = parentObjectName+"/"+buffer;

		//make sure object is in our datastore (sender datastore)
		//NOTE: if the object is complete, only the full object name is in datastore.
		if(this->dtnstore->find(objectName)!=this->dtnstore->end()
                    || this->dtnstore->find(parentObjectName)!=this->dtnstore->end()
                ){
			//check if target cachesummary does not contain object
			const bool is_in = m_pCacheSummaryStore->HasObject(objectName, targetNodeIndex);
			if( !is_in ) {
				fragmentsMissing.insert(x);
			}
		}
	}

	return fragmentsMissing;
}

bool FragmentationManager::isDownloadCompleted(std::string parentObjectName, unsigned targetNodeIndex,size_t sizeBytesParentObject) {

	fragmentidset_t fragmentsMissing = this->getFragmentsToSendTarget(parentObjectName,targetNodeIndex,sizeBytesParentObject);

	if( fragmentsMissing.size() > 0 ) {
		//printf("download not complete for id=%s\n",parentObjectName.c_str());
		return false;
	}

	//printf("download complete for id=%s\n",parentObjectName.c_str());
	return true;
}

void FragmentationManager::cleanupFragmentsInDataStore(std::string parentObjectName,size_t sizeBytesParentObject){
        int requiredNumberOfFragments = (sizeBytesParentObject + this->blockSize - 1)/this->blockSize;
        for(int i=1;i<=requiredNumberOfFragments;i++){
            std::string fragName = parentObjectName + "/" + IntToString(i);
            dtnstore->erase(fragName);
        }
}

Fragmentation_t FragmentationManager::createFragmentFromObject(std::string parentObjectName,
		unsigned targetNodeIndex,size_t sizeBytesParentObject) {

	fragmentidset_t fragmentsMissing = this->getFragmentsToSendTarget(parentObjectName,targetNodeIndex,sizeBytesParentObject);

	//randomly choose fragment from set of fragments that target node does not have
	double r = rand() % fragmentsMissing.size();
//	std::cout<<"frag rand posiiton="<<r<<",size="<<fragmentsMissing.size()<<std::endl;
	fragmentidset_t::const_iterator it(fragmentsMissing.begin());
	advance(it,r);

	int fragmentIdToSend = (*it);
//	std::cout<<"fragmentIdToSend="<<fragmentIdToSend<<std::endl;
	Fragmentation_t frag(parentObjectName,fragmentIdToSend,sizeBytesParentObject,targetNodeIndex);

        m_stat.totalBlockGenerated++;
        
	return frag;
}



