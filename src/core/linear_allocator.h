#pragma once

#include <memory>
namespace rt {
    class LinearAllocator {
        std::byte* buffer;
        std::byte* bump_ptr;
        std::size_t capacity;

    public :
        explicit LinearAllocator(std::size_t capacity_bytes) : buffer(new std::byte[capacity_bytes]), bump_ptr(buffer), capacity(capacity_bytes) {}
        ~LinearAllocator() {
            delete[] buffer;
        }

        void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) {
            intptr_t current_addr = (intptr_t)bump_ptr;
            intptr_t aligned_addr = (current_addr + alignment - 1) & ~(alignment - 1);
            if (aligned_addr + size > (intptr_t)(buffer+capacity)) throw std::bad_alloc{};
            void* ptr = (void*)aligned_addr;
            bump_ptr = (std::byte*)(aligned_addr + size);
            return ptr;
        }

        void reset() {
            bump_ptr = buffer;
        }

        std::size_t used() const {
            return bump_ptr - buffer;
        }

        std::size_t total() const {
            return capacity;
        }

        std::size_t buffer_() const {
            return (std::size_t)buffer;
        }

        void restore(std::size_t offset) {
            bump_ptr = buffer + offset;
        }
    };
}
