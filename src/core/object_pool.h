#pragma once

#include "core/linear_allocator.h"

namespace rt {
    template<typename T>
    class ObjectPool {
        LinearAllocator alloc;

    public:
        explicit ObjectPool(std::size_t max_objects)
            : alloc(max_objects * sizeof(T)) {}

        template<typename... Args>
        T* construct(Args&&... args) {
            void* mem = alloc.allocate(sizeof(T), alignof(T));
            return new (mem) T(std::forward<Args>(args)...);
        }

        void destroy_all() {
            for (size_t addr=0; addr<alloc.used(); addr+=sizeof(T)){
                reinterpret_cast<T*>((addr+alloc.buffer_()))->~T();
            }
            alloc.reset();
        }
    };
};
