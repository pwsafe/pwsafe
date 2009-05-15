/*

This is a modified version of code published by Michael Kaplan in his
series of MSDN Blogs "Sorting it all Out, Getting all you can out of a
keyboard layout" (Part 0 to Part 9b) between March 23 and April 13, 2006.

See http://blogs.msdn.com/michkap/archive/2006/04/13/575500.aspx

This code has been modified for Password Safe needs to generate include
files for Password Safe so that any keyboard supported by Windows can be
displayed on an On-screen Keyboard irrespective as to whether that
keyboard is installed on the user's PC.

It also accepts 'Custom' keyboard definitions via XML.

*/
using System;
using System.Text;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO;
using System.Globalization;
using System.Threading;
using System.Xml;
using System.Xml.Schema;
using System.Diagnostics;

using Microsoft.Win32;

namespace KeyboardLayouts {
  public enum KeysEx : byte {
    None                   = 0x00,
    VK_LBUTTON             = Keys.LButton,             // 0x01
    VK_RBUTTON             = Keys.RButton,             // 0x02
    VK_CANCEL              = Keys.Cancel,              // 0x03
    VK_MBUTTON             = Keys.MButton,             // 0x04
    VK_XBUTTON1            = Keys.XButton1,            // 0x05
    VK_XBUTTON2            = Keys.XButton2,            // 0x06
    /* 0x07 : unassigned */
    VK_BACK                = Keys.Back,                // 0x08
    VK_TAB                 = Keys.Tab,                 // 0x09
    /* 0x0A - 0x0B : reserved */
    VK_CLEAR               = Keys.Clear,               // 0x0C
    VK_RETURN              = Keys.Return,              // 0x0D, Keys.Enter
    VK_SHIFT               = Keys.ShiftKey,            // 0x10
    VK_CONTROL             = Keys.ControlKey,          // 0x11
    VK_MENU                = Keys.Menu,                // 0x12
    VK_PAUSE               = Keys.Pause,               // 0x13
    VK_CAPITAL             = Keys.Capital,             // 0x14, Keys.CapsLock
    VK_KANA                = Keys.KanaMode,            // 0x15
    VK_HANGEUL             = Keys.HanguelMode,         // 0x15, Keys.HangulMode
    VK_JUNJA               = Keys.JunjaMode,           // 0x17
    VK_FINAL               = Keys.FinalMode,           // 0x18
    VK_HANJA               = Keys.HanjaMode,           // 0x19
    VK_KANJI               = Keys.KanjiMode,           // 0x19
    VK_ESCAPE              = Keys.Escape,              // 0x1B
    VK_CONVERT             = Keys.IMEConvert,          // 0x1C
    VK_NONCONVERT          = Keys.IMENonconvert,       // 0x1D
    VK_ACCEPT              = Keys.IMEAceept,           // 0x1E, Keys.IMEAccept
    VK_MODECHANGE          = Keys.IMEModeChange,       // 0x1F
    VK_SPACE               = Keys.Space,               // 0x20
    VK_PRIOR               = Keys.Prior,               // 0x21, Keys.PageUp
    VK_NEXT                = Keys.Next,                // 0x22, Keys.PageDown
    VK_END                 = Keys.End,                 // 0x23
    VK_HOME                = Keys.Home,                // 0x24
    VK_LEFT                = Keys.Left,                // 0x25
    VK_UP                  = Keys.Up,                  // 0x26
    VK_RIGHT               = Keys.Right,               // 0x27
    VK_DOWN                = Keys.Down,                // 0x28
    VK_SELECT              = Keys.Select,              // 0x29
    VK_PRINT               = Keys.Print,               // 0x2A
    VK_EXECUTE             = Keys.Execute,             // 0x2B
    VK_SNAPSHOT            = Keys.Snapshot,            // 0x2C, Keys.PrintScreen
    VK_INSERT              = Keys.Insert,              // 0x2D
    VK_DELETE              = Keys.Delete,              // 0x2E
    VK_HELP                = Keys.Help,                // 0x2F
    VK_0                   = Keys.D0,                  // 0x30
    VK_1                   = Keys.D1,                  // 0x31
    VK_2                   = Keys.D2,                  // 0x32
    VK_3                   = Keys.D3,                  // 0x33
    VK_4                   = Keys.D4,                  // 0x34
    VK_5                   = Keys.D5,                  // 0x35
    VK_6                   = Keys.D6,                  // 0x36
    VK_7                   = Keys.D7,                  // 0x37
    VK_8                   = Keys.D8,                  // 0x38
    VK_9                   = Keys.D9,                  // 0x39
    /* 0x40 : unassigned */
    VK_A                   = Keys.A,                   // 0x41
    VK_B                   = Keys.B,                   // 0x42
    VK_C                   = Keys.C,                   // 0x43
    VK_D                   = Keys.D,                   // 0x44
    VK_E                   = Keys.E,                   // 0x45
    VK_F                   = Keys.F,                   // 0x46
    VK_G                   = Keys.G,                   // 0x47
    VK_H                   = Keys.H,                   // 0x48
    VK_I                   = Keys.I,                   // 0x49
    VK_J                   = Keys.J,                   // 0x4A
    VK_K                   = Keys.K,                   // 0x4B
    VK_L                   = Keys.L,                   // 0x4C
    VK_M                   = Keys.M,                   // 0x4D
    VK_N                   = Keys.N,                   // 0x4E
    VK_O                   = Keys.O,                   // 0x4F
    VK_P                   = Keys.P,                   // 0x50
    VK_Q                   = Keys.Q,                   // 0x51
    VK_R                   = Keys.R,                   // 0x52
    VK_S                   = Keys.S,                   // 0x53
    VK_T                   = Keys.T,                   // 0x54
    VK_U                   = Keys.U,                   // 0x55
    VK_V                   = Keys.V,                   // 0x56
    VK_W                   = Keys.W,                   // 0x57
    VK_X                   = Keys.X,                   // 0x58
    VK_Y                   = Keys.Y,                   // 0x59
    VK_Z                   = Keys.Z,                   // 0x5A
    VK_LWIN                = Keys.LWin,                // 0x5B
    VK_RWIN                = Keys.RWin,                // 0x5C
    VK_APPS                = Keys.Apps,                // 0x5D
    /* 0x5E : reserved */
    VK_SLEEP               = 0x5f,                     // 0x5f, Keys.Sleep
    VK_NUMPAD0             = Keys.NumPad0,             // 0x60
    VK_NUMPAD1             = Keys.NumPad1,             // 0x61
    VK_NUMPAD2             = Keys.NumPad2,             // 0x62
    VK_NUMPAD3             = Keys.NumPad3,             // 0x63
    VK_NUMPAD4             = Keys.NumPad4,             // 0x64
    VK_NUMPAD5             = Keys.NumPad5,             // 0x65
    VK_NUMPAD6             = Keys.NumPad6,             // 0x66
    VK_NUMPAD7             = Keys.NumPad7,             // 0x67
    VK_NUMPAD8             = Keys.NumPad8,             // 0x68
    VK_NUMPAD9             = Keys.NumPad9,             // 0x69
    VK_MULTIPLY            = Keys.Multiply,            // 0x6A
    VK_ADD                 = Keys.Add,                 // 0x6B
    VK_SEPARATOR           = Keys.Separator,           // 0x6C
    VK_SUBTRACT            = Keys.Subtract,            // 0x6D
    VK_DECIMAL             = Keys.Decimal,             // 0x6E
    VK_DIVIDE              = Keys.Divide,              // 0x6F
    VK_F1                  = Keys.F1,                  // 0x70
    VK_F2                  = Keys.F2,                  // 0x71
    VK_F3                  = Keys.F3,                  // 0x72
    VK_F4                  = Keys.F4,                  // 0x73
    VK_F5                  = Keys.F5,                  // 0x74
    VK_F6                  = Keys.F6,                  // 0x75
    VK_F7                  = Keys.F7,                  // 0x76
    VK_F8                  = Keys.F8,                  // 0x77
    VK_F9                  = Keys.F9,                  // 0x78
    VK_F10                 = Keys.F10,                 // 0x79
    VK_F11                 = Keys.F11,                 // 0x7A
    VK_F12                 = Keys.F12,                 // 0x7B
    VK_F13                 = Keys.F13,                 // 0x7C
    VK_F14                 = Keys.F14,                 // 0x7D
    VK_F15                 = Keys.F15,                 // 0x7E
    VK_F16                 = Keys.F16,                 // 0x7F
    VK_F17                 = Keys.F17,                 // 0x80
    VK_F18                 = Keys.F18,                 // 0x81
    VK_F19                 = Keys.F19,                 // 0x82
    VK_F20                 = Keys.F20,                 // 0x83
    VK_F21                 = Keys.F21,                 // 0x84
    VK_F22                 = Keys.F22,                 // 0x85
    VK_F23                 = Keys.F23,                 // 0x86
    VK_F24                 = Keys.F24,                 // 0x87
    /* 0x88 - 0x8F : unassigned */
    VK_NUMLOCK             = Keys.NumLock,             // 0x90
    VK_SCROLL              = Keys.Scroll,              // 0x91
    VK_OEM_NEC_EQUAL       = 0x92,                     // 0x92, NEC PC-9800 kbd definition
    VK_OEM_FJ_JISHO        = 0x92,                     // 0x92, Fujitsu/OASYS kbd definition
    VK_OEM_FJ_MASSHOU      = 0x93,                     // 0x93, Fujitsu/OASYS kbd definition
    VK_OEM_FJ_TOUROKU      = 0x94,                     // 0x94, Fujitsu/OASYS kbd definition
    VK_OEM_FJ_LOYA         = 0x95,                     // 0x95, Fujitsu/OASYS kbd definition
    VK_OEM_FJ_ROYA         = 0x96,                     // 0x96, Fujitsu/OASYS kbd definition
    /* 0x97 - 0x9F : unassigned */
    VK_LSHIFT              = Keys.LShiftKey,           // 0xA0
    VK_RSHIFT              = Keys.RShiftKey,           // 0xA1
    VK_LCONTROL            = Keys.LControlKey,         // 0xA2
    VK_RCONTROL            = Keys.RControlKey,         // 0xA3
    VK_LMENU               = Keys.LMenu,               // 0xA4
    VK_RMENU               = Keys.RMenu,               // 0xA5
    VK_BROWSER_BACK        = Keys.BrowserBack,         // 0xA6
    VK_BROWSER_FORWARD     = Keys.BrowserForward,      // 0xA7
    VK_BROWSER_REFRESH     = Keys.BrowserRefresh,      // 0xA8
    VK_BROWSER_STOP        = Keys.BrowserStop,         // 0xA9
    VK_BROWSER_SEARCH      = Keys.BrowserSearch,       // 0xAA
    VK_BROWSER_FAVORITES   = Keys.BrowserFavorites,    // 0xAB
    VK_BROWSER_HOME        = Keys.BrowserHome,         // 0xAC
    VK_VOLUME_MUTE         = Keys.VolumeMute,          // 0xAD
    VK_VOLUME_DOWN         = Keys.VolumeDown,          // 0xAE
    VK_VOLUME_UP           = Keys.VolumeUp,            // 0xAF
    VK_MEDIA_NEXT_TRACK    = Keys.MediaNextTrack,      // 0xB0
    VK_MEDIA_PREV_TRACK    = Keys.MediaPreviousTrack,  // 0xB1
    VK_MEDIA_STOP          = Keys.MediaStop,           // 0xB2
    VK_MEDIA_PLAY_PAUSE    = Keys.MediaPlayPause,      // 0xB3
    VK_LAUNCH_MAIL         = Keys.LaunchMail,          // 0xB4
    VK_LAUNCH_MEDIA_SELECT = Keys.SelectMedia,         // 0xB5
    VK_LAUNCH_APP1         = Keys.LaunchApplication1,  // 0xB6
    VK_LAUNCH_APP2         = Keys.LaunchApplication2,  // 0xB7
    /* 0xB8 - 0xB9 : reserved */
    VK_OEM_1               = Keys.OemSemicolon,        // 0xBA, Keys.Oem1
    VK_OEM_PLUS            = Keys.Oemplus,             // 0xBB
    VK_OEM_COMMA           = Keys.Oemcomma,            // 0xBC
    VK_OEM_MINUS           = Keys.OemMinus,            // 0xBD
    VK_OEM_PERIOD          = Keys.OemPeriod,           // 0xBE
    VK_OEM_2               = Keys.OemQuestion,         // 0xBF, Keys.Oem2
    VK_OEM_3               = Keys.Oemtilde,            // 0xC0, Keys.Oem3
    /* 0xC1 - 0xD7 : reserved  */
    /* 0xD8 - 0xDA : unassigned */
    VK_OEM_4               = Keys.OemOpenBrackets,     // 0xDB, Keys.Oem4
    VK_OEM_5               = Keys.OemPipe,             // 0xDC, Keys.Oem5
    VK_OEM_6               = Keys.OemCloseBrackets,    // 0xDD, Keys.Oem6
    VK_OEM_7               = Keys.OemQuotes,           // 0xDE, Keys.Oem7
    VK_OEM_8               = Keys.Oem8,                // 0xDF
    /* 0xE0 : reserved */
    VK_OEM_AX              = 0xE1,                     // 0xE1, 'AX' key on Japanese AX kbd
    VK_OEM_102             = Keys.OemBackslash,        // 0xE2, Keys.Oem102
    VK_ICO_HELP            = 0xE3,                     // 0xE3, Help key on ICO
    VK_ICO_00              = 0xE4,                     // 0xE4, 00 key on ICO
    VK_PROCESSKEY          = Keys.ProcessKey,          // 0xE5
    VK_ICO_CLEAR           = 0xE6,                     // 0xE6
    VK_PACKET              = 0xE7,                     // 0xE7, Keys.Packet
    /* 0xE8 : unassigned */
    VK_OEM_RESET           = 0xE9,                     // 0xE9, Nokia/Ericsson definition
    VK_OEM_JUMP            = 0xEA,                     // 0xEA, Nokia/Ericsson definition
    VK_OEM_PA1             = 0xEB,                     // 0xEB, Nokia/Ericsson definition
    VK_OEM_PA2             = 0xEC,                     // 0xEC, Nokia/Ericsson definition
    VK_OEM_PA3             = 0xED,                     // 0xED, Nokia/Ericsson definition
    VK_OEM_WSCTRL          = 0xEE,                     // 0xEE, Nokia/Ericsson definition
    VK_OEM_CUSEL           = 0xEF,                     // 0xEF, Nokia/Ericsson definition
    VK_OEM_ATTN            = 0xF0,                     // 0xF0, Nokia/Ericsson definition
    VK_OEM_FINISH          = 0xF1,                     // 0xF1, Nokia/Ericsson definition
    VK_OEM_COPY            = 0xF2,                     // 0xF2, Nokia/Ericsson definition
    VK_OEM_AUTO            = 0xF3,                     // 0xF3, Nokia/Ericsson definition
    VK_OEM_ENLW            = 0xF4,                     // 0xF4, Nokia/Ericsson definition
    VK_OEM_BACKTAB         = 0xF5,                     // 0xF5, Nokia/Ericsson definition
    VK_ATTN                = Keys.Attn,                // 0xF6
    VK_CRSEL               = Keys.Crsel,               // 0xF7
    VK_EXSEL               = Keys.Exsel,               // 0xF8
    VK_EREOF               = Keys.EraseEof,            // 0xF9
    VK_PLAY                = Keys.Play,                // 0xFA
    VK_ZOOM                = Keys.Zoom,                // 0xFB
    VK_NONAME              = Keys.NoName,              // 0xFC
    VK_PA1                 = Keys.Pa1,                 // 0xFD
    VK_OEM_CLEAR           = Keys.OemClear,            // 0xFE
  }

