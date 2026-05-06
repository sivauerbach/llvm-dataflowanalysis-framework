#ifndef SIGNED_RANGE_HPP
#define SIGNED_RANGE_HPP

#include <type_traits>
#include <string>

class SignedRange {
public:
    int64_t lower;
    int64_t upper;
    bool empty;

    static int64_t min(size_t sizeInBits);
    static int64_t max(size_t sizeInBits);

public:
    static SignedRange full(size_t sizeInBits = 64) {
        return SignedRange(min(sizeInBits), max(sizeInBits));
    }

    static SignedRange meet(SignedRange r1, SignedRange r2);
    static SignedRange widen(SignedRange newRange, SignedRange oldRange, size_t sizeInBits);
    static SignedRange narrow(SignedRange newRange, SignedRange oldRange, size_t sizeInBits);
    
    static SignedRange addRanges(SignedRange r1, SignedRange r2, bool noSignedWrap, size_t sizeInBits);
    static SignedRange subRanges(SignedRange r1, SignedRange r2, bool noSignedWrap, size_t sizeInBits);
    static SignedRange mulRanges(SignedRange r1, SignedRange r2, bool noSignedWrap, size_t sizeInBits);
    static SignedRange signDivRanges(SignedRange r1, SignedRange r2, bool noSignedWrap, size_t sizeInBits);

    // Maybe need for no nsw case.
    static SignedRange singedWrapCast(SignedRange range, size_t sizeInBits);

public:
    // constructor
    SignedRange(): SignedRange(true) { }
    explicit SignedRange (bool _empty): lower(0), upper(0), empty(_empty) {}
    SignedRange (int64_t _lower, int64_t _upper): lower(_lower), upper(_upper), empty(false) {}
    SignedRange (int64_t _lower, int64_t _upper, bool _empty): lower(_lower), upper(_upper), empty(_empty) {}

    SignedRange(const SignedRange& other): lower(other.lower), upper(other.upper), empty(other.empty) {}
    SignedRange(SignedRange&& other) noexcept: lower(std::move(other.lower)), upper(std::move(other.upper)), empty(std::move(other.empty)) {}

    SignedRange& operator=(const SignedRange& other);
    SignedRange& operator=(SignedRange&& other) noexcept;

    // Note SignedRange is a poset, that is we might have x not bigger of smaller then y and x different then y.
    friend bool operator==(const SignedRange&, const SignedRange&);
    friend bool operator!=(const SignedRange&, const SignedRange&);
    friend bool operator>=(const SignedRange&, const SignedRange&);
    friend bool operator<=(const SignedRange&, const SignedRange&);
    friend bool operator>(const SignedRange&, const SignedRange&);
    friend bool operator<(const SignedRange&, const SignedRange&);

    bool isEmpty() const { return empty; }
    bool isSingleton() const { return (! isEmpty()) && (lower == upper); }
    int64_t getLower() const { return lower; }
    int64_t getUpper() const { return upper; }

    explicit operator std::string() const { return getRangeString(); }
    std::string getRangeString() const;
};

#endif // ! SIGNED_RANGE_HPP