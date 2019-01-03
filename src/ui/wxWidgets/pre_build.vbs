'
' Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' This script sets the version information for the PasswordSafe executable.
' It requires environment variables ProjectDir, SolutionDir & GitDir
' set in UserVariables.vsprops

' For the stdout.WriteLine to work, this Pre-Build Event script
' MUST be executed via cscript command.

' THIS IS THE wxWidgets VERSION

Option Explicit

Const ForReading = 1, ForWriting = 2, ForAppending = 8 
Const TristateUseDefault = -2, TristateTrue = -1, TristateFalse = 0
 
If Instr(1, WScript.FullName, "cscript.exe", vbTextCompare) = 0 then
  MsgBox " Host: " & WScript.FullName & vbCRLF & _
         "This script must be executed by cscript.exe", _
         vbCritical, _
         "Error: " & Wscript.ScriptFullName
  ' return error code to caller
  Wscript.Quit(99)
End If

Dim stdout, stdFSO
Set stdFSO = CreateObject("Scripting.FileSystemObject")
Set stdout = stdFSO.GetStandardStream(1)

Dim objShell, objFSO, cmd, rc
Set objShell = WScript.CreateObject("WScript.Shell")
Set objFSO = CreateObject("Scripting.FileSystemObject")

' Update Git revision info
Dim strGit, strSolutionDir, strProjectDir, strLanguage, strGitPGM
Dim strVersionWX, strVersionIn, strVersionHeader
Dim strLine, strGitRev

strGit = objShell.ExpandEnvironmentStrings("%GitDir%")
strProjectDir = objShell.ExpandEnvironmentStrings("%ProjectDir%")
strSolutionDir = objShell.ExpandEnvironmentStrings("%SolutionDir%")

' Remove double quotes
strGit = Replace(strGit, Chr(34), "", 1, -1, vbTextCompare)
strProjectDir = Replace(strProjectDir, Chr(34), "", 1, -1, vbTextCompare)

' Ensure ends with a back slash
If Right(strGit, 1) <> "\" Then
  strGit = strGit & "\"
End If
If Right(strProjectDir, 1) <> "\" Then
  strProjectDir = strProjectDir & "\"
End If
If Right(strSolutionDir, 1) <> "\" Then
  strSolutionDir = strSolutionDir & "\"
End If

strGitPGM = strGit + "bin\git.exe"
strVersionIn = strProjectDir + "version.in"
strVersionWX = strSolutionDir + "version.wx"
strVersionHeader = strProjectDir + "version.h"

stdout.WriteLine " "
If Not objFSO.FileExists(strVersionIn) Then
  stdout.WriteLine " *** Can't find " & strVersionIn & vbCRLF & _
         " *** Please check source tree"
  WScript.Quit(98)
End If

If Not objFSO.FileExists(strVersionWX) Then
  stdout.WriteLine " *** Can't find " & strVersionWX & vbCRLF & _
         " *** Please check source tree"
  WScript.Quit(96)
End If

If Not objFSO.FileExists(strGitPGM) Then
  stdout.WriteLine " *** Can't find git.exe" & vbCRLF & _
         " *** Please install it or create version.h from version.in manually"
  If Not objFSO.FileExists(strVersionHeader) Then
    MsgBox " *** Windows UI build will fail - can't find file: version.h"
  End If
  rc = 97
Else
  cmd = Chr(34) & strGitPGM  & Chr(34) & " describe --all --always --dirty=+ --long"
  stdout.WriteLine "  Executing: " & cmd

  Dim objWshScriptExec, objStdOut

  Set objShell = CreateObject("WScript.Shell")
  Set objWshScriptExec = objShell.Exec(cmd)
  Set objStdOut = objWshScriptExec.StdOut

  Do While objWshScriptExec.Status = 0
    WScript.Sleep 100
  Loop

  While Not objStdOut.AtEndOfStream
    strLine = objStdOut.ReadLine
    stdout.WriteLine "  " & strLine
  Wend
  strGitRev = strLine
  rc = objWshScriptExec.ExitCode
  stdout.WriteLine "  git ended with return code: " & rc

  ' Don't need these any more
  Set objWshScriptExec = Nothing
  Set objStdOut = Nothing

  If rc <> 0 Then
    ' Tidy up objects before exiting
    Set objShell = Nothing
    Set objFSO = Nothing
    Set stdout = Nothing
    Set stdFSO = Nothing
    WScript.Quit(rc)
  End If
