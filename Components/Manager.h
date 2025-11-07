//
// Created by liaohy on 11/7/25.
//

#pragma once

#include <array>
#include <cstdint>
namespace EP::Component {
template<typename T, typename indexType = uint32_t>
struct combo {
    static_assert(std::is_integral_v<indexType>, "T must be an integral type");
    using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
    explicit combo(indexType index = 0) : index(index) {};
    storage_t storage;
    indexType index;
    ~combo() = default;
};

template<typename T, uint32_t N, typename indexType = uint32_t>
class Manager {
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

        T* operator->() const {return ptr;}
        indexType getIndex() const {return reinterpret_cast<combo<T,indexType>*>(ptr)->index;}
        Manager* getManager() const {return reinterpret_cast<Manager*>(reinterpret_cast<combo<T,indexType>*>(ptr)-(reinterpret_cast<combo<T,indexType>*>(ptr)->index-1));}
        ~managed_ptr() {std::destroy_at(ptr); getManager()->delete_member(getIndex());}
    private:
        friend class Manager;
        explicit managed_ptr(T* ptr) {
            if (ptr == nullptr) {while (true);}
            this->ptr = ptr;
        };
        T* ptr;
    };
    Manager() {
        for (uint32_t i = 0; i < N; i++) {
            list[i].index = 0;
        }
        list[N-1].index = N;
    };
    consteval static uint32_t capacity() {return N;}
    [[nodiscard]] uint32_t size() const {return number;}
    [[nodiscard]] uint32_t available() const {return N - number;}

    template<typename... Args>
    managed_ptr make_managed(Args&&... args) {
        return managed_ptr(add_member(std::forward<Args>(args)...));
    }

    class iterator {
        combo<T,indexType>* cur;
        Manager* mgr;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator(Manager* mgr = nullptr, combo<T,indexType>* p = nullptr) : cur(p), mgr(mgr) {};
        reference operator*() const { return *reinterpret_cast<T*>(&cur->storage); }
        pointer operator->() const { return reinterpret_cast<T*>(&cur->storage); }

        // 前向迭代器需要支持 ++
        iterator& operator++() {
            do {
                ++cur;
            } while (cur->index==0&&cur!=&mgr->list[N-1]);
            return *this;
        }

        bool operator==(const iterator& rhs) const { return cur == rhs.cur; }
        bool operator!=(const iterator& rhs) const { return cur != rhs.cur; }
    };

    iterator begin() { return iterator(this, list); }
    iterator end() { return iterator(this, &list[N-1]); }

private:
    combo<T, indexType> list[N];
    uint32_t number = 0;

    template<typename... Args>
    T* add_member(Args&&... args) {
        for (uint32_t i = 0; i < N; i++) {
            if (list[i].index == 0) {
                ++number;
                list[i].index = i+1;
                return new (&list[i].storage) T(std::forward<Args>(args)...);
            }
        }
        return nullptr;
    }
    void delete_member(indexType index) {
        list[index-1].index = 0;
        --number;
    }
};


}
