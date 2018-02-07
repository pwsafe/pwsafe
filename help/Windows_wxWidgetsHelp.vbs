'
' Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' This script creates the Help zip files required by wxWidgets for WINDOWS

' For the stdout.WriteLine to work, this script
' MUST be executed via cscript command.

Option Explicit

Const ForReading = 1, ForWriting = 2, ForAppending = 8 
Const TristateUseDefault = -2, TristateTrue = -1, TristateFalse = 0

If Instr(1, WScript.FullName, "cscript.exe", vbTextCompare) = 0 then
  MsgBox " Host: " & WScript.FullName & vbCRLF & _
         "This script must be executed by cscript.exe", _
         vbCritical, _
         "Error: " & WScript.ScriptFullName
  ' return error code to caller
  WScript.Quit(99)
End If

Dim stdFSO, stdout, objShell, objFSO, objZip, objApp, objArgs
Dim strLanguage, strSourceDir, strTargetFile, strCurDir, strLangArray()

Set stdFSO = CreateObject("Scripting.FileSystemObject")
Set stdout = stdFSO.GetStandardStream(1)

Set objShell = WScript.CreateObject("WScript.Shell")
strCurDir = objShell.CurrentDirectory

Set objApp = CreateObject("Shell.Application")

' Ensure trailing slash
If Right(strCurDir, 1) <> "\" Then
  strCurDir = strCurDir & "\"
End If

' If no argument suplied - create all help files otherwise create just the one
' The argument is the 2 character language identifier e.g. EN, DE, FR etc.
Set objArgs  = WScript.Arguments
if objArgs.Count <> 1 Then
  ' Do all - go find all the ones here
  Call GetHelpFolderList(strCurDir, strLangArray)
Else
  ' Do just the specified one
  ReDim strLangArray(0)
  strLangArray(0) = objArgs(0)
End IF

For Each strLanguage in strLangArray
  If strLanguage = "EN" Then
    strSourceDir = strCurDir & "default\"
  Else
    strSourceDir = strCurDir & "pwsafe" & strLanguage
  End If
  
  strTargetFile = strCurDir & "help" & strLanguage & ".zip"
      
  Set objFSO = CreateObject("Scripting.FileSystemObject")

  ' Delete current zip file
  if objFSO.FileExists(strTargetFile) Then
    objFSO.DeleteFile(strTargetFile)
  End If

  ' Create zip file (minimum requirement is the header)
  Set objZip = objFSO.OpenTextFile(strTargetFile, ForWriting, vbTrue, TristateFalse)
  objZip.Write "PK" & Chr(5) & Chr(6) & String(18, Chr(0))
  objZip.Close

  ' NOTE CopyHere is ASYNCHRONOUS.  Have to make sure current copy is complete
  ' before attempting the next

  Dim objSourceFolder, objSourceFolderItems
  Set objSourceFolder = objApp.NameSpace(strSourceDir)
  Set objSourceFolderItems = objSourceFolder.Items

  ' Copy files into zip file
  objApp.NameSpace(strTargetFile).CopyHere objSourceFolderItems, 132 ' 4 + 128

  ' Delay until all copied - zip target file has all input source files
  Do Until objApp.NameSpace(strTargetFile).Items.Count = objSourceFolderItems.Count
    WScript.Sleep 100
  Loop

  ' Tidy up this time around
  Set objZip = Nothing
  Set objSourceFolderItems = Nothing
  Set objSourceFolder = Nothing
  
  ' Tell user/developer completed
  stdout.WriteLine "Help file " & """help" & strLanguage & ".zip"" created"
Next

' Cleanup
Set objShell = Nothing
Set objApp = Nothing
Set objFSO = Nothing
Set stdout = Nothing
Set stdFSO = Nothing

WScript.Quit(0)

Sub GetHelpFolderList(strCurDir, strLangArray)
  ' Get all the folders with name "pwasfeXX" in the current directory.
  ' The 'XX' is the language (e.g. DE) except that the directory "default"
  ' corresponds to "EN"
  Dim objFSO, objCurrentFolder, objFolder, objFolderCollection, strName, i
  
  Set objFSO = CreateObject("Scripting.FileSystemObject")
  Set objCurrentFolder = objFSO.GetFolder(strCurDir)
  Set objFolderCollection = objCurrentFolder.SubFolders
  
  ' Always add the default of English
  i = 0
  ReDim strLangArray(0)
  strLangArray(0) = "EN"
  
  ' Now add the others if of the form "pwsafeXX"
  For Each objFolder in objFolderCollection
    strName = objFolder.name 
    if Len(strName) = 8 AND Left(strName, 6) = "pwsafe" Then
     i = i + 1
     ReDim Preserve strLangArray(i)
     strLangArray(i) = Right(strName, 2)
    End If
  Next
  Set objFSO = Nothing
End Sub
