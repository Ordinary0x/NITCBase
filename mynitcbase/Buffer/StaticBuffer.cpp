#include "StaticBuffer.h"

// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];


StaticBuffer::StaticBuffer() {
  for(int blockIndex = 0,blockAllocMapSlot = 0 ; blockIndex < 4 ; blockIndex++){
    
    unsigned char block[BLOCK_SIZE];
    Disk::readBlock(block, blockIndex);
    read_count++;
    for(int slot =0; slot < BLOCK_SIZE ; slot++){
        blockAllocMap[blockAllocMapSlot] = block[slot];
        blockAllocMapSlot++;
    }
  }

  // initialise all blocks as free
  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].blockNum = -1;
    metainfo[bufferIndex].dirty = false;
    metainfo[bufferIndex].timeStamp = -1;
  }
}


// write back all modified blocks on system exit
StaticBuffer::~StaticBuffer() {
	for (int blockIndex = 0, blockAllocMapSlot = 0; blockIndex < 4; blockIndex++) {
		unsigned char buffer [BLOCK_SIZE];

		for (int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++) 
			buffer[slot] = blockAllocMap[blockAllocMapSlot];

		Disk::writeBlock(buffer, blockIndex);
    write_count++;
	}

  	// iterate through all the buffer blocks, write back blocks 
	// with metainfo as free=false,dirty=true using Disk::writeBlock()

	for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
		if (metainfo[bufferIndex].free == false 
			&& metainfo[bufferIndex].dirty == true){
			Disk::writeBlock(blocks[bufferIndex], metainfo[bufferIndex].blockNum);
      write_count++;
      }
	}
}



int StaticBuffer::getFreeBuffer(int blockNum) {
	if (blockNum < 0 || blockNum >= DISK_BLOCKS) return E_OUTOFBOUND;
	for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++)
		metainfo[bufferIndex].timeStamp++;

	int allocatedBuffer = 0;

	// TODO: iterate through all the blocks in the StaticBuffer
	// TODO: find the first free block in the buffer (check metainfo)
	// TODO: assign allocatedBuffer = index of the free block
	for (; allocatedBuffer < BUFFER_CAPACITY; allocatedBuffer++)
		if (metainfo[allocatedBuffer].free) break;

	if (allocatedBuffer == BUFFER_CAPACITY) {
		int lastTimestamp = -1, bufferNum = -1;
		for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
			if (metainfo[bufferIndex].timeStamp > lastTimestamp) {
				lastTimestamp = metainfo[bufferIndex].timeStamp;
				bufferNum = bufferIndex;
			}
		}

		allocatedBuffer = bufferNum;
		if (metainfo[allocatedBuffer].dirty == true) {
			Disk::writeBlock(StaticBuffer::blocks[allocatedBuffer], 
								metainfo[allocatedBuffer].blockNum);
      write_count++;
		}

		// return FAILURE;
	}

	metainfo[allocatedBuffer].free = false, 
	metainfo[allocatedBuffer].dirty = false,
	metainfo[allocatedBuffer].timeStamp = 0, 
	metainfo[allocatedBuffer].blockNum = blockNum;

	return allocatedBuffer;
}


/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {
  // Check if blockNum is valid (between zero and DISK_BLOCKS)
  // and return E_OUTOFBOUND if not valid.
  if(blockNum < 0 || blockNum >= DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  // find and return the bufferIndex which corresponds to blockNum (check metainfo)
  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++){
    if(metainfo[bufferIndex].free == false && metainfo[bufferIndex].blockNum == blockNum){
        return bufferIndex;
    }
  }
  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum){
    // find the buffer index corresponding to the block using getBufferNum().
    int bufferNum = getBufferNum(blockNum);
   
    if(bufferNum == E_BLOCKNOTINBUFFER){
        return E_BLOCKNOTINBUFFER;
    }
    else if(bufferNum == E_OUTOFBOUND){
        return E_OUTOFBOUND;
    }
    else{
        // set the dirty bit of that buffer to true in metainfo
        metainfo[bufferNum].dirty = true;
    }
    return SUCCESS;
   
}

int StaticBuffer::getStaticBlockType(int blockNum){
    // Check if blockNum is valid (non zero and less than number of disk blocks)
    // and return E_OUTOFBOUND if not valid.
    if(blockNum < 0 || blockNum >= DISK_BLOCKS){
        return E_OUTOFBOUND;
    }

    // Access the entry in block allocation map corresponding to the blockNum argument
    // and return the block type after type casting to integer.
    return (int)blockAllocMap[blockNum];
}