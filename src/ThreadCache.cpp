#include "../include/ThreadCache.hpp"
#include "../include/CentralCache.hpp"
#include <cstddef>
#include <stdlib.h>

namespace mgkl {
    void * ThreadCache::allocate(size_t size) {
        if (size == 0) {
            size = ALIGNMENT;
        }

        if (size > MAX_BYTES) {
            return malloc(size);
        }

        size_t index = SizeClass::getIndex(size);

        --freeListSize_[index];

        if (void * p = freeList_[index]) {
            freeList_[index] = *reinterpret_cast<void **>(p);
            return p;
        }

        return fetchFromCentralCache(index);
    }

    void ThreadCache::deallocate(void * p, size_t size) {
        if (size > MAX_BYTES) {
            free(p);
            return;
        }

        size_t index = SizeClass::getIndex(size);

        *reinterpret_cast<void **>(p) = freeList_[index];
        freeList_[index] = p;
        ++freeListSize_[index];

        if (shouldReturnToCentralCache(index)) {
            returnToCentralCache(freeList_[index], size);
        }
    }

    bool ThreadCache::shouldReturnToCentralCache(size_t index) {
        size_t threshold = 64;

        return (freeListSize_[index] > threshold);
    }

    void * ThreadCache::fetchFromCentralCache(size_t index) {
        void * start = CentralCache::getInstance().fetchRange(index);
        if (!start) {
            return nullptr;
        }

        void * res = start;
        freeList_[index] = *reinterpret_cast<void **>(start);

        size_t batchNum = 0;
        void * cur = start;

        while (cur != nullptr) {
            ++batchNum;
            cur = *reinterpret_cast<void **>(cur);
        }

        freeListSize_[index] += batchNum;

        return res;
    }

    void ThreadCache::returnToCentralCache(void * start, size_t size) {
        size_t index = SizeClass::getIndex(size);
        size_t alingedSize = SizeClass::ceil(size);

        size_t batchNum = freeListSize_[index];

        if (batchNum <= 1) {
            return;
        }

        size_t keepNum = std::max(batchNum / 4, size_t(1));
        size_t returnNum = batchNum - keepNum;

        char * cur = static_cast<char *>(start);
        char * splitNode = cur;
        for (size_t i = 0; i < keepNum; ++i) {
            splitNode = reinterpret_cast<char *>(*reinterpret_cast<void **>(splitNode));
            if (splitNode == nullptr) {
                returnNum = batchNum - (i + 1);
                break;
            }
        }

        if (splitNode != nullptr) {
            void * nextNode = *reinterpret_cast<void **>(splitNode);
            *reinterpret_cast<void **>(splitNode) = nullptr;

            freeListSize_[index] = keepNum;

            if (returnNum > 0 && nextNode != nullptr) {
                CentralCache::getInstance().returnRange(nextNode, returnNum * alingedSize, index);
            }
        }
    }
} // namespace mgkl
