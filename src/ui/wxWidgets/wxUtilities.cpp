/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file wxUtilities.cpp
*
* Contains generic utility functions that should be global and don't fit anywhere else
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/taskbar.h>
#include <wx/tokenzr.h>
#include <wx/versioninfo.h>

#include "core/PWScore.h"

#include "wxUtilities.h"

/*
 * Reads a file into a PWScore object, and displays an appropriate msgbox
 * in case of failure.  Returns PWScore::SUCCESS on success
 */

int ReadCore(PWScore& othercore, const wxString& file, const StringX& combination,
             bool showMsgbox /*= true*/, wxWindow* msgboxParent /*= nullptr*/,
        bool setupCopy /*= false*/)
{
  othercore.ClearDBData();

  StringX dbpath(tostringx(file));
  int rc = othercore.ReadFile(dbpath, combination);

  if (setupCopy)
    PWSprefs::GetInstance()->SetupCopyPrefs();

  switch (rc) {
    case PWScore::SUCCESS:
      othercore.SetCurFile(tostringx(file));
      break;

    case PWScore::CANT_OPEN_FILE:
      if (showMsgbox)
        wxMessageBox(wxString(file) << wxT("\n\n") << _("Could not open file for reading!"),
                    _("File Read Error"), wxOK | wxICON_ERROR, msgboxParent );
      break;

    case PWScore::BAD_DIGEST:
      if (showMsgbox && wxMessageBox(wxString(file) << wxT("\n\n") << _("File corrupt or truncated!\nData may have been lost or modified.\nContinue anyway?"),
            _("File Read Error"), wxYES_NO | wxICON_QUESTION, msgboxParent) == wxYES) {
        rc = PWScore::SUCCESS;
      }
      break;

    default:
      if (showMsgbox)
        wxMessageBox( wxString(file) << wxT("\n\n") << _("Unknown error"), _("File Read Error"), wxOK | wxICON_ERROR, msgboxParent);
      break;
  }

  return rc;
}

void HideWindowRecursively(wxTopLevelWindow* win, wxWindowList& hiddenWindows)
{
  if (!win)
    return;
  wxWindowList& children = win->GetChildren();
  for(wxWindowList::iterator itr = children.begin(); itr != children.end(); ++itr) {
    if ((*itr)->IsTopLevel() && (*itr)->IsShown()) {
      HideWindowRecursively(wxDynamicCast(*itr, wxTopLevelWindow), hiddenWindows);
    }
  }
  //Don't call Hide() here, which just calls Show(false), which is overridden in
  //derived classes, and wxDialog actually cancels the modal loop and closes the window
  win->wxWindow::Show(false);
  //push_front ensures we Show() in the reverse order of Hide()'ing
  hiddenWindows.push_front(win);
}

void ShowWindowRecursively(wxWindowList& hiddenWindows)
{
  for(wxWindowList::iterator itr = hiddenWindows.begin(); itr != hiddenWindows.end(); ++itr) {
    wxWindow* win = (*itr);
    //Show is virtual, and dialog windows assume the window is just starting up when Show()
    //is called.  Make sure to call the base version
    win->wxWindow::Show(true);
    win->Raise();
    win->Update();
  }
  hiddenWindows.clear();
}

/////////////////////////////////////////////////////////////
// MultiCheckboxValidator
//
MultiCheckboxValidator::MultiCheckboxValidator(int ids[],
                                               size_t num,
                                               const wxString& msg,
                                               const wxString& title): m_ids(new int[num]),
                                                                       m_count(num),
                                                                       m_msg(msg),
                                                                       m_title(title)

{
  memcpy(m_ids, ids, sizeof(m_ids[0])*m_count);
}

MultiCheckboxValidator::MultiCheckboxValidator(const MultiCheckboxValidator& other):
  /*Copy constructor for wxValidator is banned in 2.8.x, so explicitly call constructor, to prevent warning */
                                                                        wxValidator(),
                                                                        m_ids(new int[other.m_count]),
                                                                        m_count(other.m_count),
                                                                        m_msg(other.m_msg),
                                                                        m_title(other.m_title)
{
  memcpy(m_ids, other.m_ids, sizeof(m_ids[0])*m_count);
}

MultiCheckboxValidator::~MultiCheckboxValidator()
{
  delete [] m_ids;
}

wxObject* MultiCheckboxValidator::Clone() const
{
  return new MultiCheckboxValidator(m_ids, m_count, m_msg, m_title);
}

