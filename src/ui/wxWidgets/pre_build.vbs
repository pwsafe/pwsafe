'
' Copyright (c) 2003-2013 Rony Shapiro <ronys@users.sourceforge.net>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' This section does "Update Revision Number in Resources"
' Requires environment variables ProjectDir & GitDir
' set in UserVariables.vsprops

' For the stdout.WriteLine to work, this Pre-Build Event script
' MUST be executed via cscript command.

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
Dim strGit, strProjectDir, strGitPGM, strVersionIn, strVersionHeader
Dim objVerInFile, objVerHFile
Dim strLine, strGitRev

strGit = objShell.ExpandEnvironmentStrings("%GitDir%")
strProjectDir = objShell.ExpandEnvironmentStrings("%ProjectDir%")

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

strGitPGM = strGit + "bin\git.exe"
strVersionIn = strProjectDir + "version.in"
strVersionHeader = strProjectDir + "version.h"

stdout.WriteLine " "
If Not objFSO.FileExists(strVersionIn) Then
  stdout.WriteLine " *** Can't find " & strVersionIn & vbCRLF & _
         " *** Please check source tree"
  WScript.Quit(98)
End If

If Not objFSO.FileExists(strGitPGM) Then
  stdout.WriteLine " *** Can't find git.exe" & vbCRLF & _
         " *** Please install it or create version.h from version.in manually"
  If Not objFSO.FileExists(strVersionHeader) Then
    MsgBox " *** Windows UI build will fail - can't find file: version.h"
  End If
  rc = 99
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

Dim result

result = InStr(strGitRev, "heads/master-0-")

If result <> 0 Then
  strGitRev = Replace(strGitRev, "heads/master-0-", "")
End If

stdout.WriteLine "strGitRev=" & strGitRev & vbCRLF

' Read version.in, write version.h, substitute GITREV with strGitRev
Set objVerInFile = objFSO.OpenTextFile(strVersionIn, ForReading)
Set objVerHFile = objFSO.OpenTextFile(strVersionHeader, ForWriting, True, TristateFalse)

Do While Not objVerInFile.AtEndOfStream
  strLine = objVerInFile.ReadLine()
  result = InStr(strLine, "GITREV")
  If result <> 0 Then
    strLine = Replace(strLine, "GITREV", strGitRev)
  End If
  objVerHFile.WriteLine(strLine)
Loop

objVerInFile.Close
objVerHFile.Close

' Tidy up objects before exiting
Set objVerInFile = Nothing
Set objVerHFile = Nothing

Set objShell = Nothing
Set objFSO = Nothing
Set stdout = Nothing
Set stdFSO = Nothing

WScript.Quit(rc)
