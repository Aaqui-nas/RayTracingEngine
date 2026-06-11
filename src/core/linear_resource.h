#pragma once

#include <memory_resource>
#include "core/linear_allocator.h"

namespace rt {
    class LinearResource : public std::pmr::memory_resource {
        LinearAllocator alloc;

    protected:
        void* do_allocate(std::size_t bytes, std::size_t alignment) override {
            return alloc.allocate(bytes, alignment);
        }

        void do_deallocate(void*, std::size_t, std::size_t) override {
        }

        bool do_is_equal(const memory_resource& other) const noexcept override {
            return this == &other;
        }

    public:
        explicit LinearResource(std::size_t capacity) : alloc(capacity) {}
        void reset() { alloc.reset(); }
    };
}