bool MultiCheckboxValidator::Validate(wxWindow* parent)
{
  bool allDisabled = true;
  for(size_t idx = 0; idx < m_count; ++idx) {
    wxWindow* win = GetWindow()->FindWindow(m_ids[idx]);
    if (win) {
      if (win->IsEnabled()) {
        allDisabled = false;
        wxCheckBox* cb = wxDynamicCast(win, wxCheckBox);
        if (cb) {
          if (cb->IsChecked()) {
            return true;
          }
        }
        else {
          wxFAIL_MSG(wxString::Format(wxT("Child(id %d) is not a checkbox"), m_ids[idx]));
        }
      }
    }
    else {
      wxFAIL_MSG(wxString::Format(wxT("No child with id (%d) found in MultiCheckboxValidator"), m_ids[idx]));
    }
  }
  if (allDisabled)
    return true;
  else {
    wxMessageBox(m_msg, m_title, wxOK|wxICON_EXCLAMATION, parent);
    return false;
  }
}

void ShowHideText(wxTextCtrl *&txtCtrl, const wxString &text,
                  wxSizer *sizer, bool show)
{
  wxWindow *parent = txtCtrl->GetParent();
  wxWindowID id = txtCtrl->GetId();
  wxValidator *validator = txtCtrl->GetValidator();

  // Per Dave Silvia's suggestion:
  // Following kludge since wxTE_PASSWORD style is immutable
  wxTextCtrl *tmp = txtCtrl;
  txtCtrl = new wxTextCtrl(parent, id, text,
                           wxDefaultPosition, wxDefaultSize,
                           show ? 0 : wxTE_PASSWORD);
  if (validator != nullptr)
    txtCtrl->SetValidator(*validator);
  ApplyFontPreference(txtCtrl, PWSprefs::StringPrefs::PasswordFont);
  sizer->Replace(tmp, txtCtrl);
  delete tmp;
  sizer->Layout();
  if (!text.IsEmpty()) {
    txtCtrl->ChangeValue(text);
    txtCtrl->SetModified(true);
  }
}

int pless(int* first, int* second) { return *first - *second; }

