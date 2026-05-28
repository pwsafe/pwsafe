// SPDX-License-Identifier: BSD-3-Clause license

/*
 * Copyright (c) 2025 Anthony Lee Stark. All rights reserved.
 * 
 * This project is based on and includes modified code from:
 * project 'win32-darkmode' by ysc3839 (MIT License),
 * available at: https://github.com/ysc3839/win32-darkmode
 * and project 'darkmodelib' by ozone10 (MPL-2.0 License),
 * available at: https://github.com/ozone10/darkmodelib
 *
 * The respective original licenses apply to portions of this code.
 * See the `licenses/` folder for more information.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *	  list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *	  this list of conditions and the following disclaimer in the documentation
 *	  and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Anthony Lee Stark (@anthonyleestark) nor the names of 
 *	  its contributors may be used to endorse or promote products derived from 
 *	  this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /*
  * This file is based on the Notepad++ dark mode code, which is licensed under GPLv3.
  * Original source by Adam D. Walling (@adzm), with modifications by oZone10 and the Notepad++ team.
  * Further modified by Anthony Lee Stark (@anthonyleestark) in 2025.
  * Used with permission to relicense under the BSD-3-Clause license.
  */


#pragma once
#include <windows.h>

#if (NTDDI_VERSION >= NTDDI_VISTA) /*\
	&& (defined(__x86_64__) || defined(_M_X64)\
	|| defined(__arm64__) || defined(__arm64) || defined(_M_ARM64))*/

#if defined(_MSC_VER)
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Gdi32.lib")
#endif

namespace DarkMode
{
	struct Colors
	{
		COLORREF background = 0;
		COLORREF ctrlBackground = 0;
		COLORREF hotBackground = 0;
		COLORREF dlgBackground = 0;
		COLORREF errorBackground = 0;
		COLORREF text = 0;
		COLORREF darkerText = 0;
		COLORREF disabledText = 0;
		COLORREF linkText = 0;
		COLORREF edge = 0;
		COLORREF hotEdge = 0;
		COLORREF disabledEdge = 0;
	};

	struct ColorsView
	{
		COLORREF background = 0;
		COLORREF text = 0;
		COLORREF gridlines = 0;
		COLORREF headerBackground = 0;
		COLORREF headerHotBackground = 0;
		COLORREF headerText = 0;
		COLORREF headerEdge = 0;
	};

	// unsigned char == std::uint8_t

	/**
	 * @brief Represents tooltip from different controls.
	 */
	enum class ToolTipsType : unsigned char
	{
		tooltip,   ///< Standard tooltip control.
		toolbar,   ///< Tooltips associated with toolbar buttons.
		listview,  ///< Tooltips associated with list views.
		treeview,  ///< Tooltips associated with tree views.
		tabbar,    ///< Tooltips associated with tab controls.
		trackbar,  ///< Tooltips associated with trackbar (slider) controls.
		rebar      ///< Tooltips associated with rebar controls.
	};

	/**
	 * @brief Defines dark mode preset color tones.
	 *
	 * Used as preset to choose default colors in dark mode.
	 * Value `max` is reserved for internal range checking,
	 * do not use in application code.
	 */
	enum class ColorTone : unsigned char
	{
		black   = 0,  ///< Black
		red     = 1,  ///< Red
		green   = 2,  ///< Green
		blue    = 3,  ///< Blue
		purple  = 4,  ///< Purple
		cyan    = 5,  ///< Cyan
		olive   = 6,  ///< Olive
		max     = 7   ///< Don't use, for internal checks
	};

	/**
	 * @brief Defines the available visual styles for TreeView controls.
	 *
	 * Used to control theming behavior for TreeViews:
	 * - `classic`: Legacy style without theming.
	 * - `light`: Light mode appearance.
	 * - `dark`: Dark mode appearance.
	 *
	 * Set via configuration and used by style evaluators (e.g. @ref DarkMode::calculateTreeViewStyle).
	 *
	 * @see DarkMode::calculateTreeViewStyle()
	 */
	enum class TreeViewStyle : unsigned char
	{
		classic,  ///< Non-themed legacy appearance.
		light,    ///< Light mode.
		dark      ///< Dark mode.
	};

