#ifndef PASS_TYPES_HPP
#define PASS_TYPES_HPP

using namespace llvm;

namespace framework {

enum class PASS_TYPE {
    FORWARDS = 0,
    BACKWARDS = 1
};

namespace type_traits {

template <PASS_TYPE PassType>
struct isForword : std::false_type { };

template <PASS_TYPE PassType>
struct isBackwards : std::false_type { };

template <>
struct isForword<PASS_TYPE::FORWARDS> : std::true_type { };

template <>
struct isBackwards<PASS_TYPE::BACKWARDS> : std::true_type { };

}

}

#endif // !PASS_TYPES_HPP