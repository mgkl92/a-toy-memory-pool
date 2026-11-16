#pragma once

#include "Common.hpp"
#include <map>
#include <mutex>

namespace mgkl {
    class PageCache {
    public:
        const static size_t PAGE_SIZE = 4096;

        static PageCache & getInstance() {
            static PageCache instance;
            return instance;
        }

        void * allocateSpan(size_t npages);

        void deallocateSpan(void * p, size_t npages);

    private:
        PageCache() = default;

        void * systemAlloc(size_t npages);

    private:
        struct Span {
            void * pageAddr;
            size_t numPages;
            Span * next;
        };

        std::map<size_t, Span *> freeSpans_;

        std::map<void *, Span *> spanMap_;
        std::mutex mutex_;
    };
} // namespace mgkl