  public enum ShiftState : int {
    Base = 0,                            // 0
    Shft = 1,                            // 1
    LCtrl = 2,                           // 2
    ShftLCtrl = Shft | LCtrl,            // 3
    Menu = 4,                            // 4 -- NOT USED
    ShftMenu = Shft | Menu,              // 5 -- NOT USED
    MenuCtrl = Menu | LCtrl,             // 6
    ShftMenuLCtrl = Shft | Menu | LCtrl, // 7
    RCtrl = 8,                           // 8
    ShftRCtrl = Shft | RCtrl,            // 9
  }

  public struct st_multi_chars {
    public uint scss;
    public int num;
    public string mchars;
  }

  public class DeadKey {
    private char m_deadchar;
    private IEnumerator<KeyValuePair<ushort, char>> dk_enum;

    private Dictionary<ushort, char> m_rgdeadkeys = new Dictionary<ushort, char>();

    public DeadKey(char deadCharacter) {
      this.m_deadchar = deadCharacter;
    }

    public char DeadCharacter {
      get {
        return this.m_deadchar;
      }
    }

    public void AddDeadKeyRow(uint scancode, int caps, ShiftState ss,
                              char combinedCharacter) {
      char value;
      ushort icaps, uindex, iss, isc;
      if (caps == 1)
        icaps = 16;
      else
        icaps = 0;
      isc = Convert.ToUInt16(scancode);
      isc *= 256;
      iss = Convert.ToUInt16((int)ss);
      uindex = (ushort)(isc + iss + icaps);

      if (this.m_rgdeadkeys.TryGetValue(uindex, out value)) {
        this.m_rgdeadkeys[uindex] = combinedCharacter;
      } else {
        this.m_rgdeadkeys.Add(uindex, combinedCharacter);
      }
    }

    public int Count {
      get {
        return this.m_rgdeadkeys.Count;
      }
    }

    public void GetDeadKeyInfo(ref uint scss, out char value) {
      if (scss == 0) {
        // Initial call to set up enumerator
        dk_enum = this.m_rgdeadkeys.GetEnumerator();
        value = (char)0;
        return;
      }
      dk_enum.MoveNext();
      scss = dk_enum.Current.Key;
      value = dk_enum.Current.Value;
    }
  }

  public class VirtualKey {
    [DllImport("user32.dll", CharSet = CharSet.Unicode, EntryPoint = "MapVirtualKeyExW", ExactSpelling = true)]
    internal static extern uint MapVirtualKeyEx(uint uCode, uint uMapType, IntPtr dwhkl);
    private IntPtr m_hkl;
    private uint m_vk;
    private uint m_sc;
    private bool[,] m_rgfDeadKey = new bool[(int)ShiftState.ShftRCtrl + 1, 2];
    private string[,] m_rgss = new string[(int)ShiftState.ShftRCtrl + 1, 2];
    private string chars1, chars2, chars3, chars4;
    private List<st_multi_chars> Multi_Chars_List = new List<st_multi_chars>();

    public VirtualKey() {
      this.m_sc = (int)0;
      this.m_hkl = (IntPtr)0;
      this.m_vk = (uint)0;
    }

    public VirtualKey(IntPtr hkl, KeysEx virtualKey) {
      this.m_sc = MapVirtualKeyEx((uint)virtualKey, 0, hkl);
      this.m_hkl = hkl;
      this.m_vk = (uint)virtualKey;
    }

    public VirtualKey(IntPtr hkl, uint scanCode) {
      this.m_vk = MapVirtualKeyEx(scanCode, 1, hkl);
      this.m_hkl = hkl;
      this.m_sc = scanCode;
    }

    public KeysEx VK {
      get {
        return (KeysEx)this.m_vk;
      }
    }

    public uint SC {
      get {
        return this.m_sc;
      }
    }

    public string GetShiftState(ShiftState shiftState, bool capsLock) {
      if (this.m_rgss[(uint)shiftState, (capsLock ? 1 : 0)] == null) {
        return ("");
      }
      return (this.m_rgss[(uint)shiftState, (capsLock ? 1 : 0)]);
    }

    public void SetShiftState(ShiftState shiftState, string value, bool isDeadKey, bool capsLock) {
      this.m_rgfDeadKey[(uint)shiftState, (capsLock ? 1 : 0)] = isDeadKey;
      this.m_rgss[(uint)shiftState, (capsLock ? 1 : 0)] = value;
    }

    public bool IsSGCAPS {
      get {
        string stBase = this.GetShiftState(ShiftState.Base, false);
        string stShift = this.GetShiftState(ShiftState.Shft, false);
        string stCaps = this.GetShiftState(ShiftState.Base, true);
        string stShiftCaps = this.GetShiftState(ShiftState.Shft, true);
        return (
            ((stCaps.Length > 0) &&
            (!stBase.Equals(stCaps)) &&
            (!stShift.Equals(stCaps))) ||
            ((stShiftCaps.Length > 0) &&
            (!stBase.Equals(stShiftCaps)) &&
            (!stShift.Equals(stShiftCaps)))
          );
      }
    }

    public bool IsCapsEqualToShift {
      get {
        string stBase = this.GetShiftState(ShiftState.Base, false);
        string stShift = this.GetShiftState(ShiftState.Shft, false);
        string stCaps = this.GetShiftState(ShiftState.Base, true);
        return (
            (stBase.Length > 0) &&
            (stShift.Length > 0) &&
            (!stBase.Equals(stShift)) &&
            (stShift.Equals(stCaps))
          );
      }
    }

    public bool IsAltGrCapsEqualToAltGrShift {
      get {
        string stBase = this.GetShiftState(ShiftState.MenuCtrl, false);
        string stShift = this.GetShiftState(ShiftState.ShftMenuLCtrl, false);
        string stCaps = this.GetShiftState(ShiftState.MenuCtrl, true);
        return (
            (stBase.Length > 0) &&
            (stShift.Length > 0) &&
            (!stBase.Equals(stShift)) &&
            (stShift.Equals(stCaps))
          );
      }
    }