	/**
	 * @brief Describes metadata fields and compile-time features of the dark mode library.
	 *
	 * Values of this enum are used with @ref DarkMode::getLibInfo to retrieve version numbers and
	 * determine whether specific features were enabled during compilation.
	 *
	 * @see DarkMode::getLibInfo()
	 */
	enum class LibInfo : unsigned char
	{
		featureCheck,     ///< Returns maxValue to verify enum coverage.
		verMajor,         ///< Major version number of the library.
		verMinor,         ///< Minor version number of the library.
		verRevision,      ///< Revision/patch number of the library.
		iathookExternal,  ///< Indicates if external IAT hooking is used.
		iniConfigUsed,    ///< True if `.ini` file configuration is supported.
		allowOldOS,       ///< True if older Windows versions are allowed.
		useDlgProcCtl,    ///< True if WM_CTLCOLORxxx can be handled directly in dialog procedure.
		preferTheme,      ///< True if theme is supported and can be used over subclass, e.g. combo box on Windows 10+.
		maxValue          ///< Sentinel value for internal validation (not intended for use).
	};

	/**
	 * @brief Defines the available dark mode types for manual configurations.
	 *
	 * Can be used in DarkMode::initDarkModeConfig and in DarkMode::setDarkModeConfig
	 * with static_cast<UINT>(DarkModeType::'value').
	 *
	 * @note Also used internally to distinguish between light, dark, and classic modes.
	 *
	 * @see DarkMode::initDarkModeConfig()
	 * @see DarkMode::setDarkModeConfig()
	 */
	enum class DarkModeType : unsigned char
	{
		light = 0,  ///< Light mode appearance.
		dark = 1,   ///< Dark mode appearance.
		classic = 3 ///< Classic (non-themed or system) appearance.
	};

	/**
	 * @brief Returns library version information or compile-time feature flags.
	 *
	 * @param libInfoType The type of information to query.
	 * @return Integer representing the requested value or feature flag.
	 *
	 * @see LibInfo
	 */
	[[nodiscard]] int getLibInfo(LibInfo libInfoType) noexcept;

	// ========================================================================
	// Config
	// ========================================================================

	/**
	 * @brief Initializes the dark mode configuration based on the selected mode.
	 *
	 * For convenience @ref DarkModeType enums values can be used.
	 *
	 * @param dmType Configuration mode:
	 *        - 0: Light mode
	 *        - 1: Dark mode
	 *        - 3: Classic mode
	 *
	 * @note Values 2 and 4 are reserved for internal use only.
	 *       Using them can cause visual glitches.
	 */
	void initDarkModeConfig(UINT dmType);

	/// Sets the preferred window corner style on Windows 11. (DWM_WINDOW_CORNER_PREFERENCE values)
	void setRoundCornerConfig(UINT roundCornerStyle) noexcept;

	/// Sets the preferred border color for window edge on Windows 11.
	void setBorderColorConfig(COLORREF clr) noexcept;

	// Sets the Mica effects on Windows 11 setting. (DWM_SYSTEMBACKDROP_TYPE values)
	void setMicaConfig(UINT mica) noexcept;

	/// Sets Mica effects on the full window setting.
	void setMicaExtendedConfig(bool extendMica) noexcept;

	/// Sets dialog colors on title bar on Windows 11 setting.
	void setColorizeTitleBarConfig(bool colorize) noexcept;

	/// Applies dark mode settings based on the given configuration type. (DarkModeType values)
	void setDarkModeConfig(UINT dmType);

	/// Applies dark mode settings based on system mode preference.
	void setDarkModeConfig();

