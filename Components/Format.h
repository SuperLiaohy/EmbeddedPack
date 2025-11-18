//
// Created by liaohy on 11/17/25.
//

#pragma once
#include <array>
#include <charconv>
#include <cstdint>
#include <tuple>
#include <string.h>

namespace EP::Component {

struct Section {
    std::size_t src;
    std::size_t dst;

    consteval std::size_t len() const {return dst - src;}
    consteval bool isEmpty() const {return src == dst;}
};

template<std::size_t  N>
struct  Str  {
    char  chars[N]{};

    [[nodiscard]] consteval std::size_t len() const  {return N;}
    consteval const char* c_str() const {return chars;}
    consteval const char* data() const {return chars;}

    consteval std::size_t formatCount() const {
        std::size_t count = 0;
        std::size_t push = 0;
        for(std::size_t i = 0; i < N; ++i) {
            if (chars[i]=='\\') {
                if (i<N-1) i+=1;
                continue;
            }
            if (chars[i] == '{' && push == 0) {
                ++push;
            } else if (chars[i] == '}' && push > 0) {
                --push;
                ++count;
            }
        }
        return count;
    }

    template<std::size_t Count>
    consteval std::array<std::size_t, Count> formatIndex() const {
        if (Count == 0) {return {};}
        std::array<std::size_t, Count> indexes = {};
        std::size_t push = 0;
        std::size_t count = 0;
        for(std::size_t i = 0; i < N; ++i) {
            if (chars[i]=='\\') {
                if (i<N-1) i+=1;
                continue;
            }
            if (chars[i] == '{' && push == 0) {
                indexes[count++] = i;
                ++push;
            } else if (chars[i] == '}' && push > 0) {
                --push;
            }
        }
        return indexes;
    }

    template<std::size_t Count>
    consteval std::array<Section, Count+1> formatSection() const {
        std::array<Section, Count+1> sections = {};
        std::size_t push = 0;
        std::size_t count = 0;
        sections[count].src = 0;
        if (Count == 0) return {Section{0,N}};
        for(std::size_t i = 0; i < N; ++i) {
            if (chars[i]=='\\') {
                if (i<N-1) i+=1;
                continue;
            }
            if (chars[i] == '{' && push == 0) {
                push = i+1;
            } else if (chars[i] == '}' && push > 0) {
                sections[count].dst = push-1;
                ++count;
                if (count==Count) {
                    sections[count].src = i+1;
                    sections[count].dst = N;
                    break;
                }
                sections[count].src = i+1;
                push = 0;
            }
        }
        return sections;
    }

    template<std::size_t X>
    consteval Str<N+X-1> operator+(Str<X> str) const {
        Str<N+X-1> result = {};
        for(std::size_t i = 0; i < N-1; ++i) {
            result.chars[i] = chars[i];
        }
        for(std::size_t i = 0; i < X; ++i) {
            result.chars[i+N-1] = str.chars[i];
        }
        return result;
    }
};

template<typename T>
void fmtter(char*& buf, const T& value);


template<>
void fmtter<int>(char*& buf, const int& value) {
    // char tmp[11];
    // char* p = tmp + 11;
    // unsigned u = value < 0 ? static_cast<unsigned>(-value) : static_cast<unsigned>(value);
    // do { *--p = '0' + (u % 10); u /= 10; } while (u);
    // if (value < 0) *--p = '-';
    // size_t len = tmp + 11 - p;
    // for (size_t i = 0; i < len; ++i) buf[i] = p[i];
    // buf+= len;
    auto ptr = std::to_chars(buf, buf+10, value);
    buf = ptr.ptr;
}
template<>
void fmtter<uint32_t>(char*& buf, const uint32_t& value) {
    // char tmp[11];
    // char* p = tmp + 11;
    // unsigned u = value < 0 ? static_cast<unsigned>(-value) : static_cast<unsigned>(value);
    // do { *--p = '0' + (u % 10); u /= 10; } while (u);
    // if (value < 0) *--p = '-';
    // size_t len = tmp + 11 - p;
    // for (size_t i = 0; i < len; ++i) buf[i] = p[i];
    // buf+= len;
    auto ptr = std::to_chars(buf, buf+10, value);
    buf = ptr.ptr;
}

template<>
void fmtter<int64_t>(char*& buf, const int64_t& value) {
    char tmp[20];
    char* p = tmp + 20;
    uint64_t u = value < 0 ? static_cast<uint64_t>(-value) : static_cast<uint64_t>(value);
    do { *--p = '0' + (u % 10); u /= 10; } while (u);
    if (value < 0) *--p = '-';
    size_t len = tmp + 20 - p;
    // if (len >= bufsz) len = bufsz - 1;
    for (size_t i = 0; i < len; ++i) buf[i] = p[i];
    buf+= len;
    // buf[len] = '\0';
    // return len;
}



template <typename T>
struct is_Str : std::false_type {};

template <std::size_t N>
struct is_Str<Str<N>> : std::true_type {};

template <typename T>
concept String = is_Str<std::remove_cvref_t<T>>::value;

template <String auto fmt,typename ArgTuple, typename IndexSequence>
struct for_each_impl;

template <String auto fmt, typename ArgTuple, std::size_t... I>
struct for_each_impl<fmt, ArgTuple, std::index_sequence<I...>> {
    inline __attribute__((always_inline)) static std::size_t execute(char* buffer, ArgTuple&& argTuple) {
        constexpr std::size_t count = fmt.formatCount();
        static_assert(count == std::tuple_size_v<ArgTuple>, "format count is not equal to sizeof...(args)" );
        constexpr std::array<Section, count+1> sections = fmt.template formatSection<count>();
        auto p = buffer;
        (
            []<auto sections, typename T, std::size_t index>(char*&buffer, T&& value) {
                if constexpr (sections[index].len()>=1) {
                    memcpy(buffer, fmt.data()+sections[index].src, sections[index].len());
                    buffer += sections[index].len();
                } else if constexpr (!sections[index].isEmpty()) {
                    // for (int i = sections[index].src; i < sections[index].dst; ++i) {
                        *buffer = fmt.data()[sections[index].src];
                        ++buffer;
                    // }
                }
                fmtter<std::remove_reference_t<T>>(buffer, std::forward<T>(value));
            }.template operator()<sections, std::tuple_element_t<I, ArgTuple>, I>(buffer, std::get<I>(std::forward<ArgTuple>(argTuple)))
            , ...
        );
        if constexpr (sections[count].len()>=1) {
            memcpy(buffer, fmt.data()+sections[count].src, sections[count].len());
            buffer += sections[count].len();
        } else if constexpr (!sections[count].isEmpty()) {
            // for (int i = sections[count].src; i < sections[count].dst; ++i) {
                *buffer = fmt.data()[sections[count].src];
                ++buffer;
            // }
        }
        return buffer - p;
    }
};

template<String auto fmt, typename...Args>
std::size_t format(char* buffer,  Args&&... args) {
    using ArgTuple = std::tuple<Args...>;
    using Indices = std::make_index_sequence<sizeof...(Args)>;
    return for_each_impl<fmt, ArgTuple, Indices>::execute(buffer, std::forward_as_tuple(std::forward<Args>(args)...));
}

}
