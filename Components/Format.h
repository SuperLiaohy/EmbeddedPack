//
// Created by liaohy on 11/17/25.
//

#pragma once
#include <array>
#include <charconv>
#include <cstdint>
#include <tuple>
#include <cstring>

namespace EP::Component {

struct Section {
    std::size_t src;
    std::size_t dst;

    [[nodiscard]] consteval std::size_t len() const {return dst - src;}
    [[nodiscard]] consteval bool isEmpty() const {return src == dst;}
};

struct Feature {
    std::size_t int_width;
    std::size_t float_width;
    [[nodiscard]] consteval bool isValid() const {return int_width > 0 && float_width > 0;}
};

template<std::size_t  N>
struct  Str  {
    char  chars[N]{};

    [[nodiscard]] consteval std::size_t len() const  {return N;}
    [[nodiscard]] consteval const char* c_str() const {return chars;}

    [[nodiscard]] consteval std::size_t formatCount() const {
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
    consteval std::array<Feature, Count> formatFeature() const {
        std::array<Feature, Count> features = {};
        std::size_t push = 0;
        std::size_t count = 0;
        for(std::size_t i = 0; i < N; ++i) {
            if (chars[i]=='\\') {
                if (i<N-1) i+=1;
                continue;
            }
            if (chars[i] == '{' && push == 0) {
                push = i+1;
            } else if (chars[i] == '}' && push > 0) {
                if (push != i) {
                    int mode = 0;
                    for (std::size_t j = push-1; j <= i; ++j) {
                        if (chars[j]=='.')
                            mode = 1;
                        switch (mode) {
                            case 0:
                                // static_assert('0'<=chars[j]&&chars[j]<='9',"feature should be a number");
                                if ('0'<=chars[j]&&chars[j]<='9')
                                features[count].int_width = features[count].int_width*10 + chars[j] - '0';
                                break;
                            case 1:
                                // static_assert('0'<=chars[j]&&chars[j]<='9',"feature should be a number");
                                if ('0'<=chars[j]&&chars[j]<='9')
                                features[count].float_width = features[count].float_width*10 + chars[j] - '0';
                                break;
                            default:
                                break;
                        }
                    }
                }
                ++count;
                push = 0;
            }
        }
        return features;
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
void fmtter(char*& buf, const T& value, const Feature& feature = {});


template<>
inline void fmtter<int>(char*& buf, const int& value, const Feature& feature) {
    auto ptr = std::to_chars(buf, buf+11, value);
    buf = ptr.ptr;
}
template<>
inline void fmtter<uint32_t>(char*& buf, const uint32_t& value, const Feature& feature) {
    auto ptr = std::to_chars(buf, buf+11, value);
    buf = ptr.ptr;
}
constexpr uint32_t pow10table[] = {
    0, 10, 100, 1'000, 10'000, 100'000, 1'000'000, 10'000'000, 100'000'000, 1'000'000'000
};
template<>
inline void fmtter<float>(char*& buf, const float& value, const Feature& feature) {
    if (std::isnan(value)) {
        std::memcpy(buf, "nan", 3);
        buf += 3;
        return;
    }
    if (std::isinf(value)) {
        if (value < 0) {
            std::memcpy(buf, "-inf", 4);
            buf += 4;
        } else {
            std::memcpy(buf, "inf", 3);
            buf += 3;
        }
        return;
    }
    auto ptr = std::to_chars(buf, buf + 11, static_cast<int>(value));
    if (feature.float_width==0){buf=ptr.ptr;return;}
    uint32_t scale = pow10table[feature.float_width];
    // for (int i = 0; i < feature.float_width; ++i) {
    //     scale*=10;
    // }
    int last_part = abs(static_cast<int>(value*scale))%scale;
    if (last_part==0) {buf=ptr.ptr;return;}
    *ptr.ptr = '.';
    ptr = std::to_chars(ptr.ptr+1, ptr.ptr+1+feature.float_width, last_part);
    buf=ptr.ptr;
    // auto ptr = std::to_chars(buf, buf+11, value);
    // buf = ptr.ptr;

}

template<>
inline void fmtter<int64_t>(char*& buf, const int64_t& value, const Feature& feature) {
    auto ptr = std::to_chars(buf, buf+20, value);
    buf = ptr.ptr;
}

template<>
inline void fmtter<uint64_t>(char*& buf, const uint64_t& value, const Feature& feature) {
    auto ptr = std::to_chars(buf, buf+20, value);
    buf = ptr.ptr;
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
    static std::size_t execute(char* buffer, ArgTuple&& argTuple) {
        constexpr std::size_t count = fmt.formatCount();
        static_assert(count == std::tuple_size_v<ArgTuple>, "format count is not equal to sizeof...(args)" );
        constexpr std::array<Feature, count> features = fmt.template formatFeature<count>();
        constexpr std::array<Section, count+1> sections = fmt.template formatSection<count>();
        auto p = buffer;
        (
            []<auto sections, auto features, typename T, std::size_t index>(char*&buffer, T&& value) {
                if constexpr (sections[index].len()>=1) {
                    memcpy(buffer, fmt.c_str()+sections[index].src, sections[index].len());
                    buffer += sections[index].len();
                } else if constexpr (!sections[index].isEmpty()) {
                    *buffer = fmt.c_str()[sections[index].src];
                    ++buffer;
                }
                fmtter<std::remove_reference_t<T>>(buffer, std::forward<T>(value), features[index]);
            }.template operator()<sections, features, std::tuple_element_t<I, ArgTuple>, I>(buffer, std::get<I>(std::forward<ArgTuple>(argTuple)))
            , ...
        );
        if constexpr (sections[count].len()>=1) {
            memcpy(buffer, fmt.c_str()+sections[count].src, sections[count].len());
            buffer += sections[count].len();
        } else if constexpr (!sections[count].isEmpty()) {
            *buffer = fmt.c_str()[sections[count].src];
            ++buffer;
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
