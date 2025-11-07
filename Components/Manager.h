//
// Created by liaohy on 11/7/25.
//

#pragma once

#include <array>
#include <cstdint>
namespace EP::Component {
template<typename T, typename indexType = uint32_t>
struct combo {
    static_assert(std::is_integral_v<indexType>, "indexType must be an integral type");
    using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
    explicit combo(indexType index = 0) : index(index) {};
    storage_t storage;
    indexType index{0};
    ~combo() = default;
};

template<typename T, uint32_t N, typename indexType = uint32_t>
class Manager {
    static_assert(N < std::numeric_limits<indexType>::max() / 2, "N is so big that maybe overflow");
public:
    static Manager& instance() {
        static Manager instance;
        return instance;
    }
    class managed_ptr {
    public:
        managed_ptr() = delete;
        managed_ptr(const managed_ptr&) = delete;
        managed_ptr& operator=(const managed_ptr&) = delete;

        managed_ptr(managed_ptr&& other) noexcept {this->ptr = other.ptr; other.ptr = nullptr;}
        managed_ptr& operator=(managed_ptr&& other) noexcept {
            if (this != &other) {
                this->reset();
                this->ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }

        T* operator->() const {return ptr;}
        indexType getIndex() const {return reinterpret_cast<combo<T,indexType>*>(ptr)->index - (N + 1);}
        static Manager& getManager() {return Manager::instance();}
        ~managed_ptr() {reset();}
    private:
        friend class Manager;
        explicit managed_ptr(T* ptr) {
            if (ptr == nullptr) {while (true);}
            this->ptr = ptr;
        }
        void reset() {
            if (ptr) {
                std::destroy_at(ptr);
                getManager().deleteMember(getIndex());
                ptr = nullptr;
            }
        }
        T* ptr;
    };

    consteval static uint32_t capacity() {return N;}
    [[nodiscard]] uint32_t size() const {return number;}
    [[nodiscard]] uint32_t available() const {return N - number;}

    template<typename... Args>
    managed_ptr make_managed(Args&&... args) {
        return managed_ptr(addMember(std::forward<Args>(args)...));
    }

    class iterator {
        combo<T,indexType>* cur;
        Manager* mgr;
    public:
        using pointer = T*;
        using reference = T&;

        iterator(Manager* mgr = nullptr, combo<T,indexType>* p = nullptr) : cur(p), mgr(mgr) {};
        reference operator*() const { return *reinterpret_cast<T*>(&cur->storage); }
        pointer operator->() const { return reinterpret_cast<T*>(&cur->storage); }

        iterator& operator++() {
            if (cur != &mgr->list[N]) cur++;
            while (cur != &mgr->list[N] && cur->index <= N) cur++;
            return *this;
        }

        bool operator==(const iterator& rhs) const { return cur == rhs.cur; }
        bool operator!=(const iterator& rhs) const { return cur != rhs.cur; }
    };

    iterator begin() {
        combo<T, indexType>* first = &list[0];
        while (first != &list[N] && first->index <= N) first++;
        return iterator(this, first);
    }
    iterator end() { return iterator(this, &list[N]); }

private:
    Manager() {
        if (N == 0) return;
        for (uint32_t i = 0; i < N; ++i) {
            list[i].index = i + 1;
        }
    };

    combo<T, indexType> list[N];
    uint32_t number = 0;
    uint32_t freeItem = 0;

    template<typename... Args>
    T* addMember(Args&&... args) {
        if (freeItem == N) return nullptr;
        indexType idx = freeItem;
        freeItem = list[idx].index;
        list[idx].index = N + 1 + idx;
        ++number;
        return new (&list[idx].storage) T(std::forward<Args>(args)...);
    }
    void deleteMember(indexType index) {
        list[index].index = freeItem;
        freeItem = index;
        --number;
    }
};
}
