'Update the Resource Only DLLs for each language we support
'
' This script uses the 32-bit languageDLL to update PO files.
' A 64-bit version of this action is not required, because the 
' resources in the PO file and languageDLL are not architecture
' dependent.
'

Option Explicit

If Instr(1, WScript.FullName, "cscript.exe", vbTextCompare) = 0 then
    MsgBox "This script must be executed by cscript.exe" & vbCRLF & _
           "e.g. cscript UpdatePOs.vbs", _
           vbCritical, _
           "Error: " & Wscript.ScriptFullName
    ' return error code to caller
    Wscript.Quit(99)
End If

IncludeFile("Translations.vbs")

' Load up the list of active translations
Dim TransList
TransList = BuildTranslationList()

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
 If Arg1 = "/HELP" Or Arg1 = "/H" or Arg1 = "/?" Then
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

If DO_ALL = True Then
  Dim t
  For Each t in TransList
    If (objFSO.FileExists("pos\" & t.fileName)) Then
      objStdOut.WriteLine " Updating " & t.FileName
    Else
      objStdOut.WriteLine " Creating " & t.FileName
    End If
    UpdatePO t.FileName
  Next
Else
  ' Do one country
  Wscript.Echo "Updating PO file for country", DO_COUNTRY
  Dim OneTranslation
  Set OneTranslation = FindCountry(TransList, DO_COUNTRY)
  If Not OneTranslation Is Nothing Then
    If (objFSO.FileExists("pos\" & OneTranslation.FileName)) Then
      objStdOut.WriteLine " Updating " & OneTranslation.FileName
    Else
      objStdOut.WriteLine " Creating " & OneTranslation.FileName
    End If
    UpdatePO OneTranslation.FileName
  Else
    WScript.Echo "Error:", DO_COUNTRY, "is not a valid language"
    WScript.Quit(99)
  End If
End If

objStdOut.WriteLine " Processing Completed"

' Delete FileSystemObject
Set objFSO = Nothing
Set objStdOut = Nothing

WScript.Quit(0)

Sub UpdatePO(PO)
  ' Parameters:
  '   PO translation file in sub-directory "pos" e.g. "pwsafe_zh.po"

  Dim WshShell, oExec, objTextFile, strLine
  Set WshShell = CreateObject("WScript.Shell")

  ' Update PO
  Set oExec = WshShell.Exec(RESTEXT & " extract " & BASE_DLL & " pos\" & PO)

  objStdOut.Write " "
  Do While oExec.Status = 0
    objStdOut.Write ".."
    WScript.Sleep 100
  Loop

  ' OpenTextFile Method needs a Const value
  ' ForAppending = 8 ForReading = 1, ForWriting = 2
  Const ForAppending = 8

  Set objTextFile = objFSO.OpenTextFile("pos\Update_POs.log", ForAppending, True)
  objTextFile.WriteLine("Processing: pos\" & PO)

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
  MsgBox "This script must be executed by cscript.exe in a Command window:" & vbCRLF & _
         " " & vbCRLF & _
         "cscript UpdatePOs.vbs [CC | /help | /?]" & vbCRLF & _
         "    CC - two letter country code" & vbCRLF & _
         "    /help, /? - show this help dialog" & vbCRLF & _
         "    Default - Update all languages" & vbCRLF & _
         " " & vbCRLF & _
         "Available languages are defined in Translations.txt", _
         vbInformation, _
         "Help: " & Wscript.ScriptName

End Sub

' Loads a VBScript file (like a C++ include)
Sub IncludeFile (fspec)
  Dim fileSys, file, fileData
  set fileSys = createObject ("Scripting.FileSystemObject")
  set file = fileSys.openTextFile (fspec)
  fileData = ""
  While Not file.AtEndOfStream
    fileData = fileData & file.readAll ()
  Wend
  file.close
  ExecuteGlobal fileData
  set file = nothing
  set fileSys = nothing
End Sub
