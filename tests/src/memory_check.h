#include <cstdlib>
#include <new>
#include <cstdint>
#include <unordered_map>
#include <mutex>

struct __mem_track {
    struct __data {
        size_t amount;
        bool array;
    };
    std::unordered_map<uintptr_t, __data> mem_map;
    std::recursive_mutex mtx;

    ~__mem_track() {
        std::lock_guard<std::recursive_mutex> l(mtx);
        if (mem_map.size() > 0) {
            std::printf("[MEM] Memory problem: not all freed!\n");
            for(const auto& i : mem_map) {
                std::printf("[MEM] - #%p: %zu in size, %s\n", i.first, i.second.amount, i.second.array ? "ARRAY" : "NON ARRAY");
            }
        }
    }

    void add(void* ptr, size_t len, bool is_arr) {
        std::lock_guard<std::recursive_mutex> l(mtx);
        mem_map[(uintptr_t)ptr] = {len, is_arr};
    }
    void pop(void* ptr) {
        std::lock_guard<std::recursive_mutex> l(mtx);
        auto it = mem_map.find((uintptr_t)ptr);
        if (it != mem_map.end()) mem_map.erase(it); // there may be other allocation before including this
    }

    std::unordered_map<uintptr_t, __data> copy() {
        std::lock_guard<std::recursive_mutex> l(mtx);
        std::unordered_map<uintptr_t, __data> cpy = mem_map;
        return cpy;
    }
};

static inline __mem_track _mem;
static thread_local bool _in_tracker = false;

// Standard single-object allocation
inline void* operator new(std::size_t count) {
    if (count == 0) throw std::bad_alloc();
    if (void* ptr = std::malloc(count)) {
        if (!_in_tracker) {
            _in_tracker = true;
            _mem.add(ptr, count, false);
            _in_tracker = false;
        }
        return ptr;
    }
    throw std::bad_alloc();
}

// Standard array allocation
inline void* operator new[](std::size_t count) {
    if (count == 0) throw std::bad_alloc();
    if (void* ptr = std::malloc(count)) {
        if (!_in_tracker) {
            _in_tracker = true;
            _mem.add(ptr, count, true);
            _in_tracker = false;
        }
        return ptr;
    }
    throw std::bad_alloc();
}

// Nothrow single-object allocation
inline void* operator new(std::size_t count, const std::nothrow_t& tag) noexcept {
    try {
        return ::operator new(count);
    } catch (...) {
        return nullptr;
    }
}

// Nothrow array allocation
inline void* operator new[](std::size_t count, const std::nothrow_t& tag) noexcept {
    try {
        return ::operator new[](count);
    } catch (...) {
        return nullptr;
    }
}
inline void operator delete(void* ptr) noexcept {
    if (!ptr) return;
    if (!_in_tracker) {
        _in_tracker = true;
        _mem.pop(ptr);
        _in_tracker = false;
    }
    std::free(ptr);
}

inline void operator delete[](void* ptr) noexcept {
    if (!ptr) return;
    if (!_in_tracker) {
        _in_tracker = true;
        _mem.pop(ptr);
        _in_tracker = false;
    }
    std::free(ptr);
}

// C++14 Sized Delete Overloads
inline void operator delete(void* ptr, std::size_t) noexcept { ::operator delete(ptr); }
inline void operator delete[](void* ptr, std::size_t) noexcept { ::operator delete[](ptr); }

// C++17 Aligned Delete Overloads
inline void operator delete(void* ptr, std::align_val_t) noexcept { ::operator delete(ptr); }
inline void operator delete[](void* ptr, std::align_val_t) noexcept { ::operator delete[](ptr); }
inline void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept { ::operator delete(ptr); }
inline void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept { ::operator delete[](ptr); }

auto mem_state() { return _mem.copy(); }