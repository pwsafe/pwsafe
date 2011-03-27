'Update the Resource Only DLLs for each language we support

Option Explicit

If Instr(1, WScript.FullName, "cscript.exe", vbTextCompare) = 0 then
    MsgBox "This script must be executed by cscript.exe" & vbCRLF & _
           "e.g. cscript Create_DLLs.vbs", _
           vbCritical, _
           "Error: " & Wscript.ScriptFullName
    ' return error code to caller
    Wscript.Quit(99)
End If

Dim TOOLS, RESTEXT, RESPWSL, BASE_DLL, DEST_DIR, DO_ALL, DO_COUNTRY
Dim objFSO

TOOLS = "..\..\..\..\build\bin"
RESTEXT = TOOLS & "\restext\release\ResText.exe"
RESPWSL = TOOLS & "\respwsl\release\ResPWSL.exe"
BASE_DLL = "..\..\..\..\build\bin\pwsafe\release\pwsafe_base.dll"
DEST_DIR = "..\..\..\..\build\bin\pwsafe\I18N\"

If Wscript.Arguments.Count = 0 Then
  DO_ALL = True
Else
  DIM Arg1
  Arg1 = UCase(Wscript.Arguments(0))
 If Arg1 = "/HELP" Then
   Usage
   Wscript.Quit(0)
  Else
    DO_ALL = False
    DO_COUNTRY = Arg1
  End If
End If

Set objFSO = CreateObject("Scripting.FileSystemObject")

If (objFSO.FileExists("foo.dll")) Then
  'Delete intermediate DLL if still there
  objFSO.DeleteFile "foo.dll"
End If

Dim objStdOut
Set objStdOut = WScript.StdOut

' Now do them
If (DO_ALL = True Or DO_COUNTRY = "DE") Then
objStdOut.WriteLine " Creating German Language DLL"
Call DoI18N("de", "0x0407", "DE_DE", "DE")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "DA") Then
objStdOut.WriteLine " Creating Danish Language DLL"
Call DoI18N("dk", "0x0406", "DA_DK", "DA")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "ES") Then
objStdOut.WriteLine " Creating Spanish Language DLL"
Call DoI18N("es", "0x0c0a", "ES_ES", "ES")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "FR") Then
objStdOut.WriteLine " Creating French Language DLL"
Call DoI18N("fr", "0x040c", "FR_FR", "FR")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "FR_CA") Then
objStdOut.WriteLine " Creating French (Canadian) Language DLL"
Call DoI18N("fr", "0x0c0c", "FR_CA", "FR_CA")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "IT") Then
objStdOut.WriteLine " Creating Italian Language DLL"
Call DoI18N("it", "0x0410", "IT_IT", "IT")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "KR") Then
objStdOut.WriteLine " Creating Korean Language DLL"
Call DoI18N("kr", "0x0412", "KO_KR", "KR")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "NL") Then
objStdOut.WriteLine " Creating Dutch Language DLL"
Call DoI18N("nl", "0x0413", "NL_NL", "NL")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "PL") Then
objStdOut.WriteLine " Creating Polish Language DLL"
Call DoI18N("pl", "0x0415", "PL_PL", "PL")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "RU") Then
objStdOut.WriteLine " Creating Russian Language DLL"
Call DoI18N("ru", "0x0419", "RU_RU", "RU")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "SV") Then
objStdOut.WriteLine " Creating Swedish Language DLL"
Call DoI18N("sv", "0x041d", "SV_SE", "SV")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "ZH") Then
objStdOut.WriteLine " Creating Chinese Language DLL"
Call DoI18N("zh", "0x0804", "ZH_CN", "ZH")
objStdOut.WriteLine "   Done"
End If
objStdOut.WriteLine " Processing Completed"

' Delete FileSystemObject
Set objFSO = Nothing
Set objStdOut = Nothing

WScript.Quit(0)

Sub DoI18N(PO, LCID, LL_CC, LL)
' Parameters:
' 1. PO suffix of file in sub-directory "pos" e.g. 'zh' for "pwsafe_zh.po"
' 2. LCID e.g. 0x0804 for Chinese (Simplified)
' 3. Generated DLL in form LL_CC e.g. "ZH_CN" for "pwsafeZH_CN.dll"
'    NOTE: This is determined by the LCID and is not a free choice! See Windows XP version of
'    http://www.microsoft.com/resources/msdn/goglobal/default.mspx
'    as this generates the 2-character LL and CC values (later OSes can generate other values).
' 4. Final DLL name in form LL e.g. "ZH" for "pwsafeZH.dll"

Dim WshShell, oExec
Set WshShell = CreateObject("WScript.Shell")

' Create new intermediate DLL using PO file to replace strings
Set oExec = WshShell.Exec(RESTEXT & " apply " & BASE_DLL & " foo.dll " & "pos\pwsafe_" & PO & ".po")

Do While oExec.Status = 0
  objStdOut.Write ".."
  WScript.Sleep 100
Loop

' Create new DLL with correct name and update version information
Set oExec = WshShell.Exec(RESPWSL & " apply foo.dll " & LCID)

Do While oExec.Status = 0
  objStdOut.Write ".."
  WScript.Sleep 100
Loop

Set oExec = Nothing
Set WshShell = Nothing

If (objFSO.FileExists("foo.dll")) Then
  'Delete intermediate DLL if still there
  objFSO.DeleteFile "foo.dll"
End If

If (objFSO.FileExists(DEST_DIR & "pwsafe" & LL & ".dll")) Then
  ' Delete any old version of this language resource-only DLL
  objFSO.DeleteFile DEST_DIR & "pwsafe" & LL & ".dll"
End If

' Move and rename the new DLL
objFSO.MoveFile "pwsafe" & LL_CC & ".dll", DEST_DIR & "pwsafe" & LL & ".dll"

End Sub

Sub Usage
  MsgBox "This script must be executed by cscript.exe in a Command window, either:" & vbCRLF & _
         "'cscript Create_DLLs.vbs' to create all supported language DLLs, or" & vbCRLF & _
         "'cscript Create_DLLs.vbs country-code' to create a specific supported language DLL" & vbCRLF & _
         "e.g 'cscript Create_DLLs.vbs DE' for German/Germany, or" & vbCRLF & _
         "'cscript Create_DLLs.vbs /Help' for this message", _
         vbInformation, _
         "Help: " & Wscript.ScriptFullName

End Sub