	/// Initializes dark mode experimental features, colors, and other settings.
	void initDarkMode(const wchar_t* iniName);

	///Initializes dark mode without INI settings.
	void initDarkMode();

	// ========================================================================
	// Basic checks
	// ========================================================================

	/// Checks if non-classic mode is enabled.
	[[nodiscard]] bool isEnabled() noexcept;

	/// Checks if experimental dark mode features are currently active.
	[[nodiscard]] bool isExperimentalActive() noexcept;

	/// Checks if experimental dark mode features are supported by the system.
	[[nodiscard]] bool isExperimentalSupported() noexcept;

	/// Checks if follow the system mode behavior is enabled.
	[[nodiscard]] bool isWindowsModeEnabled() noexcept;

	/// Checks if the host OS is at least Windows 10.
	[[nodiscard]] bool isAtLeastWindows10() noexcept;

	/// Checks if the host OS is at least Windows 11.
	[[nodiscard]] bool isAtLeastWindows11() noexcept;

	/// Retrieves the current Windows build number.
	[[nodiscard]] DWORD getWindowsBuildNumber() noexcept;

	// ========================================================================
	// System Events
	// ========================================================================

	/// Handles system setting changes related to dark mode.
	bool handleSettingChange(LPARAM lParam);

	/// Checks if dark mode is enabled in the Windows registry.
	[[nodiscard]] bool isDarkModeReg();

	// ========================================================================
	// From DarkMode.h
	// ========================================================================

	/**
	 * @brief Overrides a specific system color with a custom color.
	 *
	 * Currently supports:
	 * - `COLOR_WINDOW`: Background of ComboBoxEx list.
	 * - `COLOR_WINDOWTEXT`: Text color of ComboBoxEx list.
	 * - `COLOR_BTNFACE`: Gridline color in ListView (when applicable).
	 *
	 * @param nIndex One of the supported system color indices.
	 * @param color Custom `COLORREF` value to apply.
	 */
	void setSysColor(int nIndex, COLORREF color) noexcept;

	// ========================================================================
	// Enhancements to DarkMode.h
	// ========================================================================

	/// Makes scroll bars on the specified window and all its children consistent.
	void enableDarkScrollBarForWindowAndChildren(HWND hWnd);

	// ========================================================================
	// Colors
	// ========================================================================

	/// Sets the color tone and its color set for the active theme.
	void setColorTone(ColorTone colorTone) noexcept;

	/// Retrieves the currently active color tone for the theme.
	[[nodiscard]] ColorTone getColorTone() noexcept;

	COLORREF setBackgroundColor(COLORREF clrNew);
	COLORREF setCtrlBackgroundColor(COLORREF clrNew);
	COLORREF setHotBackgroundColor(COLORREF clrNew);
	COLORREF setDlgBackgroundColor(COLORREF clrNew);
	COLORREF setErrorBackgroundColor(COLORREF clrNew);

	COLORREF setTextColor(COLORREF clrNew);
	COLORREF setDarkerTextColor(COLORREF clrNew);
	COLORREF setDisabledTextColor(COLORREF clrNew);
	COLORREF setLinkTextColor(COLORREF clrNew);

	COLORREF setEdgeColor(COLORREF clrNew);
	COLORREF setHotEdgeColor(COLORREF clrNew);
	COLORREF setDisabledEdgeColor(COLORREF clrNew);

	void setThemeColors(Colors colors) noexcept;
	void updateThemeBrushesAndPens() noexcept;

	[[nodiscard]] COLORREF getBackgroundColor() noexcept;
	[[nodiscard]] COLORREF getCtrlBackgroundColor() noexcept;
	[[nodiscard]] COLORREF getHotBackgroundColor() noexcept;
	[[nodiscard]] COLORREF getDlgBackgroundColor() noexcept;
	[[nodiscard]] COLORREF getErrorBackgroundColor() noexcept;