    public bool IsRCtrlGrCapsEqualToRCtrlShift {
      get {
        string stBase = this.GetShiftState(ShiftState.RCtrl, false);
        string stShift = this.GetShiftState(ShiftState.ShftRCtrl, false);
        string stCaps = this.GetShiftState(ShiftState.RCtrl, true);
        return (
            (stBase.Length > 0) &&
            (stShift.Length > 0) &&
            (!stBase.Equals(stShift)) &&
            (stShift.Equals(stCaps))
          );
      }
    }

    public bool IsEmpty {
      get {
        for (int i = 0; i < this.m_rgss.GetUpperBound(0); i++) {
          for (int j = 0; j <= 1; j++) {
            if (this.GetShiftState((ShiftState)i, (j == 1)).Length > 0) {
              return (false);
            }
          }
        }
        return true;
      }
    }

    public string Get_Standard_Chars1 {
      get {
        // Ignore last comma
        return chars1.Substring(0, chars1.Length - 1);
      }
    }
    public string Get_Standard_Chars2 {
      get {
        // Ignore last comma
        return chars2.Substring(0, chars2.Length - 1);
      }
    }
    public string Get_Standard_Chars3 {
      get {
        // Ignore last comma
        return chars3.Substring(0, chars3.Length - 1);
      }
    }
    public string Get_Standard_Chars4 {
      get {
        // Ignore last comma
        return chars4.Substring(0, chars4.Length - 1);
      }
    }
    public int Get_Multi_Chars_Count {
      get {
        return Multi_Chars_List.Count;
      }
    }
    public List<st_multi_chars> Get_Multi_Chars_List {
      get {
        return Multi_Chars_List;
      }
    }

    public string GetRowHeader {
      get {
        StringBuilder sbHeader = new StringBuilder();
        StringBuilder sbDeadKeys = new StringBuilder();
        StringBuilder sb_std_chars1 = new StringBuilder();
        StringBuilder sb_std_chars2 = new StringBuilder();
        StringBuilder sb_std_chars3 = new StringBuilder();
        StringBuilder sb_std_chars4 = new StringBuilder();
        // First, get the SC info stored
        sbHeader.Append(string.Format("0x{0:x02}, ", this.SC));
        // Now the CAPSLOCK value
        int capslock = 0 |
                      (this.IsCapsEqualToShift ? 1 : 0) |
                      (this.IsSGCAPS ? 2 : 0) |
                      (this.IsAltGrCapsEqualToAltGrShift ? 4 : 0) |
                      (this.IsRCtrlGrCapsEqualToRCtrlShift ? 8 : 0);

        for (ShiftState ss = 0; ss <= Loader.MaxShiftState; ss++) {
          if (ss == ShiftState.Menu || ss == ShiftState.ShftMenu) {
            // Alt and Shift+Alt don't work, so skip them
            continue;
          }

          for (int caps = 0; caps <= 1; caps++) {
            string st = this.GetShiftState(ss, (caps == 1));
            if (st.Length == 0) {
              // No character assigned here, put in 0.
              sbDeadKeys.Append("0");
              if (ss <= ShiftState.Shft)
                sb_std_chars1.Append(" 0x0000,");
              else if (ss <= ShiftState.ShftLCtrl)
                sb_std_chars2.Append(" 0x0000,");
              else if (ss <= ShiftState.ShftMenuLCtrl)
                sb_std_chars3.Append(" 0x0000,");
              else
                sb_std_chars4.Append(" 0x0000,");
            } else {
              // Exclude non-valid characters (special) and
              // Special typesetting characters ...
              // 8203 = Zero Width Space,  8204 = Zero Width Non Joiner,
              // 8205 = Zero Width Joiner, 8206 = Left-to-Right Mark
              // 8207 = Right-to-Left Mark
              //  ... there may be more ...
              if (st.Length != 1) {
                string sTemp;
                StringBuilder sbChar = new StringBuilder();
                for (int ich = 0; ich < st.Length; ich++) {
                  sbChar.Append(string.Format(" 0x{0},",((ushort)st[ich]).ToString("x4")));
                }
                int ilen = -st.Length;
                sTemp = ilen.ToString("x4").Substring(4, 4);
                if (ss <= ShiftState.Shft)
                  sb_std_chars1.Append(string.Format(" 0x{0},", sTemp));
                else if (ss <= ShiftState.ShftLCtrl)
                  sb_std_chars2.Append(string.Format(" 0x{0},", sTemp));
                else if (ss <= ShiftState.ShftMenuLCtrl)
                  sb_std_chars3.Append(string.Format(" 0x{0},", sTemp));
                else
                  sb_std_chars4.Append(string.Format(" 0x{0},", sTemp));
                st_multi_chars stmchars;
                stmchars.scss = (uint)(((int)this.SC * 256) + ss + caps * 16);
                stmchars.num = st.Length;
                stmchars.mchars = string.Format("{0}", sbChar.ToString().Substring(0, sbChar.Length - 1));
                Multi_Chars_List.Add(stmchars);
              } else {
                // Change 'non-breaking' space to 'ordinary' space
                if (((ushort)st[0]) == 160)
                  st = " ";
                if ((ushort)st[0] < 0x20 &&
                    this.VK != KeysEx.VK_SPACE ||
                    (this.VK == KeysEx.VK_SPACE && ss > ShiftState.Shft) ||
                    ((ushort)st[0] >= 8203 && (ushort)st[0] <= 8207)) {
                  if (ss <= ShiftState.Shft)
                    sb_std_chars1.Append(" 0x0000,");
                  else if (ss <= ShiftState.ShftLCtrl)
                    sb_std_chars2.Append(" 0x0000,");
                  else if (ss <= ShiftState.ShftMenuLCtrl)
                    sb_std_chars3.Append(" 0x0000,");
                  else
                    sb_std_chars4.Append(" 0x0000,");
                } else {
                  if (ss <= ShiftState.Shft)
                    sb_std_chars1.Append(string.Format(" 0x{0:x4},", ((ushort)st[0])));
                  else if (ss <= ShiftState.ShftLCtrl)
                    sb_std_chars2.Append(string.Format(" 0x{0:x4},", ((ushort)st[0])));
                  else if (ss <= ShiftState.ShftMenuLCtrl)
                    sb_std_chars3.Append(string.Format(" 0x{0:x4},", ((ushort)st[0])));
                  else
                    sb_std_chars4.Append(string.Format(" 0x{0:x4},", ((ushort)st[0])));
                }
                if (this.m_rgfDeadKey[(int)ss, caps])
                  sbDeadKeys.Append("1");
                else
                  sbDeadKeys.Append("0");
              }
            }
          }
        }

        if (Loader.MaxShiftState != ShiftState.ShftRCtrl) {
          sb_std_chars4.Append(" 0x0000, 0x0000, 0x0000, 0x0000,");
          sbDeadKeys.Append("0000");
        }

        char [] strArray = sbDeadKeys.ToString().ToCharArray();
        Array.Reverse(strArray);
        string strReversed = new string(strArray);
        ulong iDeadKeys = Convert.ToUInt64(strReversed, 2);
        sbHeader.Append(string.Format("{0,6},", iDeadKeys));

        chars1 = sb_std_chars1.ToString();
        chars2 = sb_std_chars2.ToString();
        chars3 = sb_std_chars3.ToString();
        chars4 = sb_std_chars4.ToString();
        return (sbHeader.ToString());
      }
    }
  }

  public struct st_XML_Scancode {
    public string sc;
    public string chars1;
    public string chars2;
    public string chars3;
    public string chars4;
  }

  public struct st_XML_Keyboard {
    public string KLID;
    public string Name;
    public List<st_XML_Scancode> KeyData;

    public st_XML_Keyboard(st_XML_Keyboard that) {
      this.KLID = that.KLID;
      this.Name = that.Name;
      this.KeyData = new List<st_XML_Scancode>(that.KeyData);
    }
  }

  public class ProcessXML {
    private XmlTextReader Reader;
    private string XSDPath;
    public List<string> Results = new List<string>();
    public List<st_XML_Keyboard> XMLKeyboards = new List<st_XML_Keyboard>();

    public ProcessXML(string XSD) {
      this.XSDPath = XSD;
    }

    public List<string> DoXML(bool bValidate, string XMLPath) {
      ProcessUserKeyboards(bValidate, XMLPath);
      return this.Results;
    }

    private void ProcessUserKeyboards(bool bValidate, string XMLPath) {
      this.Reader = new XmlTextReader(XMLPath);
      StreamReader SR = new StreamReader(this.XSDPath);

      XmlReaderSettings ReaderSettings = new XmlReaderSettings();

      if (bValidate) {
        XmlSchema Schema = new XmlSchema();
        Schema = XmlSchema.Read(SR,
                  new ValidationEventHandler(ReaderSettings_ValidationEventHandler));

        ReaderSettings.ValidationType = ValidationType.Schema;
        ReaderSettings.Schemas.Add(Schema);

        ReaderSettings.ValidationEventHandler +=
                new ValidationEventHandler(ReaderSettings_ValidationEventHandler);
      }
      XmlReader objXmlReader = XmlReader.Create(Reader, ReaderSettings);

      st_XML_Keyboard stkb = new st_XML_Keyboard();
      stkb.KeyData = new List<st_XML_Scancode>();
      st_XML_Scancode stsc = new st_XML_Scancode();

      while (objXmlReader.Read()) {
        switch (objXmlReader.NodeType) {
          case XmlNodeType.Element:
            if (bValidate)
              break;
            if (objXmlReader.Name == "keyboard") {
              stkb.KLID = objXmlReader.GetAttribute("klid");
              stkb.Name = objXmlReader.GetAttribute("kname");
            }
            if (objXmlReader.Name == "key") {
              stsc.sc  = objXmlReader.GetAttribute("scancode");
              string c1, c2, c3, c4;
              c1 = objXmlReader.GetAttribute("b");
              c2 = objXmlReader.GetAttribute("bC");
              c3 = objXmlReader.GetAttribute("sb");
              c4 = objXmlReader.GetAttribute("sbC");
              stsc.chars1 = " 0x" + c1 + ", 0x" + c2 + ", 0x" + c3 + ", 0x" + c4;
              c1 = objXmlReader.GetAttribute("l");
              c2 = objXmlReader.GetAttribute("lC");
              c3 = objXmlReader.GetAttribute("sl");
              c4 = objXmlReader.GetAttribute("slC");
              stsc.chars2 = " 0x" + c1 + ", 0x" + c2 + ", 0x" + c3 + ", 0x" + c4;
              c1 = objXmlReader.GetAttribute("g");
              c2 = objXmlReader.GetAttribute("gC");
              c3 = objXmlReader.GetAttribute("sg");
              c4 = objXmlReader.GetAttribute("sgC");
              stsc.chars3 = " 0x" + c1 + ", 0x" + c2 + ", 0x" + c3 + ", 0x" + c4;
              c1 = objXmlReader.GetAttribute("r");
              c2 = objXmlReader.GetAttribute("rC");
              c3 = objXmlReader.GetAttribute("sr");
              c4 = objXmlReader.GetAttribute("srC");
              stsc.chars4 = " 0x" + c1 + ", 0x" + c2 + ", 0x" + c3 + ", 0x" + c4;
              stkb.KeyData.Add(stsc);
            }
            break;
          case XmlNodeType.EndElement:
            if (bValidate)
              break;

            if (objXmlReader.Name == "keyboard") {
              XMLKeyboards.Add(new st_XML_Keyboard(stkb));
              stkb.KeyData.Clear();
            }
            break;
        }
      }
      objXmlReader.Close();
    }