End If
stdout.WriteLine " "

' If strGitRev is of the form heads/master-0-g5f69087, drop everything
' to the left of the rightmost g. Otherwise, this is a branch/WIP, leave full
' info
If InStr(strGitRev, "heads/master-0-") <> 0 Then
  strGitRev = Replace(strGitRev, "heads/master-0-", "")
End If

stdout.WriteLine "strGitRev=" & strGitRev & vbCRLF

' Version variables
Dim strMajor, strMinor, strRevision, strSpecialBuild

' Set defaults in case not in version.wx file
strMajor = "0"
strMinor = "0"
strRevision = "0"
strSpecialBuild = ""

' Get version variable
Call GetVersionInfo

' Create version.h from template and variables
Call CreateVersionFile

' Tidy up objects before exiting
Set objShell = Nothing
Set objFSO = Nothing
Set stdout = Nothing
Set stdFSO = Nothing

WScript.Quit(rc)

' Subroutines
Sub GetVersionInfo

' Get version information from files
Dim objVerWXFile

Set objVerWXFile = objFSO.OpenTextFile(strVersionWX, ForReading)

Do While Not objVerWXFile.AtEndOfStream
  Dim arrStrings, numStrings

  strLine = objVerWXFile.ReadLine()
  arrStrings = Split(strLine)
  numStrings = UBound(arrStrings)

  If numStrings >= 2 Then
    If arrStrings(0) = "VER_MAJOR" Then
      strMajor = arrStrings(2)
    ElseIf arrStrings(0) = "VER_MINOR" Then
      strMinor = arrStrings(2)
    ElseIf arrStrings(0) = "VER_REV" Then
      strRevision = arrStrings(2)
    ElseIf arrStrings(0) = "VER_SPECIAL" Then
      Dim i
      strSpecialBuild = arrStrings(2)    
      for i = 3 To numStrings
        strSpecialBuild = strSpecialBuild + " " + arrStrings(i)
      Next
    End If
  End If
Loop

' Close file
objVerWXFile.Close

' Tidy up
Set objVerWXFile = Nothing

End Sub

Sub CreateVersionFile

' Create version.h file from template and variables
Dim result
Dim objVerInFile, objVerHFile

' Read version.in, write version.h, substitute @pwsafe_....@ variables
Set objVerInFile = objFSO.OpenTextFile(strVersionIn, ForReading)
Set objVerHFile = objFSO.OpenTextFile(strVersionHeader, ForWriting, True, TristateFalse)

Do While Not objVerInFile.AtEndOfStream
  strLine = objVerInFile.ReadLine()
  result = InStr(strLine, "@pwsafe_VERSION_MAJOR@")
  If result <> 0 Then
    strLine = Replace(strLine, "@pwsafe_VERSION_MAJOR@", strMajor)
  End If
  result = InStr(strLine, "@pwsafe_VERSION_MINOR@")
  If result <> 0 Then
    strLine = Replace(strLine, "@pwsafe_VERSION_MINOR@", strMinor)
  End If
  result = InStr(strLine, "@pwsafe_REVISION@")
  If result <> 0 Then
    strLine = Replace(strLine, "@pwsafe_REVISION@", strRevision)
  End If
  result = InStr(strLine, "@pwsafe_SPECIALBUILD@")
  If result <> 0 Then
    strLine = Replace(strLine, "@pwsafe_SPECIALBUILD@", strSpecialBuild)
  End If
  result = InStr(strLine, "@pwsafe_VERSTRING@")
  If result <> 0 Then
    strLine = Replace(strLine, "@pwsafe_VERSTRING@", strGitRev)
  End If
  objVerHFile.WriteLine(strLine)
Loop

' Close files
objVerInFile.Close
objVerHFile.Close

' Tidy up
Set objVerInFile = Nothing
Set objVerHFile = Nothing

End Sub
