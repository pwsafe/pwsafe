/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

#include <wx/mstream.h>
#include <wx/taskbar.h>
#include <wx/tokenzr.h>
#include <wx/versioninfo.h>

#include "core/PWScore.h"
#include "core/PWCharPool.h" // for CheckMasterPassword()
#include "PWSafeApp.h"
#include "SafeCombinationCtrl.h"

#include "wxUtilities.h"

#include "graphics/cpane.xpm"
#ifndef NO_YUBI
#include "graphics/Yubikey-button.xpm"
#endif

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
  auto rc = othercore.ReadFile(dbpath, combination);

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
        wxMessageBox( wxString(file) << wxT("\n\n") << PWScore::StatusText(rc), _("File Read Error"), wxOK | wxICON_ERROR, msgboxParent);
      break;
  }

  return rc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MultiCheckboxValidator implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

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

void UpdatePasswordTextCtrl(wxSizer *sizer, wxTextCtrl* &textCtrl, const wxString text, wxTextCtrl* before, const int style)
{
  ASSERT(textCtrl);
#if defined(__WXGTK__)
  // Since this function is called with only a single style flag such as "0", "wxTE_PASSWORD" or "wxTE_READONLY",
  // we do not care about flags already set for the control and therefore do not preserve them.
  textCtrl->SetWindowStyle(style);
  textCtrl->ChangeValue(text);
#else
  wxWindow *parent = textCtrl->GetParent();
  wxWindowID id = textCtrl->GetId();
  wxValidator *validator = textCtrl->GetValidator();

  // Per Dave Silvia's suggestion:
  // Following kludge since wxTE_PASSWORD style is immutable
  wxTextCtrl *tmp = textCtrl;
  textCtrl = new wxTextCtrl(parent, id, text,
                           wxDefaultPosition, wxDefaultSize,
                           style);
  if (!text.IsEmpty()) {
    textCtrl->ChangeValue(text);
    textCtrl->SetModified(true);
  }
  if (validator != nullptr) {
    textCtrl->SetValidator(*validator);
  }
  if (before != nullptr) {
    textCtrl->MoveAfterInTabOrder(before);
  }
  ApplyFontPreference(textCtrl, PWSprefs::StringPrefs::PasswordFont);
  sizer->Replace(tmp, textCtrl);
  tmp->Destroy();
  sizer->Layout();
#endif
}

bool CheckPasswordStrengthAndWarn(wxWindow *win, StringX &password)
{
  // Vox populi vox dei - folks want the ability to use a weak
  // passphrase, best we can do is warn them...
  // If someone want to build a version that insists on proper
  // passphrases, then just define the preprocessor macro
  // PWS_FORCE_STRONG_PASSPHRASE in the build properties/Makefile
  StringX errmess;
  if (!CPasswordCharPool::CheckMasterPassword(password, errmess)) {
    wxString cs_msg = errmess.c_str();
#ifndef PWS_FORCE_STRONG_PASSPHRASE
    cs_msg += wxT("\n");
    cs_msg += _("Use it anyway?");
    wxMessageDialog mb(win, cs_msg, _("Weak Master Password"),
                       wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
    mb.SetYesNoLabels(_("Use anyway"), _("Cancel"));
    int rc = mb.ShowModal();
    return (rc == wxID_YES);
#else
    cs_msg += wxT("\n");
    cs_msg += _("Try another");
    wxMessageDialog mb(win, cs_msg, _("Error"), wxOK | wxICON_HAND);
    mb.ShowModal();
    return false;
#endif // PWS_FORCE_STRONG_PASSPHRASE
  }
  return true;
}

SafeCombinationCtrl* wxUtilities::CreateLabeledSafeCombinationCtrl(wxWindow* parent, wxWindowID id, const wxString& label, StringX* password, bool hasFocus)
{
  auto *sizer = new wxBoxSizer(wxVERTICAL);
  parent->GetSizer()->Add(sizer, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 12);

  auto *labelCtrl = new wxStaticText(parent, wxID_STATIC, _(label), wxDefaultPosition, wxDefaultSize, 0);
  sizer->Add(labelCtrl, 0, wxBOTTOM|wxALIGN_LEFT, 5);

  auto *safeCombinationCtrl = new SafeCombinationCtrl(parent, id, password, wxDefaultPosition, wxDefaultSize);
  sizer->Add(safeCombinationCtrl, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 0);

  if (hasFocus) {
    safeCombinationCtrl->SetFocus();
  }

  return safeCombinationCtrl;
}

std::tuple<wxBitmapButton*, wxStaticText*> wxUtilities::CreateYubiKeyControls(wxWindow *parent, wxWindowID buttonId, wxWindowID statusTextId)
{
  auto* panel = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1,  35));
  parent->GetSizer()->Add(panel, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 12);

  auto *sizer = new wxBoxSizer(wxHORIZONTAL);
  panel->SetSizer(sizer);

  auto *button = new wxBitmapButton(panel, buttonId, GetBitmapResource(wxT("graphics/Yubikey-button.xpm")), wxDefaultPosition, wxSize(35,  35), wxBU_AUTODRAW);
  button->SetToolTip(_("YubiKey"));
  sizer->Add(button, 0, wxALL|wxALIGN_CENTER|wxALIGN_LEFT, 0);

  auto *statusText = new wxStaticText(panel, statusTextId, _("Insert YubiKey"), wxDefaultPosition, wxDefaultSize, 0);
  sizer->Add(statusText, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER|wxALIGN_LEFT, 12);

  return std::make_tuple(button, statusText);
}

wxBitmapButton* wxUtilities::GetYubiKeyButtonControl(std::tuple<wxBitmapButton*, wxStaticText*>& controls)
{
  return std::get<wxBitmapButton*>(controls);
}

wxStaticText* wxUtilities::GetYubiKeyStatusControl(std::tuple<wxBitmapButton*, wxStaticText*>& controls)
{
  return std::get<wxStaticText*>(controls);
}

wxBitmap wxUtilities::GetBitmapResource( const wxString& name )
{
  if (name == wxT("graphics/cpane.xpm"))
  {
    wxBitmap bitmap(cpane_xpm);
    return bitmap;
  }
#ifndef NO_YUBI
  else if (name == wxT("graphics/Yubikey-button.xpm"))
  {
    wxBitmap bitmap(Yubikey_button_xpm);
    return bitmap;
  }
#endif
  return wxNullBitmap;
}

int pless(int* first, int* second) { return *first - *second; }

bool IsCurrentDesktopKde()
{
#ifdef __WINDOWS__
  return false;
#else
  wxString currentDesktop = wxEmptyString;

  if (!wxGetEnv(wxT("XDG_CURRENT_DESKTOP"), &currentDesktop)) {
    return false; // Environment variable does not exist
  }

  return (!currentDesktop.IsEmpty() && (currentDesktop.MakeLower().Trim() == wxT("kde")));
#endif
}

bool wxUtilities::IsDisplayManagerX11()
{
  static int isDisplayManagerX11 = 0;

  // Get the env. variable only once
  if (isDisplayManagerX11 == 0) {
    wxString XDG_SESSION_TYPE = wxEmptyString;
    if (wxGetEnv(wxT("XDG_SESSION_TYPE"), &XDG_SESSION_TYPE)) { // provides 'x11' or 'wayland'

      if (!XDG_SESSION_TYPE.IsEmpty() && XDG_SESSION_TYPE == wxT("x11")) {
        isDisplayManagerX11 = 1;
      } else {
        isDisplayManagerX11 = 2; // Don't call wxGetEnv() more than once per process if value is bad
      }
    } else {
      isDisplayManagerX11 = 3; // Don't call wxGetEnv() more than once per process if value is not set/available
    }
  }
  return (isDisplayManagerX11 == 1);
}

bool wxUtilities::IsVirtualKeyboardSupported()
{
#ifdef __WINDOWS__
  return false;
#elif defined __WXOSX__
  return true;
#else
  return wxUtilities::IsDisplayManagerX11();
#endif
}