    private void ReaderSettings_ValidationEventHandler(object sender, 
                             ValidationEventArgs args) {
      string strTemp;
      strTemp = "Line: " + this.Reader.LineNumber + " - Position: " 
                 + this.Reader.LinePosition + " - " + args.Message; 
      this.Results.Add(strTemp);
    }
  }


  public class Loader {
    public struct kbd_layout {
      public string Key;
      public string Name;
    }

    private const uint KLF_NOTELLSHELL = 0x00000080;
    internal static KeysEx[] lpKeyStateNull = new KeysEx[256];
    [DllImport("user32.dll", CharSet = CharSet.Unicode, EntryPoint = "LoadKeyboardLayoutW", ExactSpelling = true)]
    private static extern IntPtr LoadKeyboardLayout(string pwszKLID, uint Flags);
    [DllImport("user32.dll", ExactSpelling = true)]
    private static extern bool UnloadKeyboardLayout(IntPtr hkl);
    [DllImport("user32.dll", CharSet = CharSet.Unicode, ExactSpelling = true)]
    private static extern int ToUnicodeEx(uint wVirtKey, uint wScanCode, KeysEx[] lpKeyState,
                                          StringBuilder pwszBuff, int cchBuff, uint wFlags,
                                          IntPtr dwhkl);
    [DllImport("user32.dll", CharSet = CharSet.Unicode, EntryPoint = "VkKeyScanExW", ExactSpelling = true)]
    private static extern ushort VkKeyScanEx(char ch, IntPtr dwhkl);
    [DllImport("user32.dll", ExactSpelling = true)]
    private static extern int GetKeyboardLayoutList(int nBuff,
                                                    [Out, MarshalAs(UnmanagedType.LPArray)] IntPtr[] lpList);

    private static KeysEx m_RCtrlVk = KeysEx.None;

    public static KeysEx RCtrlVk {
      get {
        return m_RCtrlVk;
      }
      set {
        m_RCtrlVk = value;
      }
    }

    public static ShiftState MaxShiftState {
      get {
        return (Loader.RCtrlVk == KeysEx.None ? ShiftState.ShftMenuLCtrl : ShiftState.ShftRCtrl);
      }
    }

    private static void FillKeyState(KeysEx[] lpKeyState, ShiftState ss, bool fCapsLock) {
      lpKeyState[(int)KeysEx.VK_SHIFT] = (((ss & ShiftState.Shft) != 0) ? (KeysEx)0x80 : (KeysEx)0x00);
      lpKeyState[(int)KeysEx.VK_CONTROL] = (((ss & ShiftState.LCtrl) != 0) ? (KeysEx)0x80 : (KeysEx)0x00);
      lpKeyState[(int)KeysEx.VK_MENU] = (((ss & ShiftState.Menu) != 0) ? (KeysEx)0x80 : (KeysEx)0x00);
      if (Loader.RCtrlVk != KeysEx.None) {
        // The RCtrl key has been assigned, so let's include it
        lpKeyState[(int)Loader.RCtrlVk] = (((ss & ShiftState.RCtrl) != 0) ? (KeysEx)0x80 : (KeysEx)0x00);
      }
      lpKeyState[(int)KeysEx.VK_CAPITAL] = (fCapsLock ? (KeysEx)0x01 : (KeysEx)0x00);
    }

    private static DeadKey ProcessDeadKey(
            uint iKeyDead,              // The index into the VirtualKey of the dead key
            ShiftState shiftStateDead,  // The shiftstate that contains the dead key
            KeysEx[] lpKeyStateDead,    // The key state for the dead key
            VirtualKey[] rgKey,         // Our array of dead keys
            bool fCapsLock,             // Was the caps lock key pressed?
            IntPtr hkl)                 // The keyboard layout
    {
      KeysEx[] lpKeyState = new KeysEx[256];
      DeadKey deadKey = new DeadKey(rgKey[iKeyDead].GetShiftState(shiftStateDead, fCapsLock)[0]);
      for (uint iKey = 0; iKey < rgKey.Length; iKey++) {
        if (rgKey[iKey] != null) {
          StringBuilder sbBuffer = new StringBuilder(10);   // Scratchpad we use many places
          for (ShiftState ss = ShiftState.Base; ss <= Loader.MaxShiftState; ss++) {
            int rc = 0;
            if (ss == ShiftState.Menu || ss == ShiftState.ShftMenu) {
              // Alt and Shift+Alt don't work, so skip them
              continue;
            }
            for (int caps = 0; caps <= 1; caps++) {
              // First the dead key
              while (rc >= 0) {
                // We know that this is a dead key coming up, otherwise
                // this function would never have been called. If we do
                // *not* get a dead key then that means the state is
                // messed up so we run again and again to clear it up.
                // Risk is technically an infinite loop but per Hiroyama
                // that should be impossible here.
                rc = ToUnicodeEx((uint)rgKey[iKeyDead].VK, rgKey[iKeyDead].SC,
                                 lpKeyStateDead, sbBuffer, sbBuffer.Capacity, 0, hkl);
              }
              // Now fill the key state for the potential base character
              FillKeyState(lpKeyState, ss, (caps != 0));
              sbBuffer = new StringBuilder(10);
              rc = ToUnicodeEx((uint)rgKey[iKey].VK, rgKey[iKey].SC,
                               lpKeyState, sbBuffer, sbBuffer.Capacity, 0, hkl);
              if (rc == 1) {
                // That was indeed a base character for our dead key.
                // And we now have a composite character. Let's run
                // through one more time to get the actual base
                // character that made it all possible?
                char combchar = sbBuffer[0];
                sbBuffer = new StringBuilder(10);
                rc = ToUnicodeEx((uint)rgKey[iKey].VK, rgKey[iKey].SC,
                                 lpKeyState, sbBuffer, sbBuffer.Capacity, 0, hkl);
                char basechar = sbBuffer[0];
                if (deadKey.DeadCharacter == combchar) {
                  // Since the combined character is the same as the dead key,
                  // we must clear out the keyboard buffer.
                  ClearKeyboardBuffer((uint)KeysEx.VK_DECIMAL, rgKey[(uint)KeysEx.VK_DECIMAL].SC, hkl);
                }
                if ((((ss == ShiftState.LCtrl) || (ss == ShiftState.ShftLCtrl)) &&
                    (char.IsControl(basechar))) ||
                    (basechar.Equals(combchar))) {
                  // ToUnicodeEx has an internal knowledge about those
                  // VK_A ~ VK_Z keys to produce the control characters,
                  // when the conversion rule is not provided in keyboard
                  // layout files
                  // Additionally, dead key state is lost for some of these
                  // character combinations, for unknown reasons.
                  // Therefore, if the base character and combining are equal,
                  // and its a CTRL or CTRL+SHIFT state, and a control character
                  // is returned, then we do not add this "dead key" (which
                  // is not really a dead key).
                  continue;
                }
                if (caps == 0 && (ss == ShiftState.Base || ss == ShiftState.Shft))
                  deadKey.AddDeadKeyRow(rgKey[iKey].SC, caps, ss, combchar);
              } else if (rc > 1) {
                // Not a valid dead key combination, sorry! We just ignore it.
              } else if (rc < 0) {
                // It's another dead key, so we ignore it (other than to flush it from the state)
                ClearKeyboardBuffer((uint)KeysEx.VK_DECIMAL, rgKey[(uint)KeysEx.VK_DECIMAL].SC, hkl);
              }
            }
          }
        }
      }
      return deadKey;
    }

    private static void ClearKeyboardBuffer(uint vk, uint sc, IntPtr hkl) {
      StringBuilder sb = new StringBuilder(10);
      int rc;
      do {
        rc = ToUnicodeEx(vk, sc, lpKeyStateNull, sb, sb.Capacity, 0, hkl);
      } while (rc < 0) ;
    }

    private static void GetKeyboardsOnMachine(ref List<kbd_layout> kll) {
      RegistryKey KBL_rkey = Registry.LocalMachine;
      KBL_rkey = KBL_rkey.OpenSubKey("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts", false);
      kbd_layout kbl = new kbd_layout();

      foreach (string Keyname in KBL_rkey.GetSubKeyNames()) {
        kbl.Key = Keyname.ToUpper();
        RegistryKey tempKey = KBL_rkey.OpenSubKey(Keyname);
        kbl.Name = tempKey.GetValue("Layout Text").ToString();
        kll.Add(kbl);
      }

      KBL_rkey.Close();
    }

