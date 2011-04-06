'
' Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' This section does "Update Revision Number in Resources"
' Requires environment variables ProjectDir & TortoiseSVNDir
' set in UserVariables.vsprops

' For the stdout.WriteLine to work, this Post-Build Event script
' MUST be executed via cscript command.

Option Explicit

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

' Update SVN revision number
Dim strTSVN, strProjectDir, strTSVNPGM, strVersionHeader
strTSVN = objShell.ExpandEnvironmentStrings("%TortoiseSVNDir%")
strProjectDir = objShell.ExpandEnvironmentStrings("%ProjectDir%")

' Remove double quotes
strTSVN = Replace(strTSVN, Chr(34), "", 1, -1, vbTextCompare)
strProjectDir = Replace(strProjectDir, Chr(34), "", 1, -1, vbTextCompare)

' Ensure ends with a back slash
If Right(strTSVN, 1) <> "\" Then
  strTSVN = strTSVN & "\"
End If
If Right(strProjectDir, 1) <> "\" Then
  strProjectDir = strProjectDir & "\"
End If

strTSVNPGM = strTSVN + "bin\SubWCRev.exe"
strVersionHeader = strProjectDir + "..\version.h"

stdout.WriteLine " "
If Not objFSO.FileExists(strTSVNPGM) Then
  stdout.WriteLine " *** Can't find TortoiseSVN's SubWCRev.exe" & vbCRLF & _
         " *** Please install it or create version.h from version.in manually"
  If Not objFSO.FileExists(strVersionHeader) Then
    MsgBox " *** Windows UI build will fail - can't find file: version.h"
  End if
  rc = 99
Else
  cmd = Chr(34) & strTSVNPGM  & Chr(34) & " ..\..\.. ..\version.in ..\version.h"
  stdout.WriteLine "  Executing: " & cmd

  Dim objWshScriptExec, objStdOut, strLine

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

  stdout.WriteLine "  SubWCRev ended with return code: " & objWshScriptExec.ExitCode
  rc = 0
End if
stdout.WriteLine " "

Set objWshScriptExec = Nothing
Set objStdOut = Nothing
Set objShell = Nothing
Set objFSO = Nothing
Set stdout = Nothing
Set stdFSO = Nothing

WScript.Quit(rc)