// Wrapper for wxTaskBarIcon::IsAvailable() that doesn't crash
// on Fedora or Ubuntu
bool IsTaskBarIconAvailable()
{
#if defined(__WXGTK__) && !defined(__OpenBSD__)
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

wxIcon CreateIconWithOverlay(const wxIcon& icon, const wxColour& color, const wxString& text)
{
  auto bitmap = wxBitmap(icon);
  wxImage image = bitmap.ConvertToImage();

  if (!image.HasAlpha())
    image.InitAlpha();

  bitmap = wxBitmap(image);
  wxMemoryDC memoryDC;
  memoryDC.SelectObject(bitmap);
  auto font = memoryDC.GetFont();
  font.MakeLarger();
  font.MakeLarger();
  font.MakeLarger();
  font.MakeBold();
  memoryDC.SetFont(font);
  memoryDC.SetTextForeground(color);
  memoryDC.SetBackgroundMode(wxTRANSPARENT);
  memoryDC.DrawLabel(text, wxRect(bitmap.GetSize()));
  memoryDC.SelectObject(wxNullBitmap);

  wxIcon overLayIcon = wxNullIcon;
  overLayIcon.CopyFromBitmap(bitmap);
  return overLayIcon;
}

/**
 * The following works around a bug in several versions of GTK3 which causes
 * spinbox controls to be displayed incorrectly - too wide or too narrow.
 * 
 * In addition to some heuristics based on the distribution type and version, we allow
 * the user control of the width of the spinbox via the PWS_FIX_GTK_SPINBOX environment variable as follows:
 * 
 * 0 - This is the same as not setting the environment variable, i.e., let PasswordSafe try to determine the correct width
 * 1 - This lets wx set the width to wxDefaultSize, which may be way too wide for some versions of GTK
 * 2..10 - This sets the width to display this many characters in the text entry field of the spinner.
 * 
 * More details:
 * For GTK2, the fixed size wxSize(60, -1) resulted in a suitable width for the control element.
 * Building the application with Gtk3 results in partially hidden controls on the right, where
 * horizontally aligned buttons appear instead of vertically aligned arrows.
 * Choosing wxDefaultSize in this case will result in a text entry field that is much too wide.
 * 
 * @see https://trac.wxwidgets.org/ticket/18568
 */
 
void FixInitialSpinnerSize(wxSpinCtrl* control)
{
  static long spinboxWidthFix = -1L;

  // Get the env. variable only once
  if (spinboxWidthFix == -1L) {
    wxString PWS_FIX_GTK_SPINBOX = wxEmptyString;
    if (wxGetEnv(wxT("PWS_FIX_GTK_SPINBOX"), &PWS_FIX_GTK_SPINBOX)) {

      if (!PWS_FIX_GTK_SPINBOX.IsEmpty() && PWS_FIX_GTK_SPINBOX.ToLong(&spinboxWidthFix)) {
        if (spinboxWidthFix < 0) {
          spinboxWidthFix = 0L;
        } else if (spinboxWidthFix > 10) {
          spinboxWidthFix = 10L;
        }
      } else { // Don't call wxGetEnv() more than once per process if value bad
        spinboxWidthFix = 0L;
      }
    } else { // Don't call wxGetEnv() more than once per process if not set
      spinboxWidthFix = 0L;
    }
  }


  // Set default size
  if (spinboxWidthFix == 1) {
    control->SetInitialSize(wxDefaultSize);
  }
  // Set size according to text width
  else if (spinboxWidthFix > 1) {
    auto text = wxString('0', spinboxWidthFix);

    control->SetInitialSize(
      control->GetSizeFromTextSize(
          control->GetTextExtent(text)
      )
    );
  }

  // Determine necessary text entry field width
  else if (spinboxWidthFix == 0) {
    auto platformInfo = wxPlatformInfo::Get();

    // wxGtk
    if (platformInfo.GetPortId() == wxPortId::wxPORT_GTK) {

      // GTK3 workaround
      if (platformInfo.GetToolkitMajorVersion() >= 3) {

        auto linuxInfo = platformInfo.GetLinuxDistributionInfo();

        if (linuxInfo.Id.IsEmpty() || linuxInfo.Release.IsEmpty()) {
          pws_os::Trace(L"FixInitialSpinnerSize: Consider installing 'lsb_release'.");
          control->SetInitialSize(wxDefaultSize);
        }

        // Fedora 32 with GTK
        else if (
          (linuxInfo.Id.Lower() == wxT("fedora")) &&
          (linuxInfo.Release == wxT("32"))
        ) {
          control->SetInitialSize(wxDefaultSize);
        }

        // Limit the spinners width on any other Linux distribution with GTK
        else {
          auto text = wxT("00");

          control->SetInitialSize(
            control->GetSizeFromTextSize(
                control->GetTextExtent(text)
            )
          );
        }
      }

      // GTK2
      else {
        control->SetInitialSize(wxSize(65, -1));
      }
    }

    // Any other toolkit (wxMSW; wxMac; wxOSX/Carbon; ...)
    else {
      control->SetInitialSize(wxSize(65, -1));
    }
  }

  // Keep the size that was passed to spinners constructor
  else {
    ;
  }
}

/**
 * Returns 'true' if the mime type description begins with 'image'.
 *
 * Example: "image/png", "application/zip"
 */
bool IsMimeTypeImage(const stringT& mimeTypeDescription)
{
  const stringT IMAGE = L"image";

  if (mimeTypeDescription.length() < IMAGE.length()) {
    return false;
  }
  else {
    return (mimeTypeDescription.substr(0, 5) == IMAGE) ? true : false;
  }
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

bool ImagePanel::LoadFromAttachment(const CItemAtt& itemAttachment, wxWindow* parent, const wxString& messageBoxTitle)
{
  auto size = itemAttachment.GetContentSize();

  if (size <= 0) {
    return false;
  }

  unsigned char buffer[size];

  if (!itemAttachment.GetContent(buffer, size)) {
    wxMessageDialog(
      parent,
      _("An error occurred while trying to get the image data from database item.\n"
        "Therefore, the image cannot be displayed in the preview."), messageBoxTitle,
      wxICON_ERROR
    ).ShowModal();

    return false;
  }

  wxMemoryInputStream stream(&buffer, size);

  if (!LoadFromMemory(stream)) {
    wxMessageDialog(
      parent,
      _("An error occurred while trying to load the image data into the preview area.\n"
        "Therefore, the image cannot be displayed in the preview."), messageBoxTitle,
      wxICON_ERROR
    ).ShowModal();

    return false;
  }

  return true;
}

bool ImagePanel::LoadFromFile(const wxString &file, wxBitmapType format)
{
  if (!m_Image.CanRead(file)) {
    return false;
  }

  if (m_Image.LoadFile(file, format)) {

    DetermineImageProperties(m_Image);
    Refresh(); // Triggers OnPaint to display the image

    return true;
  }
  else {
    return false;
  }
}

bool ImagePanel::LoadFromMemory(wxInputStream &stream)
{
  if (!m_Image.CanRead(stream)) {
    return false;
  }

  if (m_Image.LoadFile(stream)) {

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

bool IsCloseInProgress()
{
  return wxGetApp().IsCloseInProgress();
}
