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
  
#ifndef ICAN_DTN_OBJECTQUEUE_H
#define ICAN_DTN_OBJECTQUEUE_H

#include "ican_common.h"
#include "ican_dtn_common.h"

enum DtnObjectType{unknown, interest, fragment};

typedef struct ObjectMetadata{
        DtnObjectType objectType;
        unsigned interestNodeIndex;
        Fragmentation_t* fragmentMetadata;

        virtual ~ObjectMetadata(){
            if(fragmentMetadata){
                delete fragmentMetadata;
                fragmentMetadata = NULL;
            }
       }
        
        ObjectMetadata():objectType(unknown), fragmentMetadata(NULL), interestNodeIndex(999999){}

	//interest
    	ObjectMetadata(DtnObjectType objectType_, unsigned nodeIndex_):objectType(objectType_), fragmentMetadata(NULL), 
    		interestNodeIndex(nodeIndex_){}

	//frag
	ObjectMetadata(DtnObjectType objectType_, Fragmentation_t fragmentMetadata_):objectType(objectType_), interestNodeIndex(99999){
	            fragmentMetadata = new Fragmentation_t(fragmentMetadata_);        
        }

        ObjectMetadata(ObjectMetadata const & o):objectType(o.objectType), interestNodeIndex(o.interestNodeIndex){
       		if(o.fragmentMetadata!=NULL)
                    fragmentMetadata = new Fragmentation_t(*o.fragmentMetadata);
		else
	            fragmentMetadata = NULL;
       }

	virtual ObjectMetadata& operator=(ObjectMetadata const& o) {
                objectType = o.objectType;
                if(o.fragmentMetadata)
                        fragmentMetadata = new Fragmentation_t(*o.fragmentMetadata);
		else
	        	fragmentMetadata = NULL;

               interestNodeIndex = o.interestNodeIndex;
		return *this;
	}
       
        bool operator==(ObjectMetadata & n ) const {
            if( this->objectType == n.objectType &&
                this->interestNodeIndex == n.interestNodeIndex){
            
            	if(this->fragmentMetadata && n.fragmentMetadata){
            		if(this->fragmentMetadata->objectEqual(*n.fragmentMetadata))
            			return true;
            	}
			}
           return false;
        }

       bool IsEmpty(){
            if(objectType==unknown) return true;
            return false;
        }

        void Print(){            
            std::cout<<"objectType: "<<objectType;
            switch(objectType){
            case interest:{
                std::cout<<", interestNodeIndex: "<<interestNodeIndex<<std::endl;                
                break;
            }
            case fragment:{
                std::cout<<", objectName: "<<fragmentMetadata->getName()<<std::endl;                
                break;
            }                
	            }
            std::cout<<std::endl;
        }        
}ObjectMetadata;


typedef std::list<ObjectMetadata> ObjectQueue;

class CDtnObjectQueue {
public:
    CDtnObjectQueue(Node* node, const NodeInput* nodeInput, int interfaceIndex);
    ~CDtnObjectQueue();

    void InsertInterestDataObjectToQueue(unsigned nodeIndex);
    void InsertFragmentToQueue(Fragmentation_t frag);
    bool EventHandler(Message* msg);
    
    void TakeNextObject();     
    
    void PrintDebugInfo();
private:
    CDtnObjectQueue(){}
    ObjectQueue* m_objectQueue;
    Node* m_node;
    bool wasEmpty;
    clocktype m_sendRate;
};

#endif
