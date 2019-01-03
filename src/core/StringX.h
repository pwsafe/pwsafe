/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __STRINGX_H
#define __STRINGX_H

/**
 * \file StringX.h
 *
 * STL-based implementation of secure strings.
 * Like std::string in all respects, except that
 * memory is scrubbed before being returned to system.
 *
 */

#include <string>
#include <memory>
#include <limits>
#include <cstddef> // for ptrdiff_t
#include <cstdlib> // for malloc
#include <cstring> // for memset

#include "../os/typedefs.h"
#include "./PwsPlatform.h"

// Using extern definition here instead of including "Util.h" because Util.h
// references the StringX class and by including "Util.h" here, the StringX
// class isn't fully defined yet.
extern void trashMemory(void *buffer, size_t length);

namespace S_Alloc
{
  template <typename T>
    class SecureAlloc
    {
    public:
      // Typedefs
      typedef size_t    size_type;
      typedef ptrdiff_t difference_type;
      typedef T*        pointer;
      typedef const T*  const_pointer;
      typedef T&        reference;
      typedef const T&  const_reference;
      typedef T         value_type;

    public:
      // Constructors
      SecureAlloc() throw() {}
      SecureAlloc(const SecureAlloc&) throw() {}

      template <typename U>
        SecureAlloc(const SecureAlloc<U>&) throw() {}

      SecureAlloc& operator=(const SecureAlloc&) {
        return *this;
      }

      // Destructor
      ~SecureAlloc() throw() {}

      // Utility functions
      pointer address(reference r) const {
        return (&r);
      }

      const_pointer address(const_reference c) const {
        return (&c);
      }

      size_type max_size() const {
        return (std::numeric_limits<size_t>::max)() / sizeof(T);
      }

      // In-place construction
      void construct(pointer p, const_reference c) {
        // placement new operator
        new(reinterpret_cast<void *>(p)) T(c);
      }

      // In-place destruction
      void destroy(pointer p) const {
        // call destructor directly
        (p)->T::~T();
      }

      // Rebind to allocators of other types
      template <typename U>
        struct rebind {
          typedef SecureAlloc<U> other;
        };

      // Allocate raw memory
      pointer allocate(size_type n, const_pointer hint = 0) {
        UNREFERENCED_PARAMETER(hint);
        pointer p = static_cast<pointer>(std::malloc(n * sizeof(T)));
        if (p == nullptr)
          throw std::bad_alloc();
        return p;
      }

#ifdef _WIN32
#pragma optimize("", off)
#endif
      // Free raw memory.
      // Note that C++ standard defines this function as:
      //   deallocate(pointer p, size_type n).
      void deallocate(pointer p, size_type n) {
        // assert(p != nullptr);
        // The standard states that p must not be nullptr. However, some
        // STL implementations fail this requirement, so the check must
        // be made here.
        if (p == nullptr)
          return;

        if (n > 0) {
          const size_type N = n * sizeof(T);
          trashMemory((void *)p, N);
        }
        std::free(p);
      }
#ifdef _WIN32
#pragma optimize("", on)
#endif

    private:
      // No data

    }; // end of SecureAlloc

  // Comparison
  template <typename T1, typename T2>
    bool operator==(const SecureAlloc<T1>&,
                    const SecureAlloc<T2>&) throw() {
    return true;
  }

  template <typename T1, typename T2>
    bool operator!=(const SecureAlloc<T1>&,
                    const SecureAlloc<T2>&) throw() {
    return false;
  }

} // end namespace S_Alloc

typedef std::basic_string<wchar_t,
                          std::char_traits<wchar_t>,
                          S_Alloc::SecureAlloc<wchar_t> > StringX;

// Following should really be StringX member functions, but there's no 
// elegant way of extending a template class without public inheritance, 
// including duplicating large parts of the interface
//
// Since we need the for stringT as well, we might as well templatize them
// (In for a dime, in for a $).

template<class T> int CompareNoCase(const T &s1, const T &s2);
template<class T> int CompareCase(const T &s1, const T &s2);
template<class T> void ToLower(T &s);
template<class T> void ToUpper(T &s);
template<class T> T &TrimRight(T &s, const TCHAR *set = nullptr);
template<class T> T &TrimLeft(T &s, const TCHAR *set = nullptr);
template<class T> T &Trim(T &s, const TCHAR *set = nullptr);
template<class T> void EmptyIfOnlyWhiteSpace(T &s);
template<class T> int Replace(T &s, TCHAR from, TCHAR to);
template<class T> int Replace(T &s, const T &from, const T &to);
template<class T> int Remove(T &s, TCHAR c);
template<class T> void Format(T &s, const TCHAR *fmt, ...);
template<class T> void Format(T &s, int fmt, ...);
template<class T> void LoadAString(T &s, int id);

inline stringT stringx2std(const StringX &str) { return stringT(str.data(), str.size()); }
inline StringX std2stringx(const stringT& str)   { return StringX(str.data(), str.size()); }

#endif /* __STRINGX_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
