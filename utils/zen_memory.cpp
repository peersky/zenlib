
// leaf_mempoool heavily modified by Tim pechersky
/** mpool source significantly modified by Mike Mulshine, Jeff Snyder, et al., Princeton University Music Department **/

/**
 In short, mpool is distributed under so called "BSD license",

 Copyright (c) 2009-2010 Tatsuhiko Kubo <cubicdaiya@gmail.com>
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 * Neither the name of the authors nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* written with C99 style */



#include "zen_memory.h"
//#include "zen_debug.h"
//#include <stdlib.h>

/**
 * private function
 */



/**
 * allocate memory from memory pool
 */
char* zen::Mempool::alloc(size_t asize)
{
#if LEAF_DEBUG
    DBG("alloc " + String(asize));
#endif
#if ZEN_USE_DYNAMIC_ALLOCATION
    char* temp = (char*) malloc(asize);
    if (temp == NULL)
    {
        // allocation failed, exit from the program
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    if (clearOnAllocation > 0)
    {
        memset(temp, 0, asize);
    }
    return temp;
#else
    // If the head is NULL, the mempool is full
    if (head_ == NULL)
    {
        if ((msize_ - usize_) > asize)
        {
//			ZEN_ERROR_HANDLER();
        }
        else
        {
//			ZEN_ERROR_HANDLER();
        }
        return NULL;
    }

    // Should we alloc the first block large enough or check all blocks and pick the one closest in size?
	size_t size_to_alloc = align(asize);
	MemoryNode* node_to_alloc = head_;

    // Traverse the free list for a large enough block
    while (node_to_alloc->size < size_to_alloc)
    {
        node_to_alloc = node_to_alloc->next;

        // If we reach the end of the free list, there
        // are no blocks large enough, return NULL
        if (node_to_alloc == NULL)
        {
            if ((msize_ - usize_) > asize)
            {
//				ZEN_ERROR_HANDLER();
            }
            else
            {
//				ZEN_ERROR_HANDLER();
            }
            return NULL;
        }
    }

    // Create a new node after the node to be allocated if there is enough space
    MemoryNode* new_node;
    size_t leftover = node_to_alloc->size - size_to_alloc;
    node_to_alloc->size = size_to_alloc;
    if (leftover > getHeaderSize())
    {
        long offset = (char*) node_to_alloc - (char*) mpool_;
        offset += getHeaderSize() + node_to_alloc->size;
        new_node = create_node(&mpool_[offset],
                               node_to_alloc->next,
                               node_to_alloc->prev,
                               leftover - getHeaderSize(), getHeaderSize());
    }
    else
    {
        // Add any leftover space to the allocated node to avoid fragmentation
        node_to_alloc->size += leftover;

        new_node = node_to_alloc->next;
    }

    // Update the head if we are allocating the first node of the free list
    // The head will be NULL if there is no space left
    if (head_ == node_to_alloc)
    {
        head_ = new_node;
    }

    // Remove the allocated node from the free list
    delink_node(node_to_alloc);

    usize_ += getHeaderSize() + node_to_alloc->size;

    if (clearOnAllocation_)
    {
        char* new_pool = (char*)node_to_alloc->pool;
        for (size_t i = 0; i < node_to_alloc->size; i++) new_pool[i] = 0;
    }

    // Return the pool of the allocated node;
    return node_to_alloc->pool;
#endif
}


/**
 * allocate memory from memory pool and also clear that memory to be blank
 */
char* zen::Mempool::calloc(size_t asize)
{
#if LEAF_DEBUG
    DBG("calloc " + String(asize));
#endif
#if ZEN_USE_DYNAMIC_ALLOCATION
    char* ret = (char*) malloc(asize);
    if (ret == NULL)
    {
        // allocation failed, exit from the program
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    memset(ret, 0, asize);
    return ret;
#else
    // If the head is NULL, the mempool is full
    if (head_ == NULL)
    {
        if ((msize_ - usize_) > asize)
        {
//			ZEN_ERROR_HANDLER();
        }
        else
        {
//			ZEN_ERROR_HANDLER();
        }
        return NULL;
    }

    // Should we alloc the first block large enough or check all blocks and pick the one closest in size?
	size_t size_to_alloc = align(asize);
	MemoryNode* node_to_alloc = head_;

    // Traverse the free list for a large enough block
    while (node_to_alloc->size < size_to_alloc)
    {
        node_to_alloc = node_to_alloc->next;

        // If we reach the end of the free list, there
        // are no blocks large enough, return NULL
        if (node_to_alloc == NULL)
        {
            if ((msize_ - usize_) > asize)
            {
//				ZEN_ERROR_HANDLER();
            }
            else
            {
//				ZEN_ERROR_HANDLER();
            }
            return NULL;
        }
    }

    // Create a new node after the node to be allocated if there is enough space
	MemoryNode* new_node;
    size_t leftover = node_to_alloc->size - size_to_alloc;
    node_to_alloc->size = size_to_alloc;
    if (leftover > getHeaderSize())
    {
        long offset = (char*) node_to_alloc - (char*) mpool_;
        offset += getHeaderSize() + node_to_alloc->size;
        new_node = create_node(&mpool_[offset],
                               node_to_alloc->next,
                               node_to_alloc->prev,
                               leftover - getHeaderSize(), getHeaderSize());
    }
    else
    {
        // Add any leftover space to the allocated node to avoid fragmentation
        node_to_alloc->size += leftover;

        new_node = node_to_alloc->next;
    }

    // Update the head if we are allocating the first node of the free list
    // The head will be NULL if there is no space left
    if (head_ == node_to_alloc)
    {
        head_ = new_node;
    }

    // Remove the allocated node from the free list
    delink_node(node_to_alloc);

    usize_ += getHeaderSize() + node_to_alloc->size;
    // Format the new pool
    for (size_t i = 0; i < node_to_alloc->size; i++) node_to_alloc->pool[i] = 0;
    // Return the pool of the allocated node;
    return node_to_alloc->pool;
#endif
}

void zen::Mempool::free(char* ptr)
{
#if LEAF_DEBUG
    DBG("free");
#endif
#if ZEN_USE_DYNAMIC_ALLOCATION
    free(ptr);
#else
    //if (ptr < pool->mpool || ptr >= pool->mpool + pool->msize)
    // Get the node at the freed space
	MemoryNode* freed_node = (MemoryNode*) (ptr - getHeaderSize());

    usize_ -= getHeaderSize() + freed_node->size;

    // Check each node in the list against the newly freed one to see if it's adjacent in memory
	MemoryNode* other_node = head_;
	MemoryNode* next_node;
    while (other_node != NULL)
    {
        if ((size_t) other_node < (size_t) mpool_ ||
            (size_t) other_node >= (((size_t) mpool_) + msize_))
        {
//			ZEN_ERROR_HANDLER();
            return;
        }
        next_node = other_node->next;
        // Check if a node is directly after the freed node
        if (((size_t) freed_node) + (getHeaderSize() + freed_node->size) == (size_t) other_node)
        {
            // Increase freed node's size
            freed_node->size += getHeaderSize() + other_node->size;
            // If we are merging with the head, move the head forward
            if (other_node == head_) head_ = head_->next;
            // Delink the merged node
            delink_node(other_node);
        }

        // Check if a node is directly before the freed node
        else if (((size_t) other_node) + (getHeaderSize() + other_node->size) == (size_t) freed_node)
        {
            // Increase the merging node's size
            other_node->size += getHeaderSize() + freed_node->size;

            if (other_node != head_)
            {
                // Delink the merging node
                delink_node(other_node);
                // Attach the merging node to the head
                other_node->next = head_;
                // Merge
                freed_node = other_node;
            }
            else
            {
                // If we are merging with the head, move the head forward
                head_ = head_->next;
                // Merge
                freed_node = other_node;
            }
        }

        other_node = next_node;
    }

    // Ensure the freed node is attached to the head
    freed_node->next = head_;
    if (head_ != NULL) head_->prev = freed_node;
    head_ = freed_node;

    // Format the freed pool
    //    char* freed_pool = (char*)freed_node->pool;
    //    for (int i = 0; i < freed_node->size; i++) freed_pool[i] = 0;
#endif
}




