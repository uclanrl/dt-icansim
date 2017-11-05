#ifndef NC3N_NETWORKCODING_H
#define NC3N_NETWORKCODING_H



#include "ican_common.h"
#include "ican_dtn_common.h"
#include "ican_dtn_cachesummarystore.h"

#include "coding/codetorrentencoder.h"
#include "coding/codetorrentdecoder.h"
#include "coding/ncutil.h"

#include <boost/shared_ptr.hpp>

//#include <boost/thread.hpp>
//#include <boost/thread/locks.hpp>
//#include <boost/thread/mutex.hpp>

//using boost::mutex;
//using boost::lock_guard;

//#include <thread>

typedef boost::shared_ptr<codetorrentencoder> encoderptr;
typedef boost::shared_ptr<codetorrentdecoder> decoderptr;
typedef boost::shared_ptr<CodedBlock> codedblockptr_t;

//keeps track of encoders
typedef std::map<std::string, encoderptr> encoder_t;

//keeps track of decoders
typedef std::map<std::string, decoderptr> decoder_t;

//object names that this node has received
typedef std::set<std::string> networkcodingobjectsreceived_t;

//maps for parent object the associated coded blocks this node has
typedef std::set<NetworkCoding_t> blocksreceived_t;
typedef std::map<std::string,blocksreceived_t> parenttoblockidsmapping_t;


//global shared
//all the coded blocks go here!
// blockid,codedblockptr
//typedef std::map<std::string,codedblockptr_t> codedblockptrmap_t;

//keep track if object has been rejected so can wait for mix
typedef std::map<std::string,bool> rejectedobject_t;

// track who published which object
typedef std::map<std::string,bool> objectpublisherregistry_t;

//#define OBJECT_BLOCK_SIZE 1024*32 //TODO: read from input

struct NetworkCodingStat
{
    unsigned totalObjectReconstructed;
    unsigned totalBlockReceived;
    unsigned totalBlockGenerated;
};

class NetworkCoding {
public:
	NetworkCoding(Node* node,CDtnCacheSummaryStore*  m_pCacheSummaryStore_,
			NetworkCodingOption m_ncOption_, size_t OBJECT_BLOCK_SIZE_,DtnDataStore* _dtnstore);
    bool EventHandler(Node* node,Message* msg);
    void Printstat(Node* node, NetworkType networkType);
    void receiveCodedBlock(NetworkCoding_t neworkcoding);
    NetworkCoding_t createCodedBlockFromObject(std::string parentObjectName,
    		unsigned targetNodeId,long fileSize);
    virtual ~NetworkCoding();
	void RegisterObject(std::string objectName);

	bool acceptBlock(std::string blockId);

	void rejectBlock(std::string parentObjectName, unsigned targetNodeId);

private:
	void trackCodedBlock(std::string parentObjectName, NetworkCoding_t networkcoding);
	NetworkCoding_t getBlockToSendTarget(std::string parentObjectName,
			unsigned targetNodeId,long parentObjectFileSize);

	void cleanUpBlocksInDataStore(std::string parentObjectName);


    NetworkCoding();
    Node* m_node;
    encoder_t encoder;
    decoder_t decoder;

    NetworkCodingStat m_stat;
    networkcodingobjectsreceived_t networkcodingobjectsreceived;
    parenttoblockidsmapping_t parenttoblockidsmapping;

    //static codedblockptrmap_t codedblockstorage;
    //boost::mutex codedblockstoragemutex;


    CDtnCacheSummaryStore* m_pCacheSummaryStore;
    DtnDataStore* dtnstore;
    NetworkCodingOption m_ncOption;

    rejectedobject_t rejectedobject;
    objectpublisherregistry_t objectpublisherregistry;

	size_t OBJECT_BLOCK_SIZE;

	unsigned thisNodeId;
	int MAXID;

	void writeFile(std::string blockStorageId,CodedBlockPtr block);
	codedblockptr_t getFile(std::string blockStorageId);
};

#endif
