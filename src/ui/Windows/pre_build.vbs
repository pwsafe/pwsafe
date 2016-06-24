'
' Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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

' THIS IS THE MFC VERSION

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
Dim strGit, strSolutionDir, strProjectDir, strGitPGM
Dim strVersionMFC, strVersionIn, strVersionHeader
Dim objVerMFCFile, objVerInFile, objVerHFile
Dim strLine, strGitRev

strGit = objShell.ExpandEnvironmentStrings("%GitDir%")
strProjectDir = objShell.ExpandEnvironmentStrings("%ProjectDir%")

' To prevent changing configure-14.vbs and so UserVariables-14.props
' to retrieve value of $(SolutionDir) macro into an environmental variable
Dim fso, strCurrentDirectory
Set fso = CreateObject("Scripting.FileSystemObject")
strCurrentDirectory = fso.GetAbsolutePathName(".")
If Right(strCurrentDirectory, 1) <> "\" Then
  strCurrentDirectory = strCurrentDirectory & "\"
End If

' Remove "src/ui/Windows/"
strSolutionDir = Left(strCurrentDirectory, Len(strCurrentDirectory) - Len("src/ui/Windows/"))
Set fso = Nothing

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
strVersionMFC = strSolutionDir + "version.mfc"
strVersionHeader = strProjectDir + "version.h"

stdout.WriteLine " "
If Not objFSO.FileExists(strVersionIn) Then
  stdout.WriteLine " *** Can't find " & strVersionIn & vbCRLF & _
         " *** Please check source tree"
  WScript.Quit(98)
End If

If Not objFSO.FileExists(strVersionMFC) Then
  stdout.WriteLine " *** Can't find " & strVersionMFC & vbCRLF & _
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

Dim result

result = InStr(strGitRev, "heads/master-0-")

If result <> 0 Then
  strGitRev = Replace(strGitRev, "heads/master-0-", "")
End If

stdout.WriteLine "strGitRev=" & strGitRev & vbCRLF

' Read version.mfc to get 
Dim strMajor, strMinor, strRevision, strSpecialBuild
Set objVerMFCFile = objFSO.OpenTextFile(strVersionMFC, ForReading)

' Set defaults in case not in version.mfc file
strMajor = "0"
strMinor = "0"
strRevision = "0"
strSpecialBuild = ""

Do While Not objVerMFCFile.AtEndOfStream
  Dim arrStrings, numStrings, i
  strLine = objVerMFCFile.ReadLine()
  result = InStr(strLine, "VER_MAJOR")
  If result <> 0 AND Left(strLine, 1) <> "#" Then
    arrStrings = Split(strLine)
    strMajor = arrStrings(2)
  End If
  result = InStr(strLine, "VER_MINOR")
  If result <> 0 AND Left(strLine, 1) <> "#" Then
    arrStrings = Split(strLine)
    strMinor = arrStrings(2)
  End If
  result = InStr(strLine, "VER_REV")
  If result <> 0 AND Left(strLine, 1) <> "#" Then
    arrStrings = Split(strLine)
    strRevision = arrStrings(2)
  End If
  result = InStr(strLine, "VER_SPECIAL")
  If result <> 0 AND Left(strLine, 1) <> "#" Then
    arrStrings = Split(strLine)
    numStrings = UBound(arrStrings)
    strSpecialBuild = arrStrings(2)    
    for i = 3 To numStrings
      strSpecialBuild = strSpecialBuild + " " + arrStrings(i)
    Next
  End If
Loop

objVerMFCFile.Close

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

objVerInFile.Close
objVerHFile.Close

' Tidy up objects before exiting
Set objVerMFCFile = Nothing
Set objVerInFile = Nothing
Set objVerHFile = Nothing

Set objShell = Nothing
Set objFSO = Nothing
Set stdout = Nothing
Set stdFSO = Nothing

WScript.Quit(rc)
