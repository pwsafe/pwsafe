#ifndef __FMTSPECS_CVT_H__
#define __FMTSPECS_CVT_H__

#if defined(__GNUC__)  && (defined(UNICODE) || defined(_UNICODE))
#define CONVERT_GLIBC_FORMATSPECS
#endif

/*
 * In UNICODE builds (more specifically, the wide versions of printf functions family) Microsoft's libc
 * and glibc interpret the %s format specifier differently. Microsoft interprets the corresponding
 * string argument to be wide-char, while glibc interprets it as single-char.  glibc requires either %S
 * or %ls to interpret the corresponding string argument as wide-char.
 * 
 * Since we *always* use single or wide char functions/strings depending on UNICODE, we convert
 * all %s to %S while building with GNU/glibc and UNICODE defined using the functions below
 * 
 * http://msdn.microsoft.com/en-us/library/hf4y5e3w(v=VS.100).aspx
 * http://msdn.microsoft.com/en-us/library/tcxf1dw6(v=VS.100).aspx
 * http://www.gnu.org/software/libc/manual/html_node/Other-Output-Conversions.html#Other-Output-Conversions
 * 
 * Note that this is not a Linux vs. Windows difference.  The same issue exists if we build this on WIN32
 * with GNU with UNICODE defined.  Also, we don't do a similar conversion for ANSI builds (i.e. %S => %s)
 * since all our format specs are always %s anyway.
 */
 
#ifdef CONVERT_GLIBC_FORMATSPECS
template <typename T>
inline void ConvertFormatSpecs(T& specs)
{
  for(typename T::size_type pos = 0; (pos = specs.find(L"%s", pos)) != T::npos; pos += 2) {
    specs[pos+1] = L'S';
  }
}

template <typename T>
inline T ConvertFormatSpecs(const wchar_t* fmt)
{
  T specs(fmt);
  ConvertFormatSpecs(specs);
  return specs;
}

#define _FMT(s) ConvertFormatSpecs<stringT>(_T(s)).c_str()

/* not available unless UNICODE is defined */
inline stringT FormatStr(const wchar_t* s) { return ConvertFormatSpecs<stringT>(s); }

#else
// Not GNU, or not UNICODE

template <typename T>
inline void ConvertFormatSpecs(T& /*specs*/) {}

template <typename T>
inline T ConvertFormatSpecs(const wchar_t* fmt) { return T(fmt); }

#define _FMT(s) _T(s)

inline stringT FormatStr(const wchar_t* str) { return stringT(str); }
inline stringT FormatStr(const char* str) { return stringT(str); }

#endif

//__FMTSPECS_CVT_H__
#endif
