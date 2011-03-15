'Update the Resource Only DLLs for each language we support

Dim TOOLS, RESTEXT, RESPWSL, BASE_DLL, DEST_DIR
Dim objFSO

TOOLS = "..\..\..\..\build\bin"
RESTEXT = TOOLS & "\restext\release\ResText.exe"
RESPWSL = TOOLS & "\respwsl\release\ResPWSL.exe"
BASE_DLL = "..\..\..\..\build\bin\pwsafe\release\pwsafe_base.dll"
DEST_DIR = "..\..\..\..\build\bin\pwsafe\I18N\"

Set objFSO = CreateObject("Scripting.FileSystemObject")

If (objFSO.FileExists("foo.dll")) Then
  'Delete intermediate DLL if still there
  objFSO.DeleteFile "foo.dll"
End If

' Now do them
Call DoI18N("de", "0x0407", "DE_DE", "DE")
Call DoI18N("dk", "0x0406", "DA_DK", "DA")
Call DoI18N("es", "0x0c0a", "ES_ES", "ES")
Call DoI18N("fr", "0x040c", "FR_FR", "FR")
Call DoI18N("it", "0x0410", "IT_IT", "IT")
Call DoI18N("kr", "0x0412", "KO_KR", "KR")
Call DoI18N("nl", "0x0413", "NL_NL", "NL")
Call DoI18N("pl", "0x0415", "PL_PL", "PL")
Call DoI18N("ru", "0x0419", "RU_RU", "RV")
Call DoI18N("sv", "0x041d", "SV_SE", "SV")
Call DoI18N("zh", "0x0804", "ZH_CN", "ZH")

' Delete FileSystemObject
Set objFSO = Nothing

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
     WScript.Sleep 100
Loop

' Create new DLL with correct name and update version information
Set oExec = WshShell.Exec(RESPWSL & " apply foo.dll " & LCID)

Do While oExec.Status = 0
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
