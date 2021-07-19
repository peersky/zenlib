#pragma once
	//==============================================================================

//#include "ZEN.h"
#include <new>

	//==============================================================================


#define MPOOL_ALIGN_SIZE (8)

namespace zen {

class MemoryNode
{
public:
	MemoryNode(char* block_location, MemoryNode* next, MemoryNode* prev, size_t size, size_t header_size)
	{
		pool = block_location + header_size;
		next = next;
		prev = prev;
		size = size;
		
	}
	
	~MemoryNode()
	{
	}
	char                *pool;     // memory pool field
	struct MemoryNode *next;     // next node pointer
	struct MemoryNode *prev;     // prev node pointer
	size_t size;
private:
};


class Mempool
{
public:
	Mempool()
	{
		mpool = NULL;
		usize = NULL;
		msize = NULL;
		head = NULL;
	}
	
	Mempool(char* memory, size_t size)
	{
		size_t header_size = getHeaderSize();
		
		mpool = (char*)memory;
		usize  = 0;
		msize  = size;
		
		head = create_node(mpool, NULL, NULL, msize-header_size, header_size);
// 		create_node(pool->mpool, NULL, NULL, pool->msize - header_size, header_size);
		
		/*
		 for (int i = 0; i < pool->head->size; i++)
		 {
		 memory[i+leaf.header_size]=0;
		 }
		 */
			//is zeroing out the memory necessary? This takes a long time on large pools - JS

	}
	
	~Mempool()
	{
	}
	
	
	char* alloc(size_t asize);
	char* calloc(size_t asize);
	void free(char* ptr);
	size_t getSize();
	size_t getUsed();
	
	size_t getPoolSize()
	{
		return msize;
	}
	
	size_t getPoolUsed()
	{
		return usize;
	}
	
	
	
	
	
private:

	 inline size_t align(size_t size) {
		return (size + (MPOOL_ALIGN_SIZE - 1)) & ~(MPOOL_ALIGN_SIZE - 1);
	}

	
	inline zen::MemoryNode *create_node(char* block_location, MemoryNode* next, MemoryNode* prev, size_t size, size_t header_size)
	{
		zen::MemoryNode *pNode = new (block_location) MemoryNode(block_location, next, prev, size, header_size);
		pNode->pool = block_location + header_size;
		pNode->next = next;
		pNode->prev = prev;
		pNode->size = size;
		
		return pNode;
	}
	
	inline size_t getHeaderSize (void)
	{
		return align(sizeof(MemoryNode));
	}
	
	static inline void delink_node(MemoryNode* node)
	{
			// If there is a node after the node to remove
		if (node->next != NULL)
		{
				// Close the link
			node->next->prev = node->prev;
		}
			// If there is a node before the node to remove
		if (node->prev != NULL)
		{
				// Close the link
			node->prev->next = node->next;
		}
		
		node->next = NULL;
		node->prev = NULL;
	}
	


	
	
	Mempool*			pParent;	// In case memory object is stored within another memory space
	char*				mpool;       // start of the mpool
	size_t              usize;       // used size of the pool
	size_t              msize;       // max size of the pool
	bool            	clearOnAllocation;
	zen::MemoryNode 	*head;		// first node of memory pool free list
};

} //namespace zen
