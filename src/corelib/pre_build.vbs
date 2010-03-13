'
' Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' This section creates C++ source and header files from corelib.rc2
' for use by non-MFC configurations

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

Dim strProjectDir, strCPPFile, strHFile, bCPPExists, bHExists
strProjectDir = objShell.ExpandEnvironmentStrings("%ProjectDir%")

' Remove double quotes
strProjectDir = Replace(strProjectDir, Chr(34), "", 1, -1, vbTextCompare)

' Ensure ends with a back slash
If Right(strProjectDir, 1) <> "\" Then
  strProjectDir = strProjectDir & "\"
End If

strCPPFile = strProjectDir + "corelib_st.cpp"
strHFile = strProjectDir + "corelib_st.h"

' Check if any already exist
bCPPExists = objFSO.FileExists(strCPPFile)
bHExists = objFSO.FileExists(strHFile)

' Create command string
cmd = "perl -w " & "..\..\Misc\rc2cpp.pl" & " " & "corelib.rc2"

' If prefix script by "Perl", may get return code: 
'    80070002 - System cannot find file (i.e. "Perl") if not in the path, or
' If just execute the script, may get return code:
'    80070483 - No program associated with the '.pl' extension
Dim objWshScriptExec, objStdOut, strLine

rc = 0
stdout.WriteLine " "
stdout.WriteLine " Executing script: " & cmd
  
On Error Resume Next
Set objWshScriptExec = objShell.Exec(cmd)
If Err.Number <> 0 Then
  rc = Err.Number
End If
Err.Clear
On Error Goto 0

If rc = 0 Then
  Set objStdOut = objWshScriptExec.StdOut

  Do While objWshScriptExec.Status = 0
     WScript.Sleep 100
  Loop

  While Not objStdOut.AtEndOfStream
    strLine = objStdOut.ReadLine
    stdout.WriteLine "  " & strLine
  Wend

  stdout.WriteLine " Script ended with return code: " & objWshScriptExec.ExitCode
Else
  If Hex(rc) = "80070483" Or Hex(rc) = "80070002" Then
    stdout.WriteLine " *** Can't find Perl on your computer to run conversion script rc2cpp.pl" & vbCRLF & _
           " *** Please install it or create corelib_st.cpp & corelib_st.h from corelib.rc2 manually"
    If bCPPExists Or bHExists Then
      stdout.WriteLine " *** Any existing copies of corelib_st.cpp & corelib_st.h may be out of date."
    End If
  End If
  rc = 99
End If

stdout.WriteLine " "

Set objWshScriptExec = Nothing
Set objStdOut = Nothing
Set objShell = Nothing
Set objFSO = Nothing
Set stdout = Nothing
Set stdFSO = Nothing

WScript.Quit(rc)
