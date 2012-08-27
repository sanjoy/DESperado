#ifndef __UTILS_HPP
#define __UTILS_HPP

#include <stdint.h>
#include <iostream>

namespace des {

template <bool> class StaticAssertion;
template <> class StaticAssertion<true> { };

#define SEMI_STATIC_JOIN(a, b) SEMI_STATIC_JOIN_HELPER(a, b)
#define SEMI_STATIC_JOIN_HELPER(a, b) a##b

template <int> class StaticAssertionHelper { };

#define static_assert(test)                                                    \
     typedef                                                                   \
     StaticAssertionHelper<sizeof(StaticAssertion<static_cast<bool>((test))>)> \
     SEMI_STATIC_JOIN(__StaticAssertTypedef__, __LINE__)


template<class T>
class SmartPointer {
 public:
  explicit SmartPointer(T* ptr) : pointer_(ptr) {}

  SmartPointer(const SmartPointer &other) {
    pointer_ = other.pointer_;
    const_cast<SmartPointer&>(other).pointer_ = NULL;
  }

  void operator=(SmartPointer& other) {
    pointer_ = other.pointer_;
    other.pointer_ = NULL;
  }

  T* operator->() { return pointer_; }
  T* get_pointer() { return pointer_; }

  ~SmartPointer() {
    delete pointer_;
  }

 private:
  T* pointer_;
};

template<typename T>
T min(T a, T b) { return a > b ? b : a; }

#if defined(__GNUC__) && !defined(DEBUG)
#if (__GNUC__ >= 4)
#define ALWAYS_INLINE(header) inline header  __attribute__((always_inline))
#define NEVER_INLINE(header) header __attribute__((noinline))
#define LIKELY(x)    __builtin_expect(!!(x), 1)
#define UNLIKELY(x)  __builtin_expect(!!(x), 0)
#else
#define ALWAYS_INLINE(header) inline __attribute__((always_inline)) header
#define NEVER_INLINE(header) header
#define LIKELY(x)    __builtin_expect(!!(x), 1)
#define UNLIKELY(x)  __builtin_expect(!!(x), 0)
#endif
#elif defined(_MSC_VER) && !defined(DEBUG)
#define ALWAYS_INLINE(header) __forceinline header
#define NEVER_INLINE(header) header
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#else
#define ALWAYS_INLINE(header) inline header
#define NEVER_INLINE(header) header
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

};


#endif // __UTILS_HPP
