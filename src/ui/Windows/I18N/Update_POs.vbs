'Update the Resource Only DLLs for each language we support

Option Explicit

If Instr(1, WScript.FullName, "cscript.exe", vbTextCompare) = 0 then
    MsgBox "This script must be executed by cscript.exe" & vbCRLF & _
           "e.g. cscript UpdatePOs.vbs", _
           vbCritical, _
           "Error: " & Wscript.ScriptFullName
    ' return error code to caller
    Wscript.Quit(99)
End If

Dim TOOLS, RESTEXT, BASE_DLL, DO_ALL, DO_COUNTRY
Dim objFSO

TOOLS = "..\..\..\..\build\bin"
RESTEXT = TOOLS & "\restext\release\ResText.exe"
BASE_DLL = "..\..\..\..\build\bin\pwsafe\release\pwsafe_base.dll"

Set objFSO = CreateObject("Scripting.FileSystemObject")

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

If (objFSO.FileExists("pos\Update_POs.log")) Then
  'Delete log file
  objFSO.DeleteFile "pos\Update_POs.log"
End If

Dim objStdOut
Set objStdOut = WScript.StdOut

' Now do them
If (DO_ALL = True Or DO_COUNTRY = "DE") Then
  If (objFSO.FileExists("pos\pwsafe_de.po")) Then
    objStdOut.WriteLine " Updating German Language PO"
  Else
    objStdOut.WriteLine " Creating German Language PO"
  End If
  Call UpdatePO("DE")
End If
If (DO_ALL = True Or DO_COUNTRY = "DK") Then
  If (objFSO.FileExists("pos\pwsafe_dk.po")) Then
    objStdOut.WriteLine " Updating Danish Language PO"
  Else
    objStdOut.WriteLine " Creating Danish Language PO"
  End If
  Call UpdatePO("DK")
End If
If (DO_ALL = True Or DO_COUNTRY = "ES") Then
  If (objFSO.FileExists("pos\pwsafe_es.po")) Then
    objStdOut.WriteLine " Updating Spanish Language PO"
  Else
    objStdOut.WriteLine " Creating Spanish Language PO"
  End If
  Call UpdatePO("ES")
End If
If (DO_ALL = True Or DO_COUNTRY = "FR") Then
  If (objFSO.FileExists("pos\pwsafe_fr.po")) Then
    objStdOut.WriteLine " Updating French Language PO"
  Else
    objStdOut.WriteLine " Creating French Language PO"
  End If
  Call UpdatePO("FR")
End If
If (DO_ALL = True Or DO_COUNTRY = "FR_CA") Then
  If (objFSO.FileExists("pos\pwsafe_fr_ca.po")) Then
    objStdOut.WriteLine " Updating French (Canadian) Language PO"
  Else
    objStdOut.WriteLine " Creating French (Canadian) Language PO"
  End If
  Call UpdatePO("FR_CA")
End If
If (DO_ALL = True Or DO_COUNTRY = "IT") Then
  If (objFSO.FileExists("pos\pwsafe_it.po")) Then
    objStdOut.WriteLine " Updating Italian Language PO"
  Else
    objStdOut.WriteLine " Creating Italian Language PO"
  End If
  Call UpdatePO("IT")
End If
If (DO_ALL = True Or DO_COUNTRY = "KR") Then
  If (objFSO.FileExists("pos\pwsafe_kr.po")) Then
    objStdOut.WriteLine " Updating Korean Language PO"
  Else
    objStdOut.WriteLine " Creating Korean Language PO"
  End If
  Call UpdatePO("KR")
End If
If (DO_ALL = True Or DO_COUNTRY = "NL") Then
  If (objFSO.FileExists("pos\pwsafe_nl.po")) Then
    objStdOut.WriteLine " Updating Dutch Language PO"
  Else
    objStdOut.WriteLine " Creating Dutch Language PO"
  End If
  Call UpdatePO("NL")
End If
If (DO_ALL = True Or DO_COUNTRY = "PL") Then
  If (objFSO.FileExists("pos\pwsafe_pl.po")) Then
    objStdOut.WriteLine " Updating Polish Language PO"
  Else
    objStdOut.WriteLine " Creating Polish Language PO"
  End If
  Call UpdatePO("PL")
End If
If (DO_ALL = True Or DO_COUNTRY = "RU") Then
  If (objFSO.FileExists("pos\pwsafe_ru.po")) Then
    objStdOut.WriteLine " Updating Russian Language PO"
  Else
    objStdOut.WriteLine " Creating Russian Language PO"
  End If
  Call UpdatePO("RU")
End If
If (DO_ALL = True Or DO_COUNTRY = "SV") Then
  If (objFSO.FileExists("pos\pwsafe_sv.po")) Then
    objStdOut.WriteLine " Updating Swedish Language PO"
  Else
    objStdOut.WriteLine " Creating Swedish Language PO"
  End If
  Call UpdatePO("SV")
End If
If (DO_ALL = True Or DO_COUNTRY = "ZH") Then
  If (objFSO.FileExists("pos\pwsafe_zh.po")) Then
    objStdOut.WriteLine " Updating Chinese (Simplified) Language PO"
  Else
    objStdOut.WriteLine " Creating Chinese (Simplified) Language PO"
  End If
  Call UpdatePO("ZH")
End If
objStdOut.WriteLine " Processing Completed"

' Delete FileSystemObject
Set objFSO = Nothing
Set objStdOut = Nothing

WScript.Quit(0)

Sub UpdatePO(PO)
' Parameters:
'   PO suffix of file in sub-directory "pos" e.g. 'zh' for "pwsafe_zh.po"

Dim WshShell, oExec, objTextFile, strLine
Set WshShell = CreateObject("WScript.Shell")

' Update PO
Set oExec = WshShell.Exec(RESTEXT & " extract " & BASE_DLL & " pos\pwsafe_" & PO & ".po")

Do While oExec.Status = 0
  objStdOut.Write ".."
  WScript.Sleep 100
Loop

' OpenTextFile Method needs a Const value
' ForAppending = 8 ForReading = 1, ForWriting = 2
Const ForAppending = 8

Set objTextFile = objFSO.OpenTextFile("pos\Update_POs.log", ForAppending, True)
objTextFile.WriteLine("Processing: pos\pwsafe_" & PO & ".po")

Do Until oExec.StdOut.AtEndOfStream
  strLine = oExec.StdOut.ReadLine()
  objTextFile.WriteLine(" " & strLine)
Loop

objTextFile.WriteLine("")
objTextFile.Close

Set objTextFile = Nothing
Set oExec = Nothing
Set WshShell = Nothing

objStdOut.WriteLine "   Done"

End Sub

Sub Usage
  MsgBox "This script must be executed by cscript.exe in a Command window, either:" & vbCRLF & _
         "'cscript UpdatePOs.vbs' to update all supported language POs, or" & vbCRLF & _
         "'cscript UpdatePOs.vbs country-code' to update a specific supported language PO" & vbCRLF & _
         "e.g 'cscript UpdatePOs.vbs DE' for German/Germany, or" & vbCRLF & _
         "'cscript UpdatePOs.vbs /Help' for this message", _
         vbInformation, _
         "Help: " & Wscript.ScriptFullName

End Sub