#include "networkcoding.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string.h>


//codedblockptrmap_t NetworkCoding::codedblockstorage;

void freeCodedBlock(CodedBlockPtr ptr) {
	free(ptr->coeffs);
	free(ptr->sums);
	free(ptr);
}

void NetworkCoding::writeFile(std::string blockStorageId,CodedBlockPtr block) {
    std::string filename = "block-" + blockStorageId;
    printf("writing file=%s\n",filename.c_str());
    int totalBytesWritten = 0;


    int coeffsSize = block->num_blocks_gen;

    FILE* _pnetworkCodedBlockFile = fopen(filename.c_str(), "wb");
    if (!_pnetworkCodedBlockFile) {
        printf("Unable create networkcoded block file %s\n", filename.c_str());
        ReportError("Unable create networkcoded block file\n");
        return;
    }
    totalBytesWritten = fwrite(block->coeffs, 1, coeffsSize, _pnetworkCodedBlockFile);
    totalBytesWritten += fwrite(block->sums, 1,
            OBJECT_BLOCK_SIZE, _pnetworkCodedBlockFile);

    fclose(_pnetworkCodedBlockFile);
}

codedblockptr_t NetworkCoding::getFile(std::string blockStorageId) {
    std::string filename = "block-" + blockStorageId;
    printf("getting file=%s\n",filename.c_str());

    int numBlocksPerGen = 32;
    int fileSize = numBlocksPerGen + OBJECT_BLOCK_SIZE;

    CodedBlockPtr block = (CodedBlockPtr) malloc(sizeof(CodedBlock));
    printf("alloc block\n");
    block->coeffs = (CoeffsPtr) malloc(numBlocksPerGen);
    printf("alloc coeffs\n");
    block->sums = (BlockPtr) malloc(OBJECT_BLOCK_SIZE);
    memset(block->coeffs, 0, numBlocksPerGen);
    memset(block->sums, 0, OBJECT_BLOCK_SIZE);
    printf("finish allocation\n");



    size_t bufferLen = fileSize + 1;
    unsigned char* storagePutData = (unsigned char*) malloc(
            bufferLen * sizeof(unsigned char));
    memset(storagePutData, 0, sizeof(storagePutData));
    unsigned char * storagePutDataPointer = &storagePutData[0];

    FILE* pnetworkCodedBlockFile = fopen(filename.c_str(), "rb");
    if (pnetworkCodedBlockFile == NULL) {
        printf("Unable to open block file %s\n",
        		filename.c_str());
        ReportError("Unable to open block file\n");
        //return;
    }

    fread(storagePutDataPointer, fileSize, 1,
            pnetworkCodedBlockFile);
    fclose(pnetworkCodedBlockFile);

    printf("finished reading file\n");



//    bool isInnovative = decoderref->store_block(0, storagePutDataPointer,
//            storagePutDataPointer + coeffsLen);


    block->num_blocks_gen = numBlocksPerGen;
    block->block_size = OBJECT_BLOCK_SIZE;
    //bool isHelpful = localDecoder->store_block(gen,block->coeffs,block->sums);

	memcpy(block->coeffs, storagePutDataPointer, numBlocksPerGen);
	memcpy(block->sums, storagePutDataPointer + numBlocksPerGen, OBJECT_BLOCK_SIZE);


    printf("Read block from file sucessfully\n");
    assert(block->coeffs);
    printf("assert coeffs\n");
    assert(block->sums);
    printf("assert sums\n");
    codedblockptr_t ptr(block,freeCodedBlock);
    printf("allocated ptr\n");

    if (storagePutData) {
        free(storagePutData);
        storagePutData = NULL;
        storagePutDataPointer = NULL;
    }

    return ptr;
}

NetworkCoding::NetworkCoding() {
	memset(&m_stat, 0, sizeof(m_stat));

 this->dtnstore = NULL;
 this->m_node = NULL;
 this->m_pCacheSummaryStore = NULL;
 this->OBJECT_BLOCK_SIZE = 0;
}

NetworkCoding::NetworkCoding(Node* node,
		CDtnCacheSummaryStore*  m_pCacheSummaryStore_,
		NetworkCodingOption m_ncOption_, size_t OBJECT_BLOCK_SIZE_,DtnDataStore* _dtnstore)
	:m_node(node),m_pCacheSummaryStore(m_pCacheSummaryStore_),
	 m_ncOption(m_ncOption_), OBJECT_BLOCK_SIZE(OBJECT_BLOCK_SIZE_), dtnstore(_dtnstore) {
	memset(&m_stat, 0, sizeof(m_stat));


        if(m_ncOption==nc_fullobjectonly)
            std::cout<<"NC OPTION: FULL OBJECT ONLY"<<std::endl;
        if(m_ncOption==nc_mixing)
            std::cout<<"NC OPTION: MIXING"<<std::endl;
        if(m_ncOption==nc_sourceonly)
            std::cout<<"NC OPTION: SOURCE ONLY"<<std::endl;        

        this->thisNodeId = this->m_node->nodeId+1;
        this->MAXID = 2147483647;
}

NetworkCoding::~NetworkCoding(){

}

bool NetworkCoding::EventHandler(Node* node,Message* msg) {
    return false;

}

void NetworkCoding::RegisterObject(std::string objectName){
	//receive register event

	//unsigned sourceNodeId = this->m_node->nodeId;
	this->objectpublisherregistry[objectName] = true;
	this->networkcodingobjectsreceived.insert(objectName);

	std::cout<<"register object name: "<<objectName<<std::endl;
}

void NetworkCoding::Printstat(Node* node, NetworkType networkType) {
    char statbuf[MAX_STRING_LENGTH];
    sprintf(statbuf, "DataObject Reconstructed = %d", m_stat.totalObjectReconstructed);
    IO_PrintStat(node, "NetworkCoding", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "NetworkCoding blocks generated = %d", m_stat.totalBlockGenerated);
    IO_PrintStat(node, "NetworkCoding", "ICAN", ANY_DEST, 0, statbuf);

    sprintf(statbuf, "NetworkCoding blocks received = %d", m_stat.totalBlockReceived);
    IO_PrintStat(node, "NetworkCoding", "ICAN", ANY_DEST, 0, statbuf);
}

void NetworkCoding::trackCodedBlock(std::string parentObjectName, NetworkCoding_t networkcoding) {
	blocksreceived_t blocksReceived = this->parenttoblockidsmapping[parentObjectName];
	blocksReceived.insert(networkcoding);
	this->parenttoblockidsmapping[parentObjectName] = blocksReceived;
}

void NetworkCoding::receiveCodedBlock(NetworkCoding_t networkcoding) {
	printf("received coded block=%s\n",networkcoding.getName().c_str());	

	m_stat.totalBlockReceived++;

	std::string parentObjectName = networkcoding.parentObjectName;

	printf("unreject=%s\n",parentObjectName.c_str());
	this->rejectedobject[parentObjectName] = false;


	const bool is_already_completed =
			this->networkcodingobjectsreceived.find(parentObjectName) != this->networkcodingobjectsreceived.end();
	if(is_already_completed) {
		//do nothing
		return;
	}

	trackCodedBlock(parentObjectName,networkcoding);

	//check if we already started decoding for this object
	if( this->decoder.find(parentObjectName) == this->decoder.end() ) {
		//create decoder
		std::string randomFileName = "out-"+gen_random_string(10);
		size_t blockSize= OBJECT_BLOCK_SIZE;
		long fileSize = networkcoding.parentObjectFileSize;
		decoderptr _decoder( new codetorrentdecoder(fileSize,randomFileName.c_str(),blockSize));
		this->decoder[parentObjectName] = _decoder;
	}
        
	std::string blockStorageId = networkcoding.getName();
    printf("calling getfile\n");
	codedblockptr_t block = getFile(networkcoding.getBlockIdOnly());
    printf("after getfile\n");

	//TODO read should be threadsafe
	//boost::lock_guard<boost::mutex> guard(codedblockstoragemutex);
	//codedblockptr block = codedblockstorage[blockStorageId];
	decoderptr localDecoder = this->decoder[parentObjectName];
	int gen = 0;
	bool isHelpful = localDecoder->store_block(gen,block->coeffs,block->sums);
	printf("isHelpful=%d coded block=%s isMixed=%d targetnodeid=%d\n",
			isHelpful,networkcoding.getName().c_str(),networkcoding.isMixed,
			networkcoding.targetNodeId+1);
	bool isDone = localDecoder->decode();
	if(isDone) {
        m_stat.totalObjectReconstructed++;

        //this->RegisterObject(networkcoding.parentObjectName);
        this->networkcodingobjectsreceived.insert(networkcoding.parentObjectName);

        //clean up once constructed no longer need decoder
        this->decoder.erase(parentObjectName);
        this->cleanUpBlocksInDataStore(parentObjectName);
        this->parenttoblockidsmapping.erase(parentObjectName);

        SetIcanEvent(m_node, 0, MSG_ROUTING_ICAN_DTNMANAGER_NETWORKCODINGOBJECTCONSTRUCTED, sizeof(networkcoding), &networkcoding);
	}

}

