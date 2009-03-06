'
' Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' Copy pws_at(_D).dll from Debug/Release 'bin' libraries to the
' configuration's own 'bin' library

' For the stdout.WriteLine to work, this Post-Build Event script
' MUST be executed via cscript command.

If Instr(1, WScript.FullName, "cscript.exe", vbTextCompare) = 0 then
    MsgBox " Host: " & WScript.FullName & vbCRLF & _
           "This script must be executed by cscript.exe", _
           vbCritical, _
           "Error: " & Wscript.ScriptFullName
    ' return error code to caller
    Wscript.Quit(99)
End If

Dim oShell, objFSO
Dim strConfig, strOutDir, strInfile, strOutfile
Dim objInfile, objOutfile, strDLL
Dim rc, bOutputFileExists

rc = 0
bOutputFileExists = False

' Get variables set up via 'configure.vbs' and then 'UserVariables.vsprops'
Set oShell = CreateObject("WScript.Shell")
strConfig = oShell.ExpandEnvironmentStrings("%ConfigurationName%")
strOutDir = oShell.ExpandEnvironmentStrings("%OutDir%")

' If either emtpy - prompt the user to re-run 'configure.vbs'
If strConfig = "%ConfigurationName%" Then
  EndScript 1
End If

If strOutDir = "%OutDir%" Then
  EndScript 2
End If

Set objFSO = CreateObject("Scripting.FileSystemObject")

' Check that the input DLL exists for this configuration
strConfigLC = LCase(strConfig)

Select Case strConfigLC
  Case "debug", "release"
    ' No action required
    rc = -1
  Case "debuge", "debugm", "debugx"
    strInDir = Replace(strOutDir, strConfig , "Debug")
    strDLL = "pws_at_D.dll"
    strInfile = strInDir & "\" & strDLL
    If Not objFSO.FileExists(strInfile) Then
    	rc = 3
    Else
      Set objInfile = objFSO.GetFile(strInfile)
    End If
    strOutfile = strOutDir & "\" & strDLL
  Case "demo", "releasee", "releasem", "releasex"
    strInDir = Replace(strOutDir, strConfig , "Release")
    strDLL = "pws_at.dll"
    strInfile = strInDir & "\" & strDLL
    If Not objFSO.FileExists(strInfile) Then
    	rc = 4
    Else
      Set objInfile = objFSO.GetFile(strInfile)
    End If
    strOutfile = strOutDir & "\" & strDLL
  Case Else
    rc = 5
End Select

If rc <> 0 Then
  EndScript rc
End If

' Check if the output DLL exists for this configuration
If objFSO.FileExists(strOutfile) Then
  Set objOutfile = objFSO.GetFile(strOutfile)
  bOutputFileExists = True
End If

Const OverwriteExisting = TRUE

If bOutputFileExists = True Then
  ' If the output DLL already exists only copy if input is newer 
  If objInfile.DateLastModified > objOutfile.DateLastModified Then
     objFSO.CopyFile strInfile, strOutfile, OverwriteExisting
  Else
    rc = 6
  End If
Else
  ' Output DLL doesn't exist, copy file
  objFSO.CopyFile  strInfile, strOutfile, OverwriteExisting
End If

EndScript rc

' Next statement not really needed as EndScript tidies up and exits
' but just in case.....
WScript.Quit(ExitCode)

Sub EndScript(iCode)
' As there is no 'goto' in vbscript, this allows the script to be
' terminated just by calling this subroutine with an exit code

Dim ExitCode
Dim strErrorMsg

ExitCode = iCode

' Get stdout NOTE: Host engine MUST be cscript and NOT wscript
Set stdFSO = CreateObject("Scripting.FileSystemObject")
Set stdout = stdFSO.GetStandardStream(1)

Select Case iCode
  Case -1
    stdout.WriteLine " "
    stdout.WriteLine " >>> No Post-Build action required."
    stdout.WriteLine " "
    ExitCode = 0
  Case 0
    stdout.WriteLine " "
    stdout.WriteLine " >>> DLL: '" & strDLL & "' copied successfully."
    stdout.WriteLine " "
  Case 1
    strErrorMsg = "'Configuration' variable is not set." & vbCRLF & _
                  "Please close this solution and re-run 'configure.vbs'."
  Case 2
    strErrorMsg = "'Output directory' variable is not set." & vbCRLF & _
                  "Please close this solution and re-run 'configure.vbs'."
  Case 3
    strErrorMsg = "Debug pws_at_D.dll does not exist." & vbCRLF & _
                  "Please ensure that the Debug build completed without error."
  Case 4
    strErrorMsg = "Release pws_at.dll does not exist." & vbCRLF & _
                  "Please ensure that the Release build completed without error."
  Case 5
    strErrorMsg = "Unknown configuration: '" & strConfig & "'. Unable to continue."
  Case 6
    stdout.WriteLine " "
    stdout.WriteLine " >>> DLL: '" & strDLL & "' not copied, already up to date."
    stdout.WriteLine " "
    ' Reset error code
    ExitCode = 0
End Select

if (ExitCode <> 0) Then
    MsgBox strErrorMsg, vbCritical, "Error: " & WScript.ScriptName
End If

' Tidy up objects
Set stdFSO = Nothing
Set objInfile = Nothing
Set objOutfile = Nothing
Set oShell = Nothing
Set objFSO = Nothing

' Byee
WScript.Quit(ExitCode)

End Sub
