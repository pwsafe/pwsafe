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

If (objFSO.FileExists("pos\Update_POs.log")) Then
  'Delete log file
  objFSO.DeleteFile "pos\Update_POs.log"
End If

Dim objStdOut
Set objStdOut = WScript.StdOut

' Now do them
If (DO_ALL = True Or DO_COUNTRY = "DE") Then
objStdOut.WriteLine " Updating German Language PO"
Call UpdatePO("DE")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "DK") Then
objStdOut.WriteLine " Updating Danish Language PO"
Call UpdatePO("DK")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "ES") Then
objStdOut.WriteLine " Updating Spanish Language PO"
Call UpdatePO("ES")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "FR") Then
objStdOut.WriteLine " Updating French Language PO"
Call UpdatePO("FR")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "IT") Then
objStdOut.WriteLine " Updating Italian Language PO"
Call UpdatePO("IT")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "KR") Then
objStdOut.WriteLine " Updating Korean Language PO"
Call UpdatePO("KR")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "NL") Then
objStdOut.WriteLine " Updating Dutch Language PO"
Call UpdatePO("NL")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "PL") Then
objStdOut.WriteLine " Updating Polish Language PO"
Call UpdatePO("PL")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "RU") Then
objStdOut.WriteLine " Updating Russian Language PO"
Call UpdatePO("RU")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "SV") Then
objStdOut.WriteLine " Updating Swedish Language PO"
Call UpdatePO("SV")
objStdOut.WriteLine "   Done"
End If
If (DO_ALL = True Or DO_COUNTRY = "ZH") Then
objStdOut.WriteLine " Updating Chinese Language PO"
Call UpdatePO("ZH")
objStdOut.WriteLine "   Done"
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