	[[nodiscard]] COLORREF getTextColor() noexcept;
	[[nodiscard]] COLORREF getDarkerTextColor() noexcept;
	[[nodiscard]] COLORREF getDisabledTextColor() noexcept;
	[[nodiscard]] COLORREF getLinkTextColor() noexcept;

	[[nodiscard]] COLORREF getEdgeColor() noexcept;
	[[nodiscard]] COLORREF getHotEdgeColor() noexcept;
	[[nodiscard]] COLORREF getDisabledEdgeColor() noexcept;

	[[nodiscard]] HBRUSH getBackgroundBrush() noexcept;
	[[nodiscard]] HBRUSH getDlgBackgroundBrush() noexcept;
	[[nodiscard]] HBRUSH getCtrlBackgroundBrush() noexcept;
	[[nodiscard]] HBRUSH getHotBackgroundBrush() noexcept;
	[[nodiscard]] HBRUSH getErrorBackgroundBrush() noexcept;

	[[nodiscard]] HBRUSH getEdgeBrush() noexcept;
	[[nodiscard]] HBRUSH getHotEdgeBrush() noexcept;
	[[nodiscard]] HBRUSH getDisabledEdgeBrush() noexcept;

	[[nodiscard]] HPEN getDarkerTextPen() noexcept;
	[[nodiscard]] HPEN getEdgePen() noexcept;
	[[nodiscard]] HPEN getHotEdgePen() noexcept;
	[[nodiscard]] HPEN getDisabledEdgePen() noexcept;

	COLORREF setViewBackgroundColor(COLORREF clrNew);
	COLORREF setViewTextColor(COLORREF clrNew);
	COLORREF setViewGridlinesColor(COLORREF clrNew);

	COLORREF setHeaderBackgroundColor(COLORREF clrNew);
	COLORREF setHeaderHotBackgroundColor(COLORREF clrNew);
	COLORREF setHeaderTextColor(COLORREF clrNew);
	COLORREF setHeaderEdgeColor(COLORREF clrNew);

	void setViewColors(ColorsView colors);
	void updateViewBrushesAndPens();

	[[nodiscard]] COLORREF getViewBackgroundColor() noexcept;
	[[nodiscard]] COLORREF getViewTextColor() noexcept;
	[[nodiscard]] COLORREF getViewGridlinesColor() noexcept;

	[[nodiscard]] COLORREF getHeaderBackgroundColor() noexcept;
	[[nodiscard]] COLORREF getHeaderHotBackgroundColor() noexcept;
	[[nodiscard]] COLORREF getHeaderTextColor() noexcept;
	[[nodiscard]] COLORREF getHeaderEdgeColor() noexcept;

	[[nodiscard]] HBRUSH getViewBackgroundBrush() noexcept;
	[[nodiscard]] HBRUSH getViewGridlinesBrush() noexcept;

	[[nodiscard]] HBRUSH getHeaderBackgroundBrush() noexcept;
	[[nodiscard]] HBRUSH getHeaderHotBackgroundBrush() noexcept;

	[[nodiscard]] HPEN getHeaderEdgePen() noexcept;

	/// Initializes default color set based on the current mode type.
	void setDefaultColors(bool updateBrushesAndOther);

	// ========================================================================
	// Paint Helpers
	// ========================================================================

	/// Paints a rounded rectangle using the specified pen and brush.
	void paintRoundRect(HDC hdc, const RECT& rect, HPEN hpen, HBRUSH hBrush, int width = 0, int height = 0);
	/// Paints an unfilled rounded rectangle (frame only).
	void paintRoundFrameRect(HDC hdc, const RECT& rect, HPEN hpen, int width = 0, int height = 0);

	// ========================================================================
	// Control Subclassing
	// ========================================================================

	/// Applies themed owner drawn subclassing to a checkbox, radio, or tri-state button control.
	void setCheckboxOrRadioBtnCtrlSubclass(HWND hWnd);
	/// Removes the owner drawn subclass from a a checkbox, radio, or tri-state button control.
	void removeCheckboxOrRadioBtnCtrlSubclass(HWND hWnd);

	/// Applies owner drawn subclassing to a groupbox button control.
	void setGroupboxCtrlSubclass(HWND hWnd);
	/// Removes the owner drawn subclass from a groupbox button control.
	void removeGroupboxCtrlSubclass(HWND hWnd);

	/// Applies owner drawn subclassing and theming to an updown (spinner) control.
	void setUpDownCtrlSubclass(HWND hWnd);
	/// Removes the owner drawn subclass from a updown (spinner) control.
	void removeUpDownCtrlSubclass(HWND hWnd);

	void setTabCtrlUpDownSubclass(HWND hWnd);
	void removeTabCtrlUpDownSubclass(HWND hWnd);
	void setTabCtrlSubclass(HWND hWnd);
	void removeTabCtrlSubclass(HWND hWnd);

	void setCustomBorderForListBoxOrEditCtrlSubclass(HWND hWnd);
	void removeCustomBorderForListBoxOrEditCtrlSubclass(HWND hWnd);

	void setComboBoxCtrlSubclass(HWND hWnd);
	void removeComboBoxCtrlSubclass(HWND hWnd);

	void setComboBoxExCtrlSubclass(HWND hWnd);
	void removeComboBoxExCtrlSubclass(HWND hWnd);

	void setListViewCtrlSubclass(HWND hWnd);
	void removeListViewCtrlSubclass(HWND hWnd);

	void setHeaderCtrlSubclass(HWND hWnd);
	void removeHeaderCtrlSubclass(HWND hWnd);

	void setStatusBarCtrlSubclass(HWND hWnd);
	void removeStatusBarCtrlSubclass(HWND hWnd);

	void setProgressBarCtrlSubclass(HWND hWnd);
	void removeProgressBarCtrlSubclass(HWND hWnd);

	void setStaticTextCtrlSubclass(HWND hWnd);
	void removeStaticTextCtrlSubclass(HWND hWnd);

	// ========================================================================
	// Child Subclassing
	// ========================================================================

	void setChildCtrlsSubclassAndTheme(HWND hParent, bool subclass = true, bool theme = true);
	void setChildCtrlsTheme(HWND hParent);

	// ========================================================================
	// Window, Parent, And Other Subclassing
	// ========================================================================

	/// Applies window subclassing to handle `WM_ERASEBKGND` message.
	void setWindowEraseBgSubclass(HWND hWnd);
	/// Removes the subclass used for `WM_ERASEBKGND` message handling.
	void removeWindowEraseBgSubclass(HWND hWnd);

	/// Applies window subclassing to handle `WM_CTLCOLOR*` messages.
	void setWindowCtlColorSubclass(HWND hWnd);
	/// Removes the subclass used for `WM_CTLCOLOR*` messages handling.
	void removeWindowCtlColorSubclass(HWND hWnd);

	/// Applies window subclassing for handling `NM_CUSTOMDRAW` notifications for custom drawing.
	void setWindowNotifyCustomDrawSubclass(HWND hWnd);
	/// Removes the subclass used for handling `NM_CUSTOMDRAW` notifications for custom drawing.
	void removeWindowNotifyCustomDrawSubclass(HWND hWnd);

	/// Applies window subclassing for menu bar themed custom drawing.
	void setWindowMenuBarSubclass(HWND hWnd);
	/// Removes the subclass used for menu bar themed custom drawing.
	void removeWindowMenuBarSubclass(HWND hWnd);

	/// Applies window subclassing to handle `WM_SETTINGCHANGE` message.
	void setWindowSettingChangeSubclass(HWND hWnd);
	/// Removes the subclass used for `WM_SETTINGCHANGE` message handling.
	void removeWindowSettingChangeSubclass(HWND hWnd);

	// ========================================================================
	// Theme And Helpers
	// ========================================================================

	/// Configures the SysLink control to be affected by `WM_CTLCOLORSTATIC` message.
	void enableSysLinkCtrlCtlColor(HWND hWnd);

	/// Sets dark title bar and optional Windows 11 features.
	void setDarkTitleBarEx(HWND hWnd, bool useWin11Features);
	/// Sets dark mode title bar on supported Windows versions.
	void setDarkTitleBar(HWND hWnd);

	/// Applies an experimental visual style to the specified window, if supported.
	void setDarkThemeExperimental(HWND hWnd, const wchar_t* themeClassName = L"Explorer");
	/// Applies "DarkMode_Explorer" visual style if experimental mode is active.
	void setDarkExplorerTheme(HWND hWnd);
	/// Applies "DarkMode_Explorer" visual style to scroll bars.
	void setDarkScrollBar(HWND hWnd);
	/// Applies "DarkMode_Explorer" visual style to tooltip controls based on context.
	void setDarkTooltips(HWND hWnd, ToolTipsType type = ToolTipsType::tooltip);

	/// Sets the color of line above a toolbar control for non-classic mode.
	void setDarkLineAbovePanelToolbar(HWND hWnd);
	/// Applies an experimental Explorer visual style to a list view.
	void setDarkListView(HWND hWnd);
	/// Replaces default list view checkboxes with themed dark-mode versions on Windows 11.
	void setDarkListViewCheckboxes(HWND hWnd);
	/// Sets colors and edges for a RichEdit control.
	void setDarkRichEdit(HWND hWnd);

	/// Applies visual styles; ctl color message and child controls subclassings to a window safely.
	void setDarkWndSafe(HWND hWnd, bool useWin11Features = true);
	/// Applies visual styles; ctl color message, child controls, custom drawing, and setting change subclassings to a window safely.
	void setDarkWndNotifySafeEx(HWND hWnd, bool setSettingChangeSubclass, bool useWin11Features);
	/// Applies visual styles; ctl color message, child controls, and custom drawing subclassings to a window safely.
	void setDarkWndNotifySafe(HWND hWnd, bool useWin11Features = true);

	/// Enables or disables theme-based dialog background textures in classic mode.
	void enableThemeDialogTexture(HWND hWnd, bool theme);

	/// Enables or disables visual styles for a window.
	void disableVisualStyle(HWND hWnd, bool doDisable);

	/// Calculates perceptual lightness of a COLORREF color.
	[[nodiscard]] double calculatePerceivedLightness(COLORREF clr) noexcept;

	/// Retrieves the current TreeView style configuration.
	[[nodiscard]] const TreeViewStyle& getTreeViewStyle() noexcept;

	/// Determines appropriate TreeView style based on background perceived lightness.
	void calculateTreeViewStyle();

	/// Applies the appropriate window theme style to the specified TreeView.
	void setTreeViewWindowTheme(HWND hWnd, bool force = false);

	/// Retrieves the previous TreeView style configuration.
	[[nodiscard]] const TreeViewStyle& getPrevTreeViewStyle() noexcept;

	/// Stores the current TreeView style as the previous style for later comparison.
	void setPrevTreeViewStyle() noexcept;

	/// Checks whether the current theme is dark.
	[[nodiscard]] bool isThemeDark() noexcept;

	/// Checks whether the color is dark.
	[[nodiscard]] bool isColorDark(COLORREF clr) noexcept;

	/// Forces a window to redraw its non-client frame.
	void redrawWindowFrame(HWND hWnd);
	/// Sets a window's standard style flags and redraws window if needed.
	void setWindowStyle(HWND hWnd, bool setStyle, LONG_PTR styleFlag);
	/// Sets a window's extended style flags and redraws window if needed.
	void setWindowExStyle(HWND hWnd, bool setExStyle, LONG_PTR exStyleFlag);
	/// Replaces an extended edge (e.g. client edge) with a standard window border.
	void replaceExEdgeWithBorder(HWND hWnd, bool replace, LONG_PTR exStyleFlag);
	/// Safely toggles `WS_EX_CLIENTEDGE` with `WS_BORDER` based on dark mode state.
	void replaceClientEdgeWithBorderSafe(HWND hWnd);

	/// Applies classic-themed styling to a progress bar in non-classic mode.
	void setProgressBarClassicTheme(HWND hWnd);

	// ========================================================================
	// Ctl Color
	// ========================================================================

	/// Handles text and background colorizing for read-only controls.
	[[nodiscard]] LRESULT onCtlColor(HDC hdc);

	/// Handles text and background colorizing for interactive controls.
	[[nodiscard]] LRESULT onCtlColorCtrl(HDC hdc);

	/// Handles text and background colorizing for window and disabled non-text controls.
	[[nodiscard]] LRESULT onCtlColorDlg(HDC hdc);

	/// Handles text and background colorizing for error state (for specific usage).
	[[nodiscard]] LRESULT onCtlColorError(HDC hdc);

	/// Handles text and background colorizing for static text controls.
	[[nodiscard]] LRESULT onCtlColorDlgStaticText(HDC hdc, bool isTextEnabled);

	/// Handles text and background colorizing for syslink controls.
	[[nodiscard]] LRESULT onCtlColorDlgLinkText(HDC hdc, bool isTextEnabled = true);

	/// Handles text and background colorizing for list box controls.
	[[nodiscard]] LRESULT onCtlColorListbox(WPARAM wParam, LPARAM lParam);

	// ========================================================================
	// Darkmode-aware MessageBox
	// ========================================================================
	int DarkMessageBox(HWND owner, LPCWSTR text, LPCWSTR caption, UINT type);

	// ========================================================================
	// Hook Callback Dialog Procedure
	// ========================================================================

