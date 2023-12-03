// Copyright 2023 Ashley R. Thomas
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Adapted from https://github.com/AshleyT3/MFCmisc for Password Safe.

#pragma once

#include <Windows.h>
#include <functional>

class CInvokeGuiThreadSupport
{
public:
  CInvokeGuiThreadSupport(UINT wmInvokeCustomMessage)
    :
    m_wmInvokeCustomMessage(wmInvokeCustomMessage)
    {}

  UINT GetInvokeMessage() const { return m_wmInvokeCustomMessage; }

  void SetGuiThreadId(DWORD dwGuiThreadId) {
    ASSERT(m_dwGuiThreadId == 0);
    m_dwGuiThreadId = dwGuiThreadId;
  }
  void SetGuiThreadId() { SetGuiThreadId(::GetCurrentThreadId()); }
  DWORD GetGuiThreadId() const { return m_dwGuiThreadId; }

  bool IsGuiThread() const {
    ASSERT(m_dwGuiThreadId != 0);
    return m_dwGuiThreadId == ::GetCurrentThreadId();
  }

  // InvokeOnGuiThread runs 'lambda' on the UI thread of 'hwnd'.
  // Returns:
  //   true : Successfully run on the UI thread.
  //   false: Failed to run on the UI thread. Call GetLastError() for extended error information.
  bool InvokeOnGuiThread(HWND hWnd, std::function<LRESULT()> lambda, LRESULT* plResult = nullptr) {
    LRESULT lResult;
    if (IsGuiThread()) {
      lResult = lambda();
      SetLastError(ERROR_SUCCESS);
    } else {
      lResult = ::SendMessage(hWnd, m_wmInvokeCustomMessage, reinterpret_cast<WPARAM>(&lambda), 0);
    }
    if (plResult)
      *plResult = lResult;
    return GetLastError() == ERROR_SUCCESS;
  }

private:
  DWORD m_dwGuiThreadId = 0;
  UINT m_wmInvokeCustomMessage;
};

inline LRESULT OnHandleInvokeMessage(WPARAM wParam) {
  std::function<LRESULT()>& lambda = *reinterpret_cast<std::function<LRESULT()>*>(wParam);
  return lambda();
}