void NetworkCoding::cleanUpBlocksInDataStore(std::string parentObjectName) {
	blocksreceived_t blocksReceived = this->parenttoblockidsmapping[parentObjectName];
	blocksreceived_t::const_iterator iter;
	for(iter=blocksReceived.begin();iter!=blocksReceived.end();iter++) {
		NetworkCoding_t coding = (*iter);
        std::string blockname = coding.getName();
        this->dtnstore->erase(blockname);
	}
}

NetworkCoding_t NetworkCoding::getBlockToSendTarget(std::string parentObjectName,
		unsigned targetNodeIndex,long parentObjectFileSize) {



	//iterate and check which blocks havent received yet
	//check the bloom filter of the target node
	blocksreceived_t blocksReceived = this->parenttoblockidsmapping[parentObjectName];

	//std::random_shuffle ( blocksReceived.begin(), blocksReceived.end() );

	blocksreceived_t::const_iterator iter;
	for(iter=blocksReceived.begin();iter!=blocksReceived.end();iter++) {
		NetworkCoding_t coding = (*iter);

		std::string objectName = coding.getName();

		//check if target cachesummary does not contain object
		const bool is_in = m_pCacheSummaryStore->HasObject(objectName, targetNodeIndex);
		if( !is_in ) {
			printf("getBlockToSendTarget\n");
			coding.Print();
			return coding;
		}
	}

	NetworkCoding_t none;
	return none;
}