// Wrapper for wxTaskBarIcon::IsAvailable() that doesn't crash
// on Fedora or Ubuntu
bool IsTaskBarIconAvailable()
{
#if defined(__WXGTK__)
  const wxVersionInfo verInfo = wxGetLibraryVersionInfo();
  int major = verInfo.GetMajor();
  int minor = verInfo.GetMinor();
  int micro = verInfo.GetMicro();
  if (major < 3 || (major == 3 && ((minor == 0 && micro < 4) || (minor == 1 && micro < 1)))) {
    const wxLinuxDistributionInfo ldi = wxGetLinuxDistributionInfo();
    if (ldi.Id.IsEmpty() || ldi.Id == wxT("Ubuntu") || ldi.Id == wxT("Fedora"))
      return false;
  }
#endif
  return wxTaskBarIcon::IsAvailable();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImagePanel Implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

ImagePanel::ImagePanel(wxPanel *parent, const wxSize &size) : wxPanel(parent, wxID_ANY, wxDefaultPosition, size), m_ImageWidth(0), m_ImageHeight(0), m_ImageAspectRatio(0)
{
  Bind(wxEVT_PAINT, &ImagePanel::OnPaint, this);
  Bind(wxEVT_SIZE, &ImagePanel::OnSize, this);
}

ImagePanel::~ImagePanel()
{
  Unbind(wxEVT_PAINT, &ImagePanel::OnPaint, this);
  Unbind(wxEVT_SIZE, &ImagePanel::OnSize, this);
}

bool ImagePanel::LoadFromFile(const wxString &file, wxBitmapType format)
{
  if (m_Image.LoadFile(file, format)) {

    DetermineImageProperties(m_Image);
    Refresh(); // Triggers OnPaint to display the image

    return true;
  }
  else {
    return false;
  }
}

bool ImagePanel::LoadFromMemory(wxInputStream &stream, const wxString &mimetype)
{
  if (m_Image.LoadFile(stream, mimetype)) {

    DetermineImageProperties(m_Image);
    Refresh(); // Triggers OnPaint to display the image
    return true;
  }
  else {
    return false;
  }
}

void ImagePanel::DetermineImageProperties(const wxImage &image)
{
  auto width = image.GetWidth();

  if (width < 0) {
    m_ImageWidth = -1 * width;
  }
  else {
    m_ImageWidth = width;
  }

  auto height = image.GetHeight();

  if (height < 0) {
    m_ImageHeight = -1 * height;
  }
  else {
    m_ImageHeight = height;
  }

  m_ImageAspectRatio = (double)m_ImageWidth / (double)m_ImageHeight;
}

/**
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */
void ImagePanel::OnPaint(wxPaintEvent &event)
{
  // depending on your system you may need to look at double-buffered dcs
  wxPaintDC dc(this);
  Render(dc);
}

/**
 * Here we call refresh to tell the panel to draw itself again.
 * So when the user resizes the image panel the image should be resized too.
 */
void ImagePanel::OnSize(wxSizeEvent &event)
{
  Refresh();
  event.Skip();
}

/**
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void ImagePanel::Paint()
{
  // depending on your system you may need to look at double-buffered dcs
  wxClientDC dc(this);
  Render(dc);
}

void ImagePanel::Clear()
{
  m_Image.Destroy();
}

/**
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void ImagePanel::Render(wxDC &dc)
{
  static const double WIDTH_OR_HEIGHT_FITS_THRESHOLD = 1.0;

  if (!m_Image.IsOk()) {
    return;
  }

  int newWidth = 0, newHeight = 0;

  // Width and height of the drawing area
  int areaWidth = 0, areaHeight = 0;
  dc.GetSize(&areaWidth, &areaHeight);

  if ((areaWidth <= 0) || (areaHeight <= 0)) {
    return;
  }

  // Does the image needs to be scaled to fit into the drawing area?
  if ((m_ImageWidth > areaWidth) || (m_ImageHeight > areaHeight)) {

    /*
      A ratio of less than 1 indicates that the width or height is less than
      the drawing area.
      If the ratio is 1, it means that the width or height exactly matches
      that of the drawing area.
      Only if the ratio is greater than 1 does the width or height of the
      image not fit in the drawing area.

      See constant 'WIDTH_OR_HEIGHT_FITS_THRESHOLD'.
    */
    double widthRatio = (double)m_ImageWidth / (double)areaWidth;
    double heightRatio = (double)m_ImageHeight / (double)areaHeight;

    // Does the image needs scaling in both directions, width and height?
    if ((widthRatio > WIDTH_OR_HEIGHT_FITS_THRESHOLD) && (heightRatio > WIDTH_OR_HEIGHT_FITS_THRESHOLD)) {

      // Limit image to area width and adapt image height by keeping the width height ratio
      if (widthRatio > heightRatio) {
        newWidth = areaWidth;
        newHeight = newWidth / m_ImageAspectRatio;
      }
      // Limit image to area height and adapt image width by keeping the width height ratio
      else if (heightRatio > widthRatio) {
        newHeight = areaHeight;
        newWidth = newHeight * m_ImageAspectRatio;
      }
      // Limit image to area width and height
      else {
        newWidth = areaWidth;
        newHeight = areaHeight;
      }
    }
    // Does the image needs scaling in width, only?
    else if ((widthRatio > WIDTH_OR_HEIGHT_FITS_THRESHOLD) && (heightRatio <= WIDTH_OR_HEIGHT_FITS_THRESHOLD)) {
      newWidth = areaWidth;
      newHeight = newWidth / m_ImageAspectRatio;
    }
    // Does the image needs scaling in height, only?
    else if ((widthRatio <= WIDTH_OR_HEIGHT_FITS_THRESHOLD) && (heightRatio > WIDTH_OR_HEIGHT_FITS_THRESHOLD)) {
      newHeight = areaHeight;
      newWidth = newHeight * m_ImageAspectRatio;
    }
    else {
      // The image already fits in the drawing area, so no actions regarding width and height are required
      ;
    }

    // Limit new values for scaling to prevent assert violations
    if (newWidth < 1) {
      newWidth = 1;
    }

    if (newHeight < 1) {
      newHeight = 1;
    }

    // Scale the image and show it with its new dimensions
    m_Bitmap = wxBitmap(m_Image.Scale(newWidth, newHeight));
    DrawBitmapCentered(dc, wxSize(areaWidth, areaHeight), wxSize(newWidth, newHeight));
  }
  else {
    // No scaling needed, so we use the image with its original dimensions
    m_Bitmap = wxBitmap(m_Image);
    DrawBitmapCentered(dc, wxSize(areaWidth, areaHeight), wxSize(m_ImageWidth, m_ImageHeight));
  }
}

void ImagePanel::DrawBitmapCentered(wxDC &dc, const wxSize &drawAreaSize, const wxSize &imageSize)
{
  int xCenterPosition = (drawAreaSize.GetWidth() - imageSize.GetWidth()) / 2;
  int yCenterPosition = (drawAreaSize.GetHeight() - imageSize.GetHeight()) / 2;

  dc.DrawBitmap(m_Bitmap, xCenterPosition, yCenterPosition, false);
}
