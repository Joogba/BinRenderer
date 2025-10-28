#pragma once
#include <type_traits>

namespace BinRenderer {

    template <typename E>
    concept BitmaskEnum = std::is_enum_v<E>;

    template <BitmaskEnum E>
    constexpr E operator|(E a, E b) {
        using T = std::underlying_type_t<E>;
        return static_cast<E>(static_cast<T>(a) | static_cast<T>(b));
    }
    template <BitmaskEnum E>
    constexpr E operator&(E a, E b) {
        using T = std::underlying_type_t<E>;
        return static_cast<E>(static_cast<T>(a) & static_cast<T>(b));
    }
    template <BitmaskEnum E>
    constexpr E operator~(E a) {
        using T = std::underlying_type_t<E>;
        return static_cast<E>(~static_cast<T>(a));
    }
    template <BitmaskEnum E>
    constexpr E& operator|=(E& a, E b) {
        a = a | b;
        return a;
    }
    template <BitmaskEnum E>
    constexpr bool HasFlag(E value, E flag) {
        using T = std::underlying_type_t<E>;
        return (static_cast<T>(value) & static_cast<T>(flag)) != 0;
    }

} // namespace BinRenderer