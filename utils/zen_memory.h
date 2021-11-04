/*
 MIT License

 Copyright (c) 2021 Tim Pechersky

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once
#include <new>

#define MPOOL_ALIGN_SIZE (8)

namespace zen
{

	class MemoryNode
	{
	public:
		MemoryNode(char *block_location, MemoryNode *next, MemoryNode *prev, size_t size, size_t headerSize)
		{
			pool = block_location + headerSize;
			next = next;
			prev = prev;
			size = size;
		}

		~MemoryNode()
		{
		}
		char *pool;				 // memory pool field
		struct MemoryNode *next; // next node pointer
		struct MemoryNode *prev; // prev node pointer
		size_t size;

	private:
	};

	class Mempool
	{
	public:
		Mempool()
		{
			mpool_ = NULL;
			usize_ = 0;
			msize_ = 0;
			head_ = NULL;
		}

		Mempool(char *memory, size_t size)
		{
			size_t headerSize = getHeaderSize();

			mpool_ = (char *)memory;
			usize_ = 0;
			msize_ = size;

			head_ = create_node(mpool_, NULL, NULL, msize_ - headerSize, headerSize);
		}

		~Mempool()
		{
		}

		char *alloc(size_t asize);
		char *calloc(size_t asize);
		void free(char *ptr);
		size_t getSize();
		size_t getUsed();

		size_t getPoolSize()
		{
			return msize_;
		}

		size_t getPoolUsed()
		{
			return usize_;
		}

	private:
		inline size_t align(size_t size)
		{
			return (size + (MPOOL_ALIGN_SIZE - 1)) & ~(MPOOL_ALIGN_SIZE - 1);
		}

		inline zen::MemoryNode *create_node(char *block_location, MemoryNode *next, MemoryNode *prev, size_t size, size_t headerSize)
		{
			zen::MemoryNode *pNode = new (block_location) MemoryNode(block_location, next, prev, size, headerSize);
			pNode->pool = block_location + headerSize;
			pNode->next = next;
			pNode->prev = prev;
			pNode->size = size;

			return pNode;
		}

		inline size_t getHeaderSize(void)
		{
			return align(sizeof(MemoryNode));
		}

		static inline void delink_node(MemoryNode *node)
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

		Mempool *pParent_; // In case memory object is stored within another memory space
		char *mpool_;	   // start of the mpool
		size_t usize_;	   // used size of the pool
		size_t msize_;	   // max size of the pool
		bool clearOnAllocation_;
		zen::MemoryNode *head_; // first node of memory pool free list
	};

} //namespace zen