NetworkCoding_t NetworkCoding::createCodedBlockFromObject(std::string parentObjectName,
		unsigned targetNodeId,long parentObjectFileSize) {

	//check if we already started encoding for this object
	if( this->encoder.find(parentObjectName) == this->encoder.end() ) {
		//create encoder
		printf("creating encoder for parentObjectName=%s\n",parentObjectName.c_str());
		std::string randomFileName = "input-"+gen_random_string(256);;
		size_t blockSize= OBJECT_BLOCK_SIZE;
		gen_random(randomFileName,parentObjectFileSize);
		encoderptr _encoder( new codetorrentencoder(randomFileName.c_str(),blockSize,parentObjectFileSize));
		this->encoder[parentObjectName] = _encoder;
	}

	encoderptr localEncoder = this->encoder[parentObjectName];

	bool isMixingEnabled = false;

	const bool is_in = this->networkcodingobjectsreceived.find(parentObjectName) != this->networkcodingobjectsreceived.end();
	const bool full_cache_code = is_in && m_ncOption==nc_fullobjectonly;
	const bool mixing_full_cache_code = is_in && m_ncOption == nc_mixing;
	const bool is_in_registry = this->objectpublisherregistry.find(parentObjectName) != this->objectpublisherregistry.end();

	//full cache encoding
	if(full_cache_code || mixing_full_cache_code ) {
		//check if ok to send
		printf("full object encoding nodeid=%d\n",this->thisNodeId);
		int blockId = rand() % MAXID;
		NetworkCoding_t networkCoding(parentObjectName, parentObjectFileSize, targetNodeId,blockId,false);

		CodedBlockPtr block = localEncoder->encode();
		block->id = blockId;
		std::string blockStorageId = networkCoding.getName();
		codedblockptr_t _codedblockptr(block,freeCodedBlock);

		writeFile(networkCoding.getBlockIdOnly(),block);
		//boost::lock_guard<boost::mutex> guard(codedblockstoragemutex);
		//this->codedblockstorage[blockStorageId] = _codedblockptr;

		m_stat.totalBlockGenerated++;

		trackCodedBlock(parentObjectName,networkCoding);

		printf("full encoding nodeid=%d name=%s\n",this->thisNodeId,networkCoding.getName().c_str());
		return networkCoding;
	}
	//mixing
	else if(m_ncOption == nc_mixing){
		printf("mixing nodeid=%d\n",this->thisNodeId);

        const bool is_in_rejected = this->rejectedobject.find(parentObjectName) != this->rejectedobject.end();
        //PrintTime(NULL);
        //TODO
        if(is_in_rejected && !is_in ) {
        	bool isStopSend = this->rejectedobject[parentObjectName];
        	if(isStopSend) {
        		printf("reject stopsend=%s\n",parentObjectName.c_str());
    			NetworkCoding_t none;
    			return none;
        	}
        }


		//check if started receiving blocks and have blocks to mix
		if( this->decoder.find(parentObjectName) == this->decoder.end() ) {
			NetworkCoding_t none;
			return none;
		}
		decoderptr localDecoder = this->decoder[parentObjectName];
		CodedBlockPtr blockMix = localDecoder->mix();
		int blockId = rand() % MAXID;
		blockMix->id = blockId;
		NetworkCoding_t networkCoding(parentObjectName, parentObjectFileSize, targetNodeId,blockId,true);


		std::string blockStorageId = networkCoding.getName();
		codedblockptr_t _codedblockptr(blockMix,freeCodedBlock);

		writeFile(networkCoding.getBlockIdOnly(),blockMix);

		//boost::lock_guard<boost::mutex> guard(codedblockstoragemutex);
		//this->codedblockstorage[blockStorageId] = _codedblockptr;

		m_stat.totalBlockGenerated++;

		trackCodedBlock(parentObjectName,networkCoding);

		printf("mixing nodeid=%d name=%s\n",this->thisNodeId,networkCoding.getName().c_str());

		return networkCoding;
	}
	//forward random block not in target's bloomfilter
	else if(m_ncOption == nc_sourceonly && is_in && is_in_registry){
		printf("source only coding nodeid=%d\n",this->thisNodeId);

		//can do source coding
		printf("generaring source block for parentObjectName=%s\n",parentObjectName.c_str());
		int blockId = rand() % MAXID;
		NetworkCoding_t networkCoding(parentObjectName, parentObjectFileSize, targetNodeId,blockId,false);

		CodedBlockPtr block = localEncoder->encode();
		block->print();
		std::string blockStorageId = networkCoding.getName();
		codedblockptr_t _codedblockptr(block,freeCodedBlock);

		writeFile(networkCoding.getBlockIdOnly(),block);

		//boost::lock_guard<boost::mutex> guard(codedblockstoragemutex);
		//this->codedblockstorage[blockStorageId] = _codedblockptr;

		m_stat.totalBlockGenerated++;

		trackCodedBlock(parentObjectName,networkCoding);

		printf("source only coding nodeid=%d name=%s\n",this->thisNodeId,networkCoding.getName().c_str());

		return networkCoding;
	}
	else{

		NetworkCoding_t networkCoding = this->getBlockToSendTarget(parentObjectName,targetNodeId,parentObjectFileSize);
        networkCoding.targetNodeId = targetNodeId;

        printf("forward random  nodeid=%d block=%s\n",this->thisNodeId,networkCoding.getName().c_str());
		return networkCoding;
	}
}


bool NetworkCoding::acceptBlock(std::string blockId) {
	if(m_ncOption == nc_mixing) {
		std::string parentObjectName = GetPrefixFromName(blockId);

		const bool is_already_completed =
				this->networkcodingobjectsreceived.find(parentObjectName) != this->networkcodingobjectsreceived.end();
		if(is_already_completed) {
			printf("already completed downloading file\n");
			return false;
		}

		const bool is_in = this->decoder.find(parentObjectName) != this->decoder.end();
		if(!is_in) {
			return true;
		}

		std::string szPrefix(blockId);
	    size_t delim_pos;
	    delim_pos = szPrefix.find("/");
	    if (delim_pos != std::string::npos)
	    {
	        szPrefix = szPrefix.substr(delim_pos+1, std::string::npos);
	    }
	    else {
	    	return false;
	    }
	    printf("szPrefix=%s\n",szPrefix.c_str());
	    codedblockptr_t _codedblockptr = getFile(szPrefix);

		//boost::lock_guard<boost::mutex> guard(codedblockstoragemutex);
		//codedblockptr _codedblockptr = this->codedblockstorage[blockId];

		decoderptr localDecoder = this->decoder[parentObjectName];
		int gen = 0;
		bool isHelpful = localDecoder->check_innovative(gen,_codedblockptr->coeffs,_codedblockptr->sums);
		printf("acceptBlock=%d\n",isHelpful);
		return isHelpful;
	}

	return true;
}

void NetworkCoding::rejectBlock(std::string parentObjectName, unsigned targetNodeId) {
	printf("targeted=%d has rejected=%s\n",targetNodeId+1,parentObjectName.c_str());
	this->rejectedobject[parentObjectName] = true;
}