    [STAThread]
    static void Main(string[] args) {
      const string sBanner = "/*\r\n\r\n" +
              "This include file for Password Safe support of all keyboards supported\r\n" +
              "by Windows was created by the \"Password Safe - Keyboard Data Generator\" program\r\n" +
              "kblgen V{0} on a Windows 7 64-bit RC system with all keyboard layouts installed on\r\n" +
              "{1:u}.\r\n\r\n" +
              "The \"Password Safe - Keyboard Data Generator\" program is a modified version of\r\n" +
              "code published by Michael Kaplan in his series of MSDN Blogs \"Sorting it all Out,\r\n" +
              "Getting all you can out of a keyboard layout\" (Part 0 to Part 9b) between \r\n" +
              "March 23 and April 13, 2006.\r\n\r\n" +
              "See http://blogs.msdn.com/michkap/archive/2006/04/13/575500.aspx\r\n\r\n" +
              "  *** PLEASE DO NOT EDIT THIS FILE DIRECTLY ***\r\n\r\n" +
              "*/\r\n";
      string version = Application.ProductVersion;

      List<kbd_layout> kbd_layout_list = new List<kbd_layout>();
      GetKeyboardsOnMachine(ref kbd_layout_list);

      int cKeyboards = GetKeyboardLayoutList(0, null);
      IntPtr[] rghkl = new IntPtr[cKeyboards];
      GetKeyboardLayoutList(cKeyboards, rghkl);

      DateTime dateTime = DateTime.Now;

      CultureInfo cultureInfo   = Thread.CurrentThread.CurrentCulture;
      TextInfo textInfo = cultureInfo.TextInfo;

      List<string> Char_List = new List<string>();
      List<int> Char_Use_Count = new List<int>();
      List<string> Multi2Char_List = new List<string>();
      List<int> Multi2Char_Use_Count = new List<int>();
      List<string> Multi3Char_List = new List<string>();
      List<int> Multi3Char_Use_Count = new List<int>();
      List<string> Multi4Char_List = new List<string>();
      List<int> Multi4Char_Use_Count = new List<int>();
      List<char> DeadKey_List = new List<char>();
      List<int> DeadKey_Use_Count = new List<int>();
      List<string> KLID_Exclude_List = new List<string>();

      // The first four handled specially via XML
      KLID_Exclude_List.Add("00000404");  // Chinese (Simplified)
      KLID_Exclude_List.Add("00000411");  // Japanese
      KLID_Exclude_List.Add("00000412");  // Korean
      KLID_Exclude_List.Add("00000804");  // Chinese (Traditional)

      // Can't display characters using MS Arial Unicode font
      KLID_Exclude_List.Add("0000045A");  // Syriac
      KLID_Exclude_List.Add("00000465");  // Divehi Phonetic
      KLID_Exclude_List.Add("00010465");  // Divehi Typewriter

      // Not shown on MS web page of Keyboard Layouts (although in Vista/Windows 7)
      KLID_Exclude_List.Add("0000042F");  // Macedonian (Fyrom)
      KLID_Exclude_List.Add("0000044C");  // Malayalam
      KLID_Exclude_List.Add("0000044D");  // Assamese - INSCRIPT
      KLID_Exclude_List.Add("00000453");  // Khmer
      KLID_Exclude_List.Add("00000454");  // Lao
      KLID_Exclude_List.Add("0000045B");  // Sinhala
      KLID_Exclude_List.Add("00000461");  // Nepali
      KLID_Exclude_List.Add("00000463");  // Pashto (Afganistan)
      KLID_Exclude_List.Add("0000046D");  // Bashkir
      KLID_Exclude_List.Add("0000046E");  // Luxembourgish
      KLID_Exclude_List.Add("0000046F");  // Greenlandic
      KLID_Exclude_List.Add("00000480");  // Uighur
      KLID_Exclude_List.Add("00000485");  // Yakut
      KLID_Exclude_List.Add("00000850");  // Mongolian (Mongolian Script)
      KLID_Exclude_List.Add("0000085D");  // Inuktitut - Latin
      KLID_Exclude_List.Add("00010416");  // Portuguese (Brazilian Abnt2)
      KLID_Exclude_List.Add("0001042E");  // Sorbian Extended
      KLID_Exclude_List.Add("0001042F");  // Macedonian (Fyrom) - Standard
      KLID_Exclude_List.Add("0001045A");  // Syriac Phonetic
      KLID_Exclude_List.Add("0001045B");  // Sinhala - wij
      KLID_Exclude_List.Add("0001045D");  // Inuktitut - Naqittaut
      KLID_Exclude_List.Add("00010401");  // Arabic (102)
      KLID_Exclude_List.Add("00020401");  // Arabic (102) AZERTY
      KLID_Exclude_List.Add("00020402");  // Bulgarian (Phonetic Layout)

      // Others
      KLID_Exclude_List.Add("0000043A");  // Maltese 47-Key
      KLID_Exclude_List.Add("0001043A");  // Maltese 48-Key
      KLID_Exclude_List.Add("00000451");  // Tibetan (Prc)
      KLID_Exclude_List.Add("00000C04");  // Chinese (Traditional, Hong Kong S.A.R.) - US Keyboard
      KLID_Exclude_List.Add("00001004");  // Chinese (Simplified, Singapore) - US Keyboard
      KLID_Exclude_List.Add("00001404");  // Chinese (Traditional, Macao S.A.R.) - Us Keyboard

      // Don't support Dvorak keyboard
      KLID_Exclude_List.Add("00010409");  // United States-Dvorak
      KLID_Exclude_List.Add("00030409");  // United States-Dvorak for left hand
      KLID_Exclude_List.Add("00040409");  // United States-Dvorak for right hand
      KLID_Exclude_List.Add("00050409");  // US English Table for IBM Arabic 238_L

      const int IDS_PHYSICAL = 8200;
      const int IDS_BASE = IDS_PHYSICAL + 1;
      string strKorean = "00000000";
      string strJapanese = "00000000";
      int max_rows = 0;
      int iCountMain = 0;
      int iCount = 0;
      int inumKLID2MC2 = 0;
      int inumKLID2MC3 = 0;
      int inumKLID2MC4 = 0;
      int iCountMC2 = 0;
      int iCountMC3 = 0;
      int iCountMC4 = 0;
      int iCountDK = -1;
      int iMC2_Offset = -1;
      int iMC3_Offset = -1;
      int iMC4_Offset = -1;
      int iDK_Offset = -1;
      // Initialise Char_List with most used entry
      // NOTE: This MUST be the first entry as offset == 0 is used in PWS
      int iOffset = 0;
      Char_List.Add(" 0x0000, 0x0000, 0x0000, 0x0000");
      Char_Use_Count.Add(0);

      StreamWriter sw_log = new StreamWriter("log.txt");

      StreamWriter sw_OSK_KLID2VKBDX_Table = new StreamWriter("OSK_KLID2VKBDX_Table.inc");
      sw_OSK_KLID2VKBDX_Table.WriteLine(sBanner, version, dateTime);
      string Cmd_arg = "";
      bool bAll = false;
      sw_OSK_KLID2VKBDX_Table.WriteLine("static const st_IKLID2VKBD IKLID2VKBD[] = {");
      if (args.Length > 0) {
        Cmd_arg = args[0].ToUpper();
        if (Cmd_arg.CompareTo("ALL") == 0)
          bAll = true;
      }
      if (Cmd_arg.Length > 0 && !bAll) {
        sw_OSK_KLID2VKBDX_Table.WriteLine("  {{0x{0}, &VKBD_{0}}},  // {1,4}", Cmd_arg, iCountMain);
        iCountMain++;
      } else {
        int i = -1;
        foreach (kbd_layout kbl in kbd_layout_list) {
          i++;
          // Exclude Dvorak keyboard or those requiring an IME + a few others
          // Unlesscommand argument 'all' specified
          if (!bAll && KLID_Exclude_List.Contains(kbl.Key))
            continue;

          sw_OSK_KLID2VKBDX_Table.WriteLine("  {{0x{0}, &VKBD_{0}}},  // {1,4}", kbl.Key,
                                            iCountMain);
          iCountMain++;
        }
      }

      StreamWriter sw_OSK_KB_Data = new StreamWriter("OSK_KB_Data.inc");
      sw_OSK_KB_Data.WriteLine(sBanner, version, dateTime);

      StreamWriter sw_OSK_KB_KLID2SCSS2MC2Data = new StreamWriter("OSK_KB_KLID2SCSS2MC2Data.inc");
      sw_OSK_KB_KLID2SCSS2MC2Data.WriteLine(sBanner, version, dateTime);

      StreamWriter sw_OSK_KB_KLID2SCSS2MC3Data = new StreamWriter("OSK_KB_KLID2SCSS2MC3Data.inc");
      sw_OSK_KB_KLID2SCSS2MC3Data.WriteLine(sBanner, version, dateTime);

      StreamWriter sw_OSK_KB_KLID2SCSS2MC4Data = new StreamWriter("OSK_KB_KLID2SCSS2MC4Data.inc");
      sw_OSK_KB_KLID2SCSS2MC4Data.WriteLine(sBanner, version, dateTime);

      StreamWriter sw_OSK_KB_SCSS2MC2Data = new StreamWriter("OSK_KB_SCSS2MC2Data.inc");
      sw_OSK_KB_SCSS2MC2Data.WriteLine(sBanner, version, dateTime);

      StreamWriter sw_OSK_KB_SCSS2MC3Data = new StreamWriter("OSK_KB_SCSS2MC3Data.inc");
      sw_OSK_KB_SCSS2MC3Data.WriteLine(sBanner, version, dateTime);

      StreamWriter sw_OSK_KB_SCSS2MC4Data = new StreamWriter("OSK_KB_SCSS2MC4Data.inc");
      sw_OSK_KB_SCSS2MC4Data.WriteLine(sBanner, version, dateTime);

      StreamWriter sw_OSK_Define_DeadKey_DataMaps = new StreamWriter("OSK_Define_DeadKey_DataMaps.inc");
      sw_OSK_Define_DeadKey_DataMaps.WriteLine(sBanner, version, dateTime);

      StreamWriter sw_OSK_Insert_DeadKey_DataMaps = new StreamWriter("OSK_Insert_DeadKey_DataMaps.inc");
      sw_OSK_Insert_DeadKey_DataMaps.WriteLine(sBanner, version, dateTime);

      StreamWriter sw_OSK_DeadKey_Data = new StreamWriter("OSK_DeadKey_Data.inc");
      sw_OSK_DeadKey_Data.WriteLine(sBanner, version, dateTime);

      StreamWriter sw_VKresource = new StreamWriter("VKresource3.inc");
      sw_VKresource.WriteLine(sBanner, version, dateTime);

      sw_VKresource.WriteLine("#define IDS_VKB_PHYSICAL                {0,4}\r\n",
                                  IDS_PHYSICAL);
      sw_VKresource.WriteLine("#define IDS_VKB_BASE                    {0,4}\r\n",
                                  IDS_PHYSICAL + 1);

      StreamWriter sw_VKPasswordSafe3 = new StreamWriter("VKPasswordSafe3.inc");
      sw_VKPasswordSafe3.WriteLine(sBanner, version, dateTime);

      sw_OSK_KB_KLID2SCSS2MC2Data.WriteLine("\r\nstatic const st_IKLID2SCSS2MC MC2[] = {");
      sw_OSK_KB_KLID2SCSS2MC3Data.WriteLine("\r\nstatic const st_IKLID2SCSS2MC MC3[] = {");
      sw_OSK_KB_KLID2SCSS2MC4Data.WriteLine("\r\nstatic const st_IKLID2SCSS2MC MC4[] = {");

      foreach (kbd_layout kbl in kbd_layout_list) {
        if (!bAll && Cmd_arg.Length > 0 && Cmd_arg != kbl.Key) {
          continue;
        } else {
          // Don't include excluded keyboards
          if (!bAll && KLID_Exclude_List.Contains(kbl.Key))
            continue;
        }

        IntPtr hkl = LoadKeyboardLayout(kbl.Key, KLF_NOTELLSHELL);
        if (iCount % 10 == 0)
          sw_VKPasswordSafe3.WriteLine("\r\nSTRINGTABLE\r\nBEGIN");
        if (iCount == 0)
          sw_VKPasswordSafe3.WriteLine("  IDS_VKB_PHYSICAL        \"- Physical -\"");

        Console.WriteLine("Processing Keyboard Layout - {0}:{1}", kbl.Key, kbl.Name);
        sw_log.WriteLine("Processing Keyboard Layout - {0}:{1}", kbl.Key, kbl.Name);

        sw_OSK_KB_Data.WriteLine("// Keyboard Layouts - {0}:{1}", kbl.Key, kbl.Name);
        sw_OSK_KB_Data.WriteLine("static st_IVKBD VKBD_{0} = {{", kbl.Key);
        int ires = IDS_BASE + iCount;
        sw_VKresource.WriteLine("#define IDS_VKB_{0}                {1,4}",
                                kbl.Key, ires);
        sw_VKPasswordSafe3.WriteLine("  IDS_VKB_{0}        \"{1}\"",
                                     kbl.Key, kbl.Name);
        iCount++;

        if (iCount % 10 == 0)
          sw_VKPasswordSafe3.WriteLine("END");

        Loader.RCtrlVk = KeysEx.None;

        KeysEx[] lpKeyState = new KeysEx[256];
        VirtualKey[] rgKey = new VirtualKey[256];
        List<DeadKey> alDead = new List<DeadKey>();

        // Scroll through the Scan Code (SC) values and get the valid Virtual Key (VK)
        // values in it. Then, store the SC in each valid VK so it can act as both a
        // flag that the VK is valid, and it can store the SC value.
        for (uint sc = 0x01; sc <= 0x7f; sc++) {
          VirtualKey key = new VirtualKey(hkl, sc);
          if (key.VK != 0) {
            rgKey[(uint)key.VK] = key;
          }
        }

        // add the special keys that do not get added from the code above
        for (KeysEx ke = KeysEx.VK_NUMPAD0; ke <= KeysEx.VK_NUMPAD9; ke++) {
          rgKey[(uint)ke] = new VirtualKey(hkl, ke);
        }

        rgKey[(uint)KeysEx.VK_DIVIDE] = new VirtualKey(hkl, KeysEx.VK_DIVIDE);
        rgKey[(uint)KeysEx.VK_CANCEL] = new VirtualKey(hkl, KeysEx.VK_CANCEL);
        rgKey[(uint)KeysEx.VK_DECIMAL] = new VirtualKey(hkl, KeysEx.VK_DECIMAL);
        // See if there is a special shift state added
        for (KeysEx vk = KeysEx.None; vk <= KeysEx.VK_OEM_CLEAR; vk++) {
          uint sc = VirtualKey.MapVirtualKeyEx((uint)vk, 0, hkl);
          uint vkL = VirtualKey.MapVirtualKeyEx(sc, 1, hkl);
          uint vkR = VirtualKey.MapVirtualKeyEx(sc, 3, hkl);
          if ((vkL != vkR) && ((uint)vk != vkL)) {
            switch (vk) {
              case KeysEx.VK_LCONTROL:
              case KeysEx.VK_RCONTROL:
              case KeysEx.VK_LSHIFT:
              case KeysEx.VK_RSHIFT:
              case KeysEx.VK_LMENU:
              case KeysEx.VK_RMENU:
                break;
              default:
                Loader.RCtrlVk = vk;
                break;
            }
          }
        }

        for (uint iKey = 0; iKey < rgKey.Length; iKey++) {
          if (rgKey[iKey] != null) {
            StringBuilder sbBuffer;   // Scratchpad we use many places
            for (ShiftState ss = ShiftState.Base; ss <= Loader.MaxShiftState; ss++) {
              if (ss == ShiftState.Menu || ss == ShiftState.ShftMenu) {
                // Alt and Shift+Alt don't work, so skip them
                continue;
              }
              for (int caps = 0; caps <= 1; caps++) {
                ClearKeyboardBuffer((uint)KeysEx.VK_DECIMAL, rgKey[(uint)KeysEx.VK_DECIMAL].SC, hkl);
                FillKeyState(lpKeyState, ss, (caps!= 0));
                sbBuffer = new StringBuilder(10);
                int rc = ToUnicodeEx((uint)rgKey[iKey].VK, rgKey[iKey].SC,
                                     lpKeyState, sbBuffer, sbBuffer.Capacity, 0, hkl);
                if (rc > 0) {
                  if (sbBuffer.Length == 0) {
                    // Someone defined NULL on the keyboard; let's coddle them
                    rgKey[iKey].SetShiftState(ss, "\u0000", false, (caps != 0));
                  } else {
                    if ((rc == 1) &&
                        (ss == ShiftState.LCtrl || ss == ShiftState.ShftLCtrl) &&
                        ((int)rgKey[iKey].VK == ((uint)sbBuffer[0] + 0x40))) {
                      // ToUnicodeEx has an internal knowledge about those
                      // VK_A ~ VK_Z keys to produce the control characters,
                      // when the conversion rule is not provided in keyboard
                      // layout files
                      continue;
                    }
                    rgKey[iKey].SetShiftState(ss, sbBuffer.ToString().Substring(0, rc), false, (caps != 0));
                  }
                } else if (rc < 0) {
                  rgKey[iKey].SetShiftState(ss, sbBuffer.ToString().Substring(0, 1), true, (caps != 0));
                  // It's a dead key; let's flush out whats stored in the keyboard state.
                  ClearKeyboardBuffer((uint)KeysEx.VK_DECIMAL, rgKey[(uint)KeysEx.VK_DECIMAL].SC, hkl);
                  DeadKey dk = null;
                  for (int iDead = 0; iDead < alDead.Count; iDead++) {
                    dk = (DeadKey)alDead[iDead];
                    if (dk.DeadCharacter == rgKey[iKey].GetShiftState(ss, caps != 0)[0]) {
                      break;
                    }
                    dk = null;
                  }
                  if (dk == null) {
                    alDead.Add(ProcessDeadKey(iKey, ss, lpKeyState, rgKey, caps == 1, hkl));
                  }
                }
              }
            }
          }
        }

        foreach (IntPtr i in rghkl) {
          if (hkl == i) {
            hkl = IntPtr.Zero;
            break;
          }
        }

        if (hkl != IntPtr.Zero) {
          UnloadKeyboardLayout(hkl);
        }

        // Okay, now we can dump the layout
        int numrows = 0;
        int inumMC2 = 0;
        int inumMC3 = 0;
        int inumMC4 = 0;

        for (uint iKey = 0; iKey < rgKey.Length; iKey++) {
          if (rgKey[iKey] == null ||
              rgKey[iKey].IsEmpty ||
              rgKey[iKey].VK == KeysEx.VK_CANCEL ||
              rgKey[iKey].VK == KeysEx.VK_BACK ||
              rgKey[iKey].VK == KeysEx.VK_TAB ||
              rgKey[iKey].VK == KeysEx.VK_RETURN ||
              rgKey[iKey].VK == KeysEx.VK_ESCAPE ||
              rgKey[iKey].VK == KeysEx.VK_MULTIPLY ||
              rgKey[iKey].VK == KeysEx.VK_ADD ||
              rgKey[iKey].VK == KeysEx.VK_SUBTRACT ||
              rgKey[iKey].VK == KeysEx.VK_DECIMAL ||
              rgKey[iKey].VK == KeysEx.VK_DIVIDE ||
              (rgKey[iKey].VK >= KeysEx.VK_NUMPAD0 &&
               rgKey[iKey].VK <= KeysEx.VK_NUMPAD9)) {
            continue;
          }
          numrows++;
        }
        max_rows = max_rows > numrows ? max_rows : numrows;
        sw_OSK_KB_Data.Write("  {0}, {1}", ires, numrows);

        for (uint iKey = 0; iKey < rgKey.Length; iKey++) {
          if (rgKey[iKey] == null ||
              rgKey[iKey].IsEmpty ||
              rgKey[iKey].VK == KeysEx.VK_CANCEL ||
              rgKey[iKey].VK == KeysEx.VK_BACK ||
              rgKey[iKey].VK == KeysEx.VK_TAB ||
              rgKey[iKey].VK == KeysEx.VK_RETURN ||
              rgKey[iKey].VK == KeysEx.VK_ESCAPE ||
              rgKey[iKey].VK == KeysEx.VK_MULTIPLY ||
              rgKey[iKey].VK == KeysEx.VK_ADD ||
              rgKey[iKey].VK == KeysEx.VK_SUBTRACT ||
              rgKey[iKey].VK == KeysEx.VK_DECIMAL ||
              rgKey[iKey].VK == KeysEx.VK_DIVIDE ||
              (rgKey[iKey].VK >= KeysEx.VK_NUMPAD0 &&
               rgKey[iKey].VK <= KeysEx.VK_NUMPAD9)) {
            continue;
          } else {
            int ichar1, ichar2, ichar3, ichar4;
            string header = rgKey[iKey].GetRowHeader;
            string chars1 = rgKey[iKey].Get_Standard_Chars1;
            ichar1 = Char_List.IndexOf(chars1);
            if (ichar1 == -1) {
              Char_List.Add(chars1);
              iOffset++;
              ichar1 = iOffset;
              Char_Use_Count.Add(1);
            } else
              Char_Use_Count[ichar1] = Char_Use_Count[ichar1] + 1;

            string chars2 = rgKey[iKey].Get_Standard_Chars2;
            ichar2 = Char_List.IndexOf(chars2);
            if (ichar2 == -1) {
              Char_List.Add(chars2);
              iOffset++;
              ichar2 = iOffset;
              Char_Use_Count.Add(1);
            } else
              Char_Use_Count[ichar2] = Char_Use_Count[ichar2] + 1;

            string chars3 = rgKey[iKey].Get_Standard_Chars3;
            ichar3 = Char_List.IndexOf(chars3);
            if (ichar3 == -1) {
              Char_List.Add(chars3);
              iOffset++;
              ichar3 = iOffset;
              Char_Use_Count.Add(1);
            } else
              Char_Use_Count[ichar3] = Char_Use_Count[ichar3] + 1;

            string chars4 = rgKey[iKey].Get_Standard_Chars4;
            ichar4 = Char_List.IndexOf(chars4);
            if (ichar4 == -1) {
              Char_List.Add(chars4);
              iOffset++;
              ichar4 = iOffset;
              Char_Use_Count.Add(1);
            } else
              Char_Use_Count[ichar4] = Char_Use_Count[ichar4] + 1;

            if (rgKey[iKey].Get_Multi_Chars_Count > 0) {
              List<st_multi_chars> Multi_Chars_List = rgKey[iKey].Get_Multi_Chars_List;
              for (int i = 0; i < Multi_Chars_List.Count; i++) {
                int index;
                switch (Multi_Chars_List[i].num) {
                  case 2:
                    if (inumMC2 == 0) {
                      sw_OSK_KB_KLID2SCSS2MC2Data.WriteLine(" {{ 0x{0}, &vctSCSS2MC2_{0} }},  // {1,4}",
                                                            kbl.Key, inumKLID2MC2);
                      sw_OSK_KB_SCSS2MC2Data.WriteLine("\r\nstatic const st_SCSS2Offset dtaSCSS2MC2_{0}[] = {{",
                                                       kbl.Key);
                      inumKLID2MC2++;
                    }

                    index = Multi2Char_List.IndexOf(Multi_Chars_List[i].mchars);
                    if (index == -1) {
                      Multi2Char_List.Add(Multi_Chars_List[i].mchars);
                      iMC2_Offset++;
                      index = iMC2_Offset;
                      iCountMC2++;
                      Multi2Char_Use_Count.Add(1);
                    } else {
                      Multi2Char_Use_Count[index] = Multi2Char_Use_Count[index] + 1;
                    }
                    sw_OSK_KB_SCSS2MC2Data.WriteLine(" {{ 0x{0:x4}, {1,3} }},  // {2,4}",
                                                     Multi_Chars_List[i].scss, index, inumMC2);
                    inumMC2++;
                    break;
                  case 3:
                    if (inumMC3 == 0) {
                      sw_OSK_KB_KLID2SCSS2MC3Data.WriteLine(" {{ 0x{0}, &vctSCSS2MC3_{0} }},  // {1,4}",
                                                            kbl.Key, inumKLID2MC3);
                      sw_OSK_KB_SCSS2MC3Data.WriteLine("\r\nstatic const st_SCSS2Offset dtaSCSS2MC3_{0}[] = {{",
                                                       kbl.Key);
                      inumKLID2MC3++;
                    }

                    index = Multi3Char_List.IndexOf(Multi_Chars_List[i].mchars);
                    if (index == -1) {
                      Multi3Char_List.Add(Multi_Chars_List[i].mchars);
                      iMC3_Offset++;
                      index = iMC3_Offset;
                      iCountMC3++;
                      Multi3Char_Use_Count.Add(1);
                    } else {
                      Multi3Char_Use_Count[index] = Multi3Char_Use_Count[index] + 1;
                    }
                    sw_OSK_KB_SCSS2MC3Data.WriteLine(" {{ 0x{0:x4}, {1,3} }},  // {2,4}",
                                                     Multi_Chars_List[i].scss, index, inumMC3);
                    inumMC3++;
                    break;
                  case 4:
                    if (inumMC4 == 0) {
                      sw_OSK_KB_KLID2SCSS2MC4Data.WriteLine(" {{ 0x{0}, &vctSCSS2MC4_{0} }}, // {1,4}",
                                                            kbl.Key, inumKLID2MC4);
                      sw_OSK_KB_SCSS2MC4Data.WriteLine("\r\nstatic const st_SCSS2Offset dtaSCSS2MC4_{0}[] = {{",
                                                       kbl.Key);
                      inumKLID2MC4++;
                    }

                    index = Multi4Char_List.IndexOf(Multi_Chars_List[i].mchars);
                    if (index == -1) {
                      Multi4Char_List.Add(Multi_Chars_List[i].mchars);
                      iMC4_Offset++;
                      index = iMC4_Offset;
                      iCountMC4++;
                      Multi4Char_Use_Count.Add(1);
                    } else {
                      Multi4Char_Use_Count[index] = Multi4Char_Use_Count[index] + 1;
                    }
                    sw_OSK_KB_SCSS2MC4Data.WriteLine(" {{ 0x{0:x4}, {1,3} }},  // {2,4}",
                                                     Multi_Chars_List[i].scss, index, inumMC4);
                    inumMC4++;
                    break;
                }
              }
            }
            sw_OSK_KB_Data.Write(",\r\n  {0} {1,4}, {2,4}, {3,4}, {4,4}", header, ichar1, ichar2, ichar3, ichar4);
          }
        }
        if (inumMC2 > 0) {
          sw_OSK_KB_SCSS2MC2Data.WriteLine("};\r\n");
          sw_OSK_KB_SCSS2MC2Data.WriteLine("static const Vct_ISCSS2MC vctSCSS2MC2_{0}(dtaSCSS2MC2_{0},", kbl.Key);
          sw_OSK_KB_SCSS2MC2Data.Write("                                               ");
          sw_OSK_KB_SCSS2MC2Data.WriteLine("dtaSCSS2MC2_{0} + {1});", kbl.Key, inumMC2);
        }

        if (inumMC3 > 0) {
          sw_OSK_KB_SCSS2MC3Data.WriteLine("};\r\n");
          sw_OSK_KB_SCSS2MC3Data.WriteLine("static const Vct_ISCSS2MC vctSCSS2MC3_{0}(dtaSCSS2MC3_{0},", kbl.Key);
          sw_OSK_KB_SCSS2MC3Data.Write("                                               ");
          sw_OSK_KB_SCSS2MC3Data.WriteLine("dtaSCSS2MC3_{0} + {1});", kbl.Key, inumMC3);
        }

        if (inumMC4 > 0) {
          sw_OSK_KB_SCSS2MC4Data.WriteLine("};\r\n");
          sw_OSK_KB_SCSS2MC4Data.WriteLine("static const Vct_ISCSS2MC vctSCSS2MC4_{0}(dtaSCSS2MC4_{0},", kbl.Key);
          sw_OSK_KB_SCSS2MC4Data.Write("                                               ");
          sw_OSK_KB_SCSS2MC4Data.WriteLine("dtaSCSS2MC4_{0} + {1});", kbl.Key, inumMC4);
        }

        if (alDead.Count > 0) {
          sw_OSK_Define_DeadKey_DataMaps.WriteLine("  static Map_IDK2SCSS m_map_IDK2SCSS_{0};", kbl.Key);
        }

        bool bFirst = alDead.Count > 0;
        foreach (DeadKey dk in alDead) {
          if (bFirst) {
            sw_OSK_DeadKey_Data.WriteLine("\r\n//\r\n// Keyboard {0} - {1} deadkeys\r\n//", kbl.Key, kbl.Name);
            sw_OSK_Insert_DeadKey_DataMaps.WriteLine("\r\n  m_mapIKLID2DK2SCSS.insert(std::make_pair(0x{0}, &m_map_IDK2SCSS_{0}));",
                                                    kbl.Key);
            bFirst = false;
          }

          char wcdk;
          int index;
          uint scss = 0;

          // Initialise enumerator
          dk.GetDeadKeyInfo(ref scss, out wcdk);
          sw_OSK_DeadKey_Data.WriteLine("\r\n// Deadkey 0x{0:x4}", ((ushort)dk.DeadCharacter).ToString("x4"));
          sw_OSK_DeadKey_Data.WriteLine("static const st_IDKSCSS2Offset dtaDeadkey_{0}_x{1}[{2}] = {{",
                                        kbl.Key, ((ushort)dk.DeadCharacter).ToString("x4"), dk.Count);

          int inumDK = 0;
          for (int id = 0; id < dk.Count; id++) {
            scss = 1;
            dk.GetDeadKeyInfo(ref scss, out wcdk);
            if (scss == 0)
              continue;

            iCountDK++;
            index = DeadKey_List.IndexOf(wcdk);
            if (index == -1) {
              DeadKey_List.Add(wcdk);
              iDK_Offset++;
              index = iDK_Offset;
              DeadKey_Use_Count.Add(1);
            } else {
              DeadKey_Use_Count[index] = DeadKey_Use_Count[index] + 1;
            }
            byte sc = (byte)((scss & 0xFF00) >> 8);
            byte ss = (byte)(scss & 0xFF);
            sw_OSK_DeadKey_Data.WriteLine("  {{ 0x{0:x2}, 0x{1:x2}, {2,4} }},  // {3,4}", sc, ss, index, inumDK++);
          }
          sw_OSK_DeadKey_Data.WriteLine("};\r\n");
          sw_OSK_DeadKey_Data.WriteLine("static const Vct_IDeadkeys vctDeadkey_{0}_x{1}(dtaDeadkey_{0}_x{1},", kbl.Key,
                                        ((ushort)dk.DeadCharacter).ToString("x4"));
          sw_OSK_DeadKey_Data.Write("                                                     ");
          sw_OSK_DeadKey_Data.WriteLine("dtaDeadkey_{0}_x{1} + {2});", kbl.Key,
                                        ((ushort)dk.DeadCharacter).ToString("x4"), dk.Count);
          sw_OSK_Insert_DeadKey_DataMaps.WriteLine("  m_map_IDK2SCSS_{0}.insert(std::make_pair(0x{1:x4}, &vctDeadkey_{0}_x{1}));",
                                                   kbl.Key, ((ushort)dk.DeadCharacter).ToString("x4"));
        }
        sw_OSK_KB_Data.WriteLine("\r\n};");
        sw_OSK_KB_Data.WriteLine();
      }

      // Process XML Custom keyboards
      // Expects 2 files in this executable's directory:
      // user_keyboard.xml and user_keyboards.xsd
      // See these for details of their format
      if (!File.Exists("user_keyboards.xml") || !File.Exists("user_keyboards.xsd")) {
        sw_log.WriteLine("Either or both of the files 'user_keyboards.xml/xsd' are missing");
        sw_log.WriteLine("No processing of User Keyboards via a XML file will be performed.");
        goto finish_up;
      }

      ProcessXML xml_vld = new ProcessXML("user_keyboards.xsd");

      xml_vld.DoXML(true, "user_keyboards.xml");
      if (xml_vld.Results.Count > 0) {
        sw_log.WriteLine("Processing User Keyboards input via XML file encountered errors:");
        foreach (string res in xml_vld.Results) {
          sw_log.WriteLine("{0}", res);
        }
      }

      if (xml_vld.Results.Count == 0) {
        sw_log.WriteLine("Processing User Keyboards input via XML file:");
        xml_vld.DoXML(false, "user_keyboards.xml");

        foreach (st_XML_Keyboard xmlkbd in xml_vld.XMLKeyboards) {
          sw_OSK_KLID2VKBDX_Table.WriteLine("  {{0x{0}, &VKBD_{0}}},  // {1,4}", xmlkbd.KLID,
                                            iCountMain);
          iCountMain++;

          Console.WriteLine("Processing Keyboard Layout - {0}:{1}", xmlkbd.KLID, xmlkbd.Name);
          sw_log.WriteLine("Processing Keyboard Layout - {0}:{1}", xmlkbd.KLID, xmlkbd.Name);

          sw_OSK_KB_Data.WriteLine("// Keyboard Layouts - {0}:{1}", xmlkbd.KLID, xmlkbd.Name);
          sw_OSK_KB_Data.WriteLine("static st_IVKBD VKBD_{0} = {{", xmlkbd.KLID);
          int ires = IDS_BASE + iCount;
          sw_VKresource.WriteLine("#define IDS_VKB_{0}                {1,4}",
                                  xmlkbd.KLID, ires);
          sw_VKPasswordSafe3.WriteLine("  IDS_VKB_{0}        \"{1}\"",
                                     xmlkbd.KLID, xmlkbd.Name);
          // Remember 2 special keyboards for later
          if (xmlkbd.Name == "Korean")
            strKorean = xmlkbd.KLID;
          if (xmlkbd.Name == "Japanese")
            strJapanese = xmlkbd.KLID;
          iCount++;

          max_rows = max_rows > xmlkbd.KeyData.Count ? max_rows : xmlkbd.KeyData.Count;
          sw_OSK_KB_Data.Write("  {0}, {1}", ires, xmlkbd.KeyData.Count);
          for (int i = 0; i < xmlkbd.KeyData.Count; i++) {
            int ichar1, ichar2, ichar3, ichar4;
            ichar1 = Char_List.IndexOf(xmlkbd.KeyData[i].chars1);
            if (ichar1 == -1) {
              Char_List.Add(xmlkbd.KeyData[i].chars1);
              iOffset++;
              ichar1 = iOffset;
              Char_Use_Count.Add(1);
            } else
              Char_Use_Count[ichar1] = Char_Use_Count[ichar1] + 1;

            ichar2 = Char_List.IndexOf(xmlkbd.KeyData[i].chars2);
            if (ichar2 == -1) {
              Char_List.Add(xmlkbd.KeyData[i].chars2);
              iOffset++;
              ichar2 = iOffset;
              Char_Use_Count.Add(1);
            } else
              Char_Use_Count[ichar2] = Char_Use_Count[ichar2] + 1;

            ichar3 = Char_List.IndexOf(xmlkbd.KeyData[i].chars3);
            if (ichar3 == -1) {
              Char_List.Add(xmlkbd.KeyData[i].chars3);
              iOffset++;
              ichar3 = iOffset;
              Char_Use_Count.Add(1);
            } else
              Char_Use_Count[ichar3] = Char_Use_Count[ichar3] + 1;

            ichar4 = Char_List.IndexOf(xmlkbd.KeyData[i].chars4);
            if (ichar4 == -1) {
              Char_List.Add(xmlkbd.KeyData[i].chars4);
              iOffset++;
              ichar4 = iOffset;
              Char_Use_Count.Add(1);
            } else
              Char_Use_Count[ichar4] = Char_Use_Count[ichar4] + 1;

            sw_OSK_KB_Data.Write(",\r\n  0x" + xmlkbd.KeyData[i].sc + ",      0,");
            sw_OSK_KB_Data.Write(" {0,4}, {1,4}, {2,4}, {3,4}", ichar1, ichar2, ichar3, ichar4);
          }
          sw_OSK_KB_Data.WriteLine("\r\n};");
          sw_OSK_KB_Data.WriteLine();
        }
      }

  finish_up:
      sw_OSK_KLID2VKBDX_Table.WriteLine("};");

      if (iCountMC2 < 1)
        sw_OSK_KB_KLID2SCSS2MC2Data.WriteLine(" {0, NULL}  // Dummy");
      sw_OSK_KB_KLID2SCSS2MC2Data.WriteLine("};");

      if (iCountMC3 < 1)
        sw_OSK_KB_KLID2SCSS2MC3Data.WriteLine(" {0, NULL}  // Dummy");
      sw_OSK_KB_KLID2SCSS2MC3Data.WriteLine("};");

      if (iCountMC4 < 1)
        sw_OSK_KB_KLID2SCSS2MC4Data.WriteLine(" {0, NULL}  // Dummy");
      sw_OSK_KB_KLID2SCSS2MC4Data.WriteLine("};");

      // Write out the standard and extended arrays
      sw_OSK_KB_Data.WriteLine("static const wchar_t wc_Chars[{0}][4] = {{", iOffset + 1);
      for (int i = 0; i < iOffset; i++) {
        sw_OSK_KB_Data.WriteLine("  {{{0}}},  // {1,4}; Use Count = {2,5}",
                                 Char_List[i], i, Char_Use_Count[i]);
      }
      if (iOffset > 0)
        sw_OSK_KB_Data.WriteLine("  {{{0}}}   // {1,4}; Use Count = {2,5}",
                                 Char_List[iOffset], iOffset, Char_Use_Count[iOffset]);
      else
        sw_OSK_KB_Data.WriteLine("  {0, 0, 0, 0}   // Dummy");
      sw_OSK_KB_Data.WriteLine("};\r\n");

      int isize;
      isize = (iMC2_Offset) < 0 ? 1 : (iMC2_Offset + 1);

      sw_OSK_KB_SCSS2MC2Data.WriteLine("\r\nstatic const wchar_t wcMC2[{0}][2] = {{", isize);
      if (Multi2Char_List.Count > 0) {
        for (int i = 0; i < iMC2_Offset; i++) {
          sw_OSK_KB_SCSS2MC2Data.WriteLine("  {{{0}}},  // {1,4}; Use Count = {2,3}",
                                           Multi2Char_List[i], i, Multi2Char_Use_Count[i]);
        }
        sw_OSK_KB_SCSS2MC2Data.WriteLine("  {{{0}}}   // {1,4}; Use Count = {2,3}",
                                         Multi2Char_List[iMC2_Offset], iMC2_Offset,
                                         Multi2Char_Use_Count[iMC2_Offset]);
      } else {
        sw_OSK_KB_SCSS2MC2Data.WriteLine("  {0, 0}   // Dummy");
      }
      sw_OSK_KB_SCSS2MC2Data.WriteLine("};");

      isize = (iMC3_Offset) < 0 ? 1 : (iMC3_Offset + 1);
      sw_OSK_KB_SCSS2MC3Data.WriteLine("\r\nstatic const wchar_t wcMC3[{0}][3] = {{", isize);
      if (Multi3Char_List.Count > 0) {
        for (int i = 0; i < iMC3_Offset; i++) {
          sw_OSK_KB_SCSS2MC3Data.WriteLine("  {{{0}}},  // {1,4}; Use Count = {2,3}",
                                           Multi3Char_List[i], i, Multi3Char_Use_Count[i]);
        }
        sw_OSK_KB_SCSS2MC3Data.WriteLine("  {{{0}}}   // {1,4}; Use Count = {2,3}",
                                         Multi3Char_List[iMC3_Offset], iMC3_Offset,
                                         Multi3Char_Use_Count[iMC3_Offset]);
      } else {
        sw_OSK_KB_SCSS2MC3Data.WriteLine("  {0, 0, 0}   // Dummy");
      }
      sw_OSK_KB_SCSS2MC3Data.WriteLine("};");

      isize = (iMC4_Offset) < 0 ? 1 : (iMC4_Offset + 1);
      sw_OSK_KB_SCSS2MC4Data.WriteLine("\r\nstatic const wchar_t wcMC4[{0}][4] = {{", isize);
      if (Multi4Char_List.Count > 0) {
        for (int i = 0; i < iMC4_Offset; i++) {
          sw_OSK_KB_SCSS2MC4Data.WriteLine("  {{{0}}},  // {1,4}; Use Count = {2,3}",
                                           Multi4Char_List[i], i, Multi4Char_Use_Count[i]);
        }
        sw_OSK_KB_SCSS2MC4Data.WriteLine("  {{{0}}}   // {1,4}; Use Count = {2,3}",
                                         Multi4Char_List[iMC4_Offset], iMC4_Offset,
                                         Multi4Char_Use_Count[iMC4_Offset]);
      } else {
        sw_OSK_KB_SCSS2MC4Data.WriteLine("  {0, 0, 0, 0}   // Dummy");
      }
      sw_OSK_KB_SCSS2MC4Data.WriteLine("};");

      isize = (iDK_Offset) < 0 ? 1 : (iDK_Offset + 1);
      sw_OSK_DeadKey_Data.WriteLine("\r\nstatic const wchar_t wcDK[{0}] = {{", isize);

      if (DeadKey_List.Count > 0) {
        for (int i = 0; i < iDK_Offset; i++) {
          sw_OSK_DeadKey_Data.WriteLine("  {{ 0x{0:x4} }},  // {1,4}; Use Count = {2,3}",
                                       ((ushort)DeadKey_List[i]).ToString("x4"),
                                        i, DeadKey_Use_Count[i]);
        }
        sw_OSK_DeadKey_Data.WriteLine("  {{ 0x{0:x4} }}   // {1,4}; Use Count = {2,3}",
                                      ((ushort)DeadKey_List[iDK_Offset]).ToString("x4"),
                                      iDK_Offset, DeadKey_Use_Count[iDK_Offset]);
      } else {
        sw_OSK_DeadKey_Data.WriteLine("  {0}   // Dummy");
      }
      sw_OSK_DeadKey_Data.WriteLine("};");

      StreamWriter sw_OSK_voskeys = new StreamWriter("OSK_voskeys.inc");
      sw_OSK_voskeys.WriteLine(sBanner, version, dateTime);
      sw_OSK_voskeys.WriteLine("#define NUM_KEYBOARDS              {0,4}", iCount);
      sw_OSK_voskeys.WriteLine("#define MAX_ROWS                   {0,4}", max_rows);
      sw_OSK_voskeys.WriteLine();
      sw_OSK_voskeys.WriteLine("#define NUM_UNIQUE_SC              {0,4}", iOffset + 1);
      sw_OSK_voskeys.WriteLine("#define NUM_UNIQUE_MC2             {0,4}", iMC2_Offset + 1);
      sw_OSK_voskeys.WriteLine("#define NUM_UNIQUE_MC3             {0,4}", iMC3_Offset + 1);
      sw_OSK_voskeys.WriteLine("#define NUM_UNIQUE_MC4             {0,4}", iMC4_Offset + 1);
      sw_OSK_voskeys.WriteLine("#define NUM_UNIQUE_DK              {0,4}", iDK_Offset + 1);
      sw_OSK_voskeys.WriteLine();
      sw_OSK_voskeys.WriteLine("#define NUM_MC2                    {0,4}", inumKLID2MC2);
      sw_OSK_voskeys.WriteLine("#define NUM_MC3                    {0,4}", inumKLID2MC3);
      sw_OSK_voskeys.WriteLine("#define NUM_MC4                    {0,4}", inumKLID2MC4);
      sw_OSK_voskeys.WriteLine();
      sw_OSK_voskeys.WriteLine("#define KOREAN_KBD           0x" + strKorean);
      sw_OSK_voskeys.WriteLine("#define JAPANESE_KBD         0x" + strJapanese);

      if (iCount % 10 != 0)
        sw_VKPasswordSafe3.WriteLine("END");

      sw_VKresource.WriteLine();

      // Close files
      sw_OSK_KLID2VKBDX_Table.Close();
      sw_OSK_Define_DeadKey_DataMaps.Close();
      sw_OSK_Insert_DeadKey_DataMaps.Close();
      sw_OSK_DeadKey_Data.Close();

      sw_OSK_voskeys.Close();

      sw_OSK_KB_Data.Close();
      sw_OSK_KB_KLID2SCSS2MC2Data.Close();
      sw_OSK_KB_KLID2SCSS2MC3Data.Close();
      sw_OSK_KB_KLID2SCSS2MC4Data.Close();
      sw_OSK_KB_SCSS2MC2Data.Close();
      sw_OSK_KB_SCSS2MC3Data.Close();
      sw_OSK_KB_SCSS2MC4Data.Close();

      sw_VKPasswordSafe3.Close();
      sw_VKresource.Close();

      sw_log.Close();
    }
  }
}
