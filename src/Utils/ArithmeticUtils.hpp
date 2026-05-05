#ifndef ARITHMETIC_UTILS_HPP
#define ARITHMETIC_UTILS_HPP

#include <limits>

template<typename T>
bool willAddOverflow(T a, T b) requires std::is_integral_v<T> && std::is_signed<T>::value {
    if ((b > 0) && (a > std::numeric_limits<T>::max() - b))
        return true;
    if ((b < 0) && (a < std::numeric_limits<T>::min() - b))
        return true;
    return false;
}

template<typename T>
bool willSubOverflow(T a, T b) requires std::is_integral_v<T> && std::is_signed<T>::value {
    if ((b < 0) && (a > std::numeric_limits<T>::max() + b))
        return true;
    if ((b > 0) && (a < std::numeric_limits<T>::min() + b))
        return true;
    
    return false;
}

template<typename T>
bool willMulOverflow(T a, T b) requires std::is_integral_v<T> && std::is_signed<T>::value {
    if (a == 0 || b == 0)
        return false;

    if (a == -1 && b == std::numeric_limits<T>::min())
        return true;
    if (b == -1 && a == std::numeric_limits<T>::min())
        return true;

    if (a > 0) {
        if (b > 0)
            return a > std::numeric_limits<T>::max() / b;
        else
            return b < std::numeric_limits<T>::min() / a;
    } else {
        if (b > 0)
            return a < std::numeric_limits<T>::min() / b;
        else
            return a != 0 && b < std::numeric_limits<T>::max() / a;
    }

    return false;
}

template<typename T>
bool willDivOverflow(T a, T b) requires std::is_integral_v<T> && std::is_signed<T>::value {
    if (b == 0)
        return true;

    if (a == std::numeric_limits<T>::min() && b == -1)
        return true;

    return false;
}

#endif // ARITHMETIC_UTILS_HPP