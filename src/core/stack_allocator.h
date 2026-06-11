#pragma once

#include <memory>
#include "core/linear_allocator.h"

namespace rt {
    class StackAllocator {
        LinearAllocator base;
        struct Marker { std::size_t offset; };

    public:
        StackAllocator(std::size_t capacity) : base(capacity) {}

        Marker mark() const {
            return Marker{base.used()};
        }
        void free_to(Marker m) {
            base.restore(m.offset);
        }
        void* allocate(std::size_t size, std::size_t alignment) {
            return base.allocate(size, alignment);
        }
    };
}
