/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file XString.h
 *
 * STL-based implementation of secure strings.
 * Like std::string in all respects, except that
 * memory is scrubbed before being returned to system.
 *
 *
 */

#ifndef _XSTRING_H_
#define _XSTRING_H_
#include <string>

#include <memory>
#include <limits>

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
        return &r;
      }

      const_pointer address(const_reference c) const {
        return &c;
      }

      size_type max_size() const {
        return (std::numeric_limits<size_t>::max)() / sizeof(T);
      }

      // In-place construction
      void construct(pointer p, const_reference c) {
        // placement new operator
        new(reinterpret_cast<void*>(p)) T(c);
      }

      // In-place destruction
      void destroy(pointer p) {
        // call destructor directly
        (p)->~T();
      }

      // Rebind to allocators of other types
      template <typename U>
        struct rebind {
          typedef SecureAlloc<U> other;
        };

      // Allocate raw memory
      pointer allocate(size_type n, const void* = NULL) {
        void* p = malloc(n * sizeof(T));
        // TRACE(_T("Securely Allocated %d bytes at %p\n"), n * sizeof(T), p);
        if(p == NULL)
          throw std::bad_alloc();
        return pointer(p);
      }

      // Free raw memory.
      // Note that C++ standard defines this function as
      // deallocate(pointer p, size_type n).
      void deallocate(void* p, size_type n){
        // assert(p != NULL);
        // The standard states that p must not be NULL. However, some
        // STL implementations fail this requirement, so the check must
        // be made here.
        if (p == NULL)
          return;

        if (n > 0) {
          const size_type N = n * sizeof(T);
          memset(p, 0x55, N);
          memset(p, 0xAA, N);
          memset(p, 0x00, N);
        }
        free(p);
      }

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

#ifdef UNICODE
typedef std::basic_string<wchar_t,
                          std::char_traits<wchar_t>,
                          S_Alloc::SecureAlloc<wchar_t> > XString;
#else
typedef std::basic_string<char,
                          std::char_traits<char>,
                          S_Alloc::SecureAlloc<char> > XString;
#endif

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
