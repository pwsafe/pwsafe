// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2006, 2008-2013, 2015 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "UnicodeUtils.h"
#include <memory>
#include <emmintrin.h>

CUnicodeUtils::CUnicodeUtils(void)
{
}

CUnicodeUtils::~CUnicodeUtils(void)
{
}

namespace
{
    // simple utility class that provides an efficient
    // writable string buffer. std::basic_string<> could
    // be used as well but has a less suitable interface.

    template<class T> class CBuffer
    {
    private:

        enum {FIXED_BUFFER_SIZE = 1024};

        T fixedBuffer[FIXED_BUFFER_SIZE];
        std::unique_ptr<T[]> dynamicBuffer;

        T* buffer;

    public:

        CBuffer (size_t minCapacity)
        {
            fixedBuffer[0] = 0;
            if (minCapacity <= FIXED_BUFFER_SIZE)
            {
                buffer = fixedBuffer;
            }
            else
            {
                dynamicBuffer.reset (new T[minCapacity]);
                buffer = dynamicBuffer.get();
            }
        }

        operator T*()
        {
            return buffer;
        }
    };

}

// wrap core routines and have string objects in the signatures
// instead of string buffers

#if defined(_MFC_VER) || defined(CSTRING_AVAILABLE)

CStringA CUnicodeUtils::GetUTF8(const CStringW& string)
{
    int size = string.GetLength()+1;
    CBuffer<char> buffer (4 * size);

    // Note: always use the Windows API function, do NOT try to implement a function that's faster.
    // The API takes this long because it can handle all edge cases, and that's what we need.
    int len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)string, size, buffer, 4*size, 0, NULL);
    if (len == 0)
        return CStringA();
    return CStringA (buffer, len-1);
}

CString CUnicodeUtils::GetUnicode(const CStringA& string)
{
    int size = string.GetLength()+1;
    CBuffer<wchar_t> buffer (2*size);

    // Note: always use the Windows API function, do NOT try to implement a function that's faster.
    // The API takes this long because it can handle all edge cases, and that's what we need.
    int len = MultiByteToWideChar(CP_UTF8, 0, (const char*)string, size, buffer, 2*size);
    if (len == 0)
        return CString();
    return CString (buffer, len-1);
}

CString CUnicodeUtils::UTF8ToUTF16 (const std::string& string)
{
    int size = (int)string.length()+1;
    CBuffer<wchar_t> buffer (2*size);

    int len = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), size, buffer, 2*size);
    if (len==0)
        return CString();
    return CString (buffer, len-1);
}
#endif //_MFC_VER

std::string CUnicodeUtils::StdGetUTF8(const std::wstring& wide)
{
    int size = (int)wide.length()+1;
    CBuffer<char> buffer (4 * size);

    int len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), size, buffer, 4*size, 0, NULL);
    if (len == 0)
        return std::string();
    return std::string (buffer, len-1);
}

std::wstring CUnicodeUtils::StdGetUnicode(const std::string& utf8)
{
    int size = (int)utf8.length()+1;
    CBuffer<wchar_t> buffer (2*size);

    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), size, buffer, 2*size);
    if (len==0)
        return std::wstring();
    return std::wstring (buffer, len-1);
}

std::string CUnicodeUtils::StdAnsiToUTF8( const std::string& ansi )
{
    int size = (int)ansi.length()+1;
    CBuffer<wchar_t> buffer (2*size);

    int len = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), size, buffer, 2*size);
    if (len==0)
        return ansi;
    CBuffer<char> buffer2 (4 * size);
    len = WideCharToMultiByte(CP_UTF8, 0, buffer, len, buffer2, 4*size, 0, NULL);
    if (len == 0)
        return ansi;
    return std::string (buffer2, len-1);
}

// load a string resource

#pragma warning(push)
#pragma warning(disable: 4200)
struct STRINGRESOURCEIMAGE
{
    WORD nLength;
    WCHAR achString[];
};
#pragma warning(pop)    // C4200

int LoadStringEx(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax, WORD wLanguage)
{
    const STRINGRESOURCEIMAGE* pImage;
    const STRINGRESOURCEIMAGE* pImageEnd;
    ULONG nResourceSize;
    HGLOBAL hGlobal;
    UINT iIndex;
#ifndef UNICODE
    BOOL defaultCharUsed;
#endif
    int ret;

    if (lpBuffer == NULL)
        return 0;
    lpBuffer[0] = 0;
    HRSRC hResource =  FindResourceEx(hInstance, RT_STRING, MAKEINTRESOURCE(((uID>>4)+1)), wLanguage);
    if (!hResource)
    {
        //try the default language before giving up!
        hResource = FindResource(hInstance, MAKEINTRESOURCE(((uID>>4)+1)), RT_STRING);
        if (!hResource)
            return 0;
    }
    hGlobal = LoadResource(hInstance, hResource);
    if (!hGlobal)
        return 0;
    pImage = (const STRINGRESOURCEIMAGE*)::LockResource(hGlobal);
    if(!pImage)
        return 0;

    nResourceSize = ::SizeofResource(hInstance, hResource);
    pImageEnd = (const STRINGRESOURCEIMAGE*)(LPBYTE(pImage)+nResourceSize);
    iIndex = uID&0x000f;

    while ((iIndex > 0) && (pImage < pImageEnd))
    {
        pImage = (const STRINGRESOURCEIMAGE*)(LPBYTE(pImage)+(sizeof(STRINGRESOURCEIMAGE)+(pImage->nLength*sizeof(WCHAR))));
        iIndex--;
    }
    if (pImage >= pImageEnd)
        return 0;
    if (pImage->nLength == 0)
        return 0;
#ifdef UNICODE
    ret = pImage->nLength;
    if (ret >= nBufferMax)
        ret = nBufferMax - 1;
    wcsncpy_s((wchar_t *)lpBuffer, nBufferMax, pImage->achString, ret);
    lpBuffer[ret] = 0;
#else
    ret = WideCharToMultiByte(CP_ACP, 0, pImage->achString, pImage->nLength, (LPSTR)lpBuffer, nBufferMax-1, ".", &defaultCharUsed);
    lpBuffer[ret] = 0;
#endif
    return ret;
}

#if defined(_DEBUG)
// Some test cases for these classes
static class CUnicodeUtilsTests
{
public:
    CUnicodeUtilsTests()
    {
#if defined(_MFC_VER) || defined(CSTRING_AVAILABLE)
        CStringA result = CUnicodeUtils::GetUTF8(L"<value>退订</value>");
        CStringW resultW = CUnicodeUtils::GetUnicode(result);
        ATLASSERT(resultW == L"<value>退订</value>");
        result = CUnicodeUtils::GetUTF8(L"äöü");
        resultW = CUnicodeUtils::GetUnicode(result);
        ATLASSERT(resultW == L"äöü");
        ATLASSERT(resultW.GetLength()==3);
        result = CUnicodeUtils::GetUTF8(L"Продолжить выполнение скрипта?");
        resultW = CUnicodeUtils::GetUnicode(result);
        ATLASSERT(resultW == L"Продолжить выполнение скрипта?");
        result = CUnicodeUtils::GetUTF8(L"dvostruki klik za automtsko uključivanje alfa");
        resultW = CUnicodeUtils::GetUnicode(result);
        ATLASSERT(resultW == L"dvostruki klik za automtsko uključivanje alfa");
        result = CUnicodeUtils::GetUTF8(L"包含有错误的结构。");
        resultW = CUnicodeUtils::GetUnicode(result);
        ATLASSERT(resultW == L"包含有错误的结构。");
        result = CUnicodeUtils::GetUTF8(L"个文件，共有 %2!d! 个文件");
        resultW = CUnicodeUtils::GetUnicode(result);
        ATLASSERT(resultW == L"个文件，共有 %2!d! 个文件");
        result = CUnicodeUtils::GetUTF8(L"は予期せぬオブジェクトを含んでいます。");
        resultW = CUnicodeUtils::GetUnicode(result);
        ATLASSERT(resultW == L"は予期せぬオブジェクトを含んでいます。");
        result = CUnicodeUtils::GetUTF8(L"Verify that the correct path and file name are given.");
        resultW = CUnicodeUtils::GetUnicode(result);
        ATLASSERT(resultW == L"Verify that the correct path and file name are given.");
#endif
    }

} UnicodeTestobject;
#endif