	/**
	 * @brief Hook procedure for customizing common dialogs with dark mode.
	 *
	 * This function handles messages for all Windows common dialogs.
	 * When initialized (`WM_INITDIALOG`), it applies dark mode styling to the dialog.
	 *
	 * ## Special Case: Font Dialog Workaround
	 * - When a hook is used with `ChooseFont`, Windows **automatically falls back**
	 *   to an **older template**, losing modern UI elements.
	 * - To prevent this forced downgrade, a **modified template** (based on Font.dlg) is used.
	 * - **CBS_OWNERDRAWFIXED should be removed** from the **Size** and **Script** combo boxes
	 *   to restore proper visualization.
	 * - **Custom owner-draw visuals remain** for other font combo boxes to allow font preview.
	 * - Same for the `"AaBbYyZz"` sample text.
	 * - However **Automatic system translation for captions and static texts is lost** in this workaround.
	 *
	 * ## Custom Font Dialog Template (Resource File)
	 * ```rc
	 * IDD_DARK_FONT_DIALOG DIALOG 13, 54, 243, 234
	 * STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU |
	 *       DS_3DLOOK
	 * CAPTION "Font"
	 * FONT 9, "Segoe UI"
	 * BEGIN
	 *     LTEXT           "&Font:", stc1, 7, 7, 98, 9
	 *     COMBOBOX        cmb1, 7, 16, 98, 76,
	 *                     CBS_SIMPLE | CBS_AUTOHSCROLL | CBS_DISABLENOSCROLL |
	 *                     CBS_SORT | WS_VSCROLL | WS_TABSTOP | CBS_HASSTRINGS |
	 *                     CBS_OWNERDRAWFIXED
	 *
	 *     LTEXT           "Font st&yle:", stc2, 114, 7, 74, 9
	 *     COMBOBOX        cmb2, 114, 16, 74, 76,
	 *                     CBS_SIMPLE | CBS_AUTOHSCROLL | CBS_DISABLENOSCROLL |
	 *                     WS_VSCROLL | WS_TABSTOP | CBS_HASSTRINGS |
	 *                     CBS_OWNERDRAWFIXED
	 *
	 *     LTEXT           "&Size:", stc3, 198, 7, 36, 9
	 *     COMBOBOX        cmb3, 198, 16, 36, 76,
	 *                     CBS_SIMPLE | CBS_AUTOHSCROLL | CBS_DISABLENOSCROLL |
	 *                     CBS_SORT | WS_VSCROLL | WS_TABSTOP | CBS_HASSTRINGS |
	 *                     CBS_OWNERDRAWFIXED // remove CBS_OWNERDRAWFIXED
	 *
	 *     GROUPBOX        "Effects", grp1, 7, 97, 98, 76, WS_GROUP
	 *     AUTOCHECKBOX    "Stri&keout", chx1, 13, 111, 90, 10, WS_TABSTOP
	 *     AUTOCHECKBOX    "&Underline", chx2, 13, 127, 90, 10
	 *
	 *     LTEXT           "&Color:", stc4, 13, 144, 89, 9
	 *     COMBOBOX        cmb4, 13, 155, 85, 100,
	 *                     CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_AUTOHSCROLL |
	 *                     CBS_HASSTRINGS | WS_BORDER | WS_VSCROLL | WS_TABSTOP
	 *
	 *     GROUPBOX        "Sample", grp2, 114, 97, 120, 43, WS_GROUP
	 *     CTEXT           "AaBbYyZz", stc5, 116, 106, 117, 33,
	 *                     SS_NOPREFIX | NOT WS_VISIBLE
	 *     LTEXT           "", stc6, 7, 178, 227, 20, SS_NOPREFIX | NOT WS_GROUP
	 *
	 *     LTEXT           "Sc&ript:", stc7, 114, 145, 118, 9
	 *     COMBOBOX        cmb5, 114, 155, 120, 30, CBS_DROPDOWNLIST |
	 *                     CBS_OWNERDRAWFIXED | CBS_AUTOHSCROLL | CBS_HASSTRINGS | // remove CBS_OWNERDRAWFIXED
	 *                     WS_BORDER | WS_VSCROLL | WS_TABSTOP
	 *
	 *     CONTROL         "<A>Show more fonts</A>", IDC_MANAGE_LINK, "SysLink",
	 *                     WS_TABSTOP, 7, 199, 227, 9
	 *
	 *     DEFPUSHBUTTON   "OK", IDOK, 141, 215, 45, 14, WS_GROUP
	 *     PUSHBUTTON      "Cancel", IDCANCEL, 190, 215, 45, 14, WS_GROUP
	 *     PUSHBUTTON      "&Apply", psh3, 92, 215, 45, 14, WS_GROUP
	 *     PUSHBUTTON      "&Help", pshHelp, 43, 215, 45, 14, WS_GROUP
	 * END
	 * ```
	 *
	 * ## Usage Example:
	 * ```cpp
	 * #define IDD_DARK_FONT_DIALOG 1000 // usually in resource.h or other header
	 *
	 * CHOOSEFONT cf{};
	 * cf.Flags |= CF_ENABLEHOOK | CF_ENABLETEMPLATE;
	 * cf.lpfnHook = static_cast<LPCFHOOKPROC>(DarkMode::HookDlgProc);
	 * cf.hInstance = GetModuleHandle(nullptr);
	 * cf.lpTemplateName = MAKEINTRESOURCE(IDD_DARK_FONT_DIALOG);
	 * ```
	 *
	 * @param hWnd Handle to the dialog window.
	 * @param uMsg Message identifier.
	 * @param wParam First message parameter (unused).
	 * @param lParam Second message parameter (unused).
	 * @return A value defined by the hook procedure.
	 */
	UINT_PTR CALLBACK HookDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
} // namespace DarkMode

#else
#define _DARKMODE_NOT_USED
#endif // (NTDDI_VERSION >= NTDDI_VISTA) //&& (x64 or arm64)
