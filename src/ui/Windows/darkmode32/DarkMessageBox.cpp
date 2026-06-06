/*
* MIT License
* 
* Copyright (C) 2026 Marton Anka
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <windows.h>
#include <cwchar>
#include <commctrl.h>
#include "DMSubclass.h"

#pragma comment(lib, "comctl32.lib")

namespace
{
    constexpr UINT_PTR kMsgBoxBackfillSubclassId = 0x44524B31; // arbitrary unique id

    bool MessageBoxShouldBeDark()
    {
        return DarkMode::isEnabled() &&
            DarkMode::isColorDark(DarkMode::getDlgBackgroundColor());
    }

    struct ExcludeChildrenCtx
    {
        HWND hwndParent;
        HDC  hdc;
    };

    BOOL CALLBACK ExcludeVisibleChildFromClip(HWND hwndChild, LPARAM param)
    {
        auto* ctx = reinterpret_cast<ExcludeChildrenCtx*>(param);

        if (!::IsWindowVisible(hwndChild))
            return TRUE;

        RECT rcScreen{};
        if (!::GetWindowRect(hwndChild, &rcScreen))
            return TRUE;

        POINT pts[2] =
        {
            { rcScreen.left,  rcScreen.top    },
            { rcScreen.right, rcScreen.bottom }
        };

        ::MapWindowPoints(nullptr, ctx->hwndParent, pts, 2);

        ::ExcludeClipRect(
            ctx->hdc,
            pts[0].x,
            pts[0].y,
            pts[1].x,
            pts[1].y);

        return TRUE;
    }

    void FillMessageBoxClientBackground(HWND hwnd, HDC hdc, bool excludeChildren)
    {
        if (!hwnd || !hdc)
            return;

        RECT rcClient{};
        ::GetClientRect(hwnd, &rcClient);

        const int saved = ::SaveDC(hdc);

        if (excludeChildren && saved != 0)
        {
            ExcludeChildrenCtx ctx{ hwnd, hdc };
            ::EnumChildWindows(
                hwnd,
                ExcludeVisibleChildFromClip,
                reinterpret_cast<LPARAM>(&ctx));
        }

        ::FillRect(hdc, &rcClient, DarkMode::getDlgBackgroundBrush());

        if (saved != 0)
            ::RestoreDC(hdc, saved);
    }

    LRESULT CALLBACK MessageBoxBackfillSubclass(
        HWND hwnd,
        UINT msg,
        WPARAM wp,
        LPARAM lp,
        UINT_PTR subclassId,
        DWORD_PTR)
    {
        switch (msg)
        {
        case WM_NCDESTROY:
            ::RemoveWindowSubclass(hwnd, MessageBoxBackfillSubclass, subclassId);
            break;

        case WM_ERASEBKGND:
            if (MessageBoxShouldBeDark())
            {
                // During erase, fill everything. Child windows will paint afterward.
                FillMessageBoxClientBackground(
                    hwnd,
                    reinterpret_cast<HDC>(wp),
                    false);

                return TRUE;
            }
            break;

        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORDLG:
            if (MessageBoxShouldBeDark())
                return DarkMode::onCtlColorDlg(reinterpret_cast<HDC>(wp));
            break;

        case WM_CTLCOLORSTATIC:
            if (MessageBoxShouldBeDark())
            {
                HWND hwndChild = reinterpret_cast<HWND>(lp);

                return DarkMode::onCtlColorDlgStaticText(
                    reinterpret_cast<HDC>(wp),
                    ::IsWindowEnabled(hwndChild) == TRUE);
            }
            break;

        case WM_PAINT:
            if (MessageBoxShouldBeDark())
            {
                // Let MessageBox do its normal painting first.
                const LRESULT ret = ::DefSubclassProc(hwnd, msg, wp, lp);

                // Then nuke the leftover light client background.
                // Clip children so the buttons, icon, and text are not painted over.
                HDC hdc = ::GetDC(hwnd);
                if (hdc)
                {
                    FillMessageBoxClientBackground(hwnd, hdc, true);
                    ::ReleaseDC(hwnd, hdc);
                }

                return ret;
            }
            break;
        }

        return ::DefSubclassProc(hwnd, msg, wp, lp);
    }

    void InstallMessageBoxBackfillSubclass(HWND hwnd)
    {
        ::SetWindowSubclass(
            hwnd,
            MessageBoxBackfillSubclass,
            kMsgBoxBackfillSubclassId,
            0);
    }

    thread_local HHOOK g_msgBoxHook = nullptr;
    thread_local bool  g_msgBoxDarkened = false;

    bool IsDialogWindow(HWND hwnd)
    {
        wchar_t cls[32]{};
        ::GetClassNameW(hwnd, cls, _countof(cls));
        return std::wcscmp(cls, L"#32770") == 0;
    }

    bool AppWantsDarkMessageBox()
    {
        return DarkMode::isEnabled();
    }

    void DarkenMessageBoxWindow(HWND hwnd)
    {
        if (!hwnd || g_msgBoxDarkened || !AppWantsDarkMessageBox())
            return;

        g_msgBoxDarkened = true;

        DarkMode::setWindowEraseBgSubclass(hwnd);
        DarkMode::setDarkWndNotifySafe(hwnd, true);

        // Disable any themed dialog texture nonsense.
        DarkMode::enableThemeDialogTexture(hwnd, false);

        // Must be installed after the darkmode32plus subclasses.
        // This lets our WM_PAINT call DefSubclassProc() first, then backfill.
        InstallMessageBoxBackfillSubclass(hwnd);

        ::RedrawWindow(
            hwnd,
            nullptr,
            nullptr,
            RDW_INVALIDATE |
            RDW_ERASE |
            RDW_ALLCHILDREN |
            RDW_FRAME |
            RDW_UPDATENOW);
    }

    LRESULT CALLBACK MessageBoxCbtProc(int code, WPARAM wParam, LPARAM lParam)
    {
        if (code == HCBT_ACTIVATE)
        {
            HWND hwnd = reinterpret_cast<HWND>(wParam);

            if (IsDialogWindow(hwnd))
                DarkenMessageBoxWindow(hwnd);
        }

        return ::CallNextHookEx(g_msgBoxHook, code, wParam, lParam);
    }

    class ScopedMessageBoxHook
    {
    public:
        ScopedMessageBoxHook()
        {
            g_msgBoxDarkened = false;

            // Thread hook only
            g_msgBoxHook = ::SetWindowsHookExW(
                WH_CBT,
                MessageBoxCbtProc,
                nullptr,
                ::GetCurrentThreadId());
        }

        ~ScopedMessageBoxHook()
        {
            if (g_msgBoxHook)
            {
                ::UnhookWindowsHookEx(g_msgBoxHook);
                g_msgBoxHook = nullptr;
            }

            g_msgBoxDarkened = false;
        }

        ScopedMessageBoxHook(const ScopedMessageBoxHook&) = delete;
        ScopedMessageBoxHook& operator=(const ScopedMessageBoxHook&) = delete;
    };
}

namespace DarkMode
{
    int DarkMessageBox(HWND owner, LPCWSTR text, LPCWSTR caption, UINT type)
    {
        ScopedMessageBoxHook hook;

        return ::MessageBoxW(
            owner,
            text,
            caption,
            type);
    }
}
