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

Dim CURPATH, TOOLS, RESTEXT, RESPWSL, BASE_DLL, DEST_DIR, DO_ALL, DO_COUNTRY
Dim objFSO

TOOLS = "..\..\..\..\build\bin"
RESTEXT = TOOLS & "\restext\release\ResText.exe"
RESPWSL = TOOLS & "\respwsl\release\ResPWSL.exe"
BASE_DLL = "..\..\..\..\build\bin\pwsafe\release\pwsafe_base.dll"
DEST_DIR = "..\..\..\..\build\bin\pwsafe\I18N\"

Set objFSO = CreateObject("Scripting.FileSystemObject")

' Make absolute addresses
DEST_DIR = objFSO.GetAbsolutePathName(DEST_DIR)
DEST_DIR = DEST_DIR & "\"

CURPATH = objFSO.GetAbsolutePathName(".")
CURPATH = CURPATH & "\"

' Check output directory exists and, if not, create it
CreateFolder(DEST_DIR)
If (Not objFSO.FileExists(BASE_DLL)) Then
  ' Check Base DLL exists
  WScript.Echo "Can't find the Base DLL - pwsafe_base.dll"
  WScript.Quit(99)
End If

If (Not objFSO.FileExists(RESTEXT)) Then
  ' Check required program exists
  WScript.Echo "Can't find Tool - ResText.exe"
  WScript.Quit(99)
End If

If (Not objFSO.FileExists(RESPWSL)) Then
  ' Check required program exists
  WScript.Echo "Can't find Tool - ResPWSL.exe"
  WScript.Quit(99)
End If

If Wscript.Arguments.Count = 0 Then
  DO_ALL = True
Else
  DIM Arg1
  Arg1 = UCase(Wscript.Arguments(0))
 If Arg1 = "/HELP" Or Arg1 = "/H" Or Arg1 = "/?" Then
   Usage
   Wscript.Quit(0)
  Else
    DO_ALL = False
    DO_COUNTRY = Arg1
  End If
End If

' Sometimes the delete files do not work for permission failures. So...
WScript.Echo "*** Please ensure that you have deleted ALL DLLs from this script directory:"
WScript.Echo  " " & CURPATH
WScript.Echo  " "
WScript.Echo "*** Please delete any DLLs you are (re)making from the target directory:"
WScript.Echo  " " & DEST_DIR
WScript.Echo  " "
Pause("Press any key to continue.")

If (objFSO.FileExists("foo.dll")) Then
  'Delete intermediate DLL if still there
  objFSO.DeleteFile "foo.dll"
End If

Dim objStdOut, DoneSome
Set objStdOut = WScript.StdOut

DoneSome = false

' Now do them

If (DO_ALL = True Or DO_COUNTRY = "CZ") Then
  objStdOut.WriteLine " Creating Czech Language DLL"
  Call DoI18N("cz", "0x0405", "CS_CZ", "CZ")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "DA") Then
  objStdOut.WriteLine " Creating Danish Language DLL"
  Call DoI18N("dk", "0x0406", "DA_DK", "DA")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "DE") Then
  objStdOut.WriteLine " Creating German Language DLL"
  Call DoI18N("de", "0x0407", "DE_DE", "DE")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "ES") Then
  objStdOut.WriteLine " Creating Spanish Language DLL"
  Call DoI18N("es", "0x0c0a", "ES_ES", "ES")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "FR") Then
  objStdOut.WriteLine " Creating French Language DLL"
  Call DoI18N("fr", "0x040c", "FR_FR", "FR")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "IT") Then
  objStdOut.WriteLine " Creating Italian Language DLL"
  Call DoI18N("it", "0x0410", "IT_IT", "IT")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "KR") Then
  objStdOut.WriteLine " Creating Korean Language DLL"
  Call DoI18N("kr", "0x0412", "KO_KR", "KO")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "NL") Then
  objStdOut.WriteLine " Creating Dutch Language DLL"
  Call DoI18N("nl", "0x0413", "NL_NL", "NL")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "PL") Then
  objStdOut.WriteLine " Creating Polish Language DLL"
  Call DoI18N("pl", "0x0415", "PL_PL", "PL")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "RU") Then
  objStdOut.WriteLine " Creating Russian Language DLL"
  Call DoI18N("ru", "0x0419", "RU_RU", "RU")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "SV") Then
  objStdOut.WriteLine " Creating Swedish Language DLL"
  Call DoI18N("sv", "0x041d", "SV_SE", "SV")
  DoneSome = true
End If

If (DO_ALL = True Or DO_COUNTRY = "ZH") Then
  objStdOut.WriteLine " Creating Chinese (Simplified) Language DLL"
  Call DoI18N("zh", "0x0804", "ZH_CN", "ZH")
  DoneSome = true
End If

If DoneSome = true Then
  objStdOut.WriteLine " Processing Completed"
End If

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

If (Not objFSO.FileExists("pos\pwsafe_" & PO & ".po")) Then
  ' Check required PO file exists
  WScript.Echo "   Can't find requested PO file - pos\pwsafe_" & PO & ".po"
  objStdOut.WriteLine "   Skipped"
  Exit Sub
End If

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

If (objFSO.FileExists("foo.dll")) Then
  'Delete intermediate DLL if still there
  On Error Resume Next
  objFSO.DeleteFile "foo.dll"
End If

If (objFSO.FileExists(DEST_DIR & "pwsafe" & LL & ".dll")) Then
  ' Delete any old version of this language resource-only DLL
  On Error Resume Next
  objFSO.DeleteFile DEST_DIR & "pwsafe" & LL & ".dll"
End If

' Move and rename the new DLL
objFSO.MoveFile "pwsafe" & LL_CC & ".dll", DEST_DIR & "pwsafe" & LL & ".dll"

objStdOut.WriteLine "   Done"

Set oExec = Nothing
Set WshShell = Nothing

End Sub

' Recursive folder create, will create directories and Sub
Sub CreateFolder(strPath)
  On Error Resume Next
  If strPath <> "" Then 'Fixes endless recursion in some instances when at lowest directory
  If Not objFSO.FolderExists(objFSO.GetParentFolderName(strPath)) then Call CreateFolder(objFSO.GetParentFolderName(strPath))
    objFSO.CreateFolder(strPath)
  End If 
End Sub

Sub Pause(strPause)
 Dim z
  WScript.Echo (strPause)
  z = WScript.StdIn.Read(1)
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
