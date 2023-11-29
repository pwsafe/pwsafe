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

#include <Afxcmn.h>
#include "ProgressPieStaticCtrl.h"

IMPLEMENT_DYNAMIC(CProgressPieStaticCtrl, CStatic)

CProgressPieStaticCtrl::CProgressPieStaticCtrl()
  :
  m_common(this)
{
}

CProgressPieStaticCtrl::~CProgressPieStaticCtrl()
{
}

BEGIN_MESSAGE_MAP(CProgressPieStaticCtrl, CStatic)
  ON_WM_PAINT()
END_MESSAGE_MAP()

void CProgressPieStaticCtrl::OnPaint()
{
  m_common.OnPaint();
}

void CProgressPieStaticCtrl::SetPercent(double percent)
{
  m_common.SetPercent(percent);
  Invalidate(FALSE);
}
