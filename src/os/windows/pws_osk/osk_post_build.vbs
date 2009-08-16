'
' Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' Copy pws_osk(_D).dll from Debug/Release 'bin' libraries to the
' configuration's own 'bin' library

' For the stdout.WriteLine to work, this Post-Build Event script
' MUST be executed via cscript command.

'Option Explicit

If Instr(1, WScript.FullName, "cscript.exe", vbTextCompare) = 0 then
    MsgBox " Host: " & WScript.FullName & vbCRLF & _
           "This script must be executed by cscript.exe", _
           vbCritical, _
           "Error: " & Wscript.ScriptFullName
    ' return error code to caller
    Wscript.Quit(99)
End If

' Copy On-screen Keyboard DLL if required
Call CopyDLL("osk")

' Copy On-screen Keyboard PDB if required
Call CopyPDB("osk")

WScript.Quit(0)

' Subroutines

Sub CopyDLL(strWhichDLL)

Dim oShell, objFSO
Dim strConfig, strConfigLC
Dim strInDir, strOutDir, strInfile, strOutfile
Dim strDLL
Dim rc

rc = 0
bOutputFileExists = False

' Get variables set up via 'configure.vbs' and then 'UserVariables.vsprops'
Set oShell = CreateObject("WScript.Shell")
strConfig = oShell.ExpandEnvironmentStrings("%ConfigurationName%")
strOutDir = oShell.ExpandEnvironmentStrings("%OutDir%")

' If either emtpy - prompt the user to re-run 'configure.vbs'
If strConfig = "%ConfigurationName%" Then
  Call EndScript(1, "")
End If

If strOutDir = "%OutDir%" Then
  Call EndScript(2, "")
End If

Set objFSO = CreateObject("Scripting.FileSystemObject")

' Check that the input DLL exists for this configuration
strConfigLC = LCase(strConfig)

Select Case strConfigLC
  Case "debug"
    strInDir = Replace(strOutDir, strConfig , "Debug")
    strDLL = "pws_" & strWhichDLL & "_D.dll"
    strInfile = strInDir & "\" & strDLL
    If Not objFSO.FileExists(strInfile) Then
      rc = 3
    End If
    strOutfile = strOutDir & "\" & strDLL
  Case "release"
    strInDir = Replace(strOutDir, strConfig , "Release")
    strDLL = "pws_" & strWhichDLL & ".dll"
    strInfile = strInDir & "\" & strDLL
    If Not objFSO.FileExists(strInfile) Then
      rc = 4
    End If
    strOutfile = strOutDir & "\" & strDLL
  Case Else
    rc = 5
End Select

If rc <> 0 Then
  Call EndScript(rc, strDLL)
End If

Select Case strConfigLC
  Case "debug"
    strOutDir = Replace(strOutDir, "\Debug" , "\DebugE")
    If objFSO.FolderExists(strOutDir) Then
      strOutfile = strOutDir & "\" & strDLL
      Call CopyFile(strDLL, strInfile, strOutfile, "DebugE")
    End If
    strOutDir = Replace(strOutDir, "\DebugE" , "\DebugM")
    If objFSO.FolderExists(strOutDir) Then
      strOutfile = strOutDir & "\" & strDLL
      Call CopyFile(strDLL, strInfile, strOutfile, "DebugM")
    End If
    strOutDir = Replace(strOutDir, "\DebugM" , "\DebugX")
    If objFSO.FolderExists(strOutDir) Then
      strOutfile = strOutDir & "\" & strDLL
      Call CopyFile(strDLL, strInfile, strOutfile, "DebugX")
    End If
  Case "release"
    strOutDir = Replace(strOutDir, "\Release" , "\Demo")
    If objFSO.FolderExists(strOutDir) Then
      strOutfile = strOutDir & "\" & strDLL
      Call CopyFile(strDLL, strInfile, strOutfile, "Demo")
    End If
    strOutDir = Replace(strOutDir, "\Demo" , "\ReleaseE")
    If objFSO.FolderExists(strOutDir) Then
      strOutfile = strOutDir & "\" & strDLL
      Call CopyFile(strDLL, strInfile, strOutfile, "ReleaseE")
    End If
    strOutDir = Replace(strOutDir, "\ReleaseE" , "\ReleaseM")
    If objFSO.FolderExists(strOutDir) Then
      strOutfile = strOutDir & "\" & strDLL
      Call CopyFile(strDLL, strInfile, strOutfile, "ReleaseM")
    End If
    strOutDir = Replace(strOutDir, "\ReleaseM" , "\ReleaseX")
    If objFSO.FolderExists(strOutDir) Then
      strOutfile = strOutDir & "\" & strDLL
      Call CopyFile(strDLL, strInfile, strOutfile, "ReleaseX")
    End If
  Case Else
    rc = 5
End Select

' Tidy up objects
Set objFSO = Nothing
Set oShell = Nothing

If rc <> 0 Then
  Call EndScript(rc, strDLL)
End If

End Sub


Sub CopyPDB(strWhichPDB)

Dim oShell, objFSO
Dim strConfig, strConfigLC
Dim strInDir, strOutDir, strInfile, strOutfile
Dim strPDB
Dim rc

rc = 0
bOutputFileExists = False

' Get variables set up via 'configure.vbs' and then 'UserVariables.vsprops'
Set oShell = CreateObject("WScript.Shell")
strConfig = oShell.ExpandEnvironmentStrings("%ConfigurationName%")
strOutDir = oShell.ExpandEnvironmentStrings("%OutDir%")

' If either emtpy - prompt the user to re-run 'configure.vbs'
If strConfig = "%ConfigurationName%" Then
  Call EndScript(1, "")
End If

If strOutDir = "%OutDir%" Then
  Call EndScript(2, "")
End If

Set objFSO = CreateObject("Scripting.FileSystemObject")

' Check that the input DLL exists for this configuration
strConfigLC = LCase(strConfig)

Select Case strConfigLC
  Case "debug"
    strInDir = Replace(strOutDir, strConfig , "Debug")
    strPDB = "pws_" & strWhichPDB & "_D.pdb"
    strInfile = strInDir & "\" & strPDB
    If Not objFSO.FileExists(strInfile) Then
      rc = 3
    End If
    strOutfile = strOutDir & "\" & strPDB
  Case "release"
    strInDir = Replace(strOutDir, strConfig , "Release")
    strPDB = "pws_" & strWhichPDB & ".pdb"
    strInfile = strInDir & "\" & strPDB
    If Not objFSO.FileExists(strInfile) Then
      rc = 4
    End If
    strOutfile = strOutDir & "\" & strPDB
  Case Else
    rc = 5
End Select

If rc <> 0 Then
  Call EndScript(rc, strDLL)
End If

Select Case strConfigLC
  Case "debug"
    strOutDir = Replace(strOutDir, "\Debug" , "\DebugM")
    If objFSO.FolderExists(strOutDir) Then
      strOutfile = strOutDir & "\" & strPDB
      Call CopyFile(strPDB, strInfile, strOutfile, "DebugM")
    End If
  Case "release"
    strOutDir = Replace(strOutDir, "\Release" , "\ReleaseM")
    If objFSO.FolderExists(strOutDir) Then
      strOutfile = strOutDir & "\" & strPDB
      Call CopyFile(strPDB, strInfile, strOutfile, "ReleaseM")
    End If
  Case Else
    rc = 5
End Select

' Tidy up objects
Set objFSO = Nothing
Set oShell = Nothing

If rc <> 0 Then
  Call EndScript(rc, strDLL)
End If

End Sub

Sub CopyFile(strFileName, strInfile, strOutfile, strOutDir)

Dim rc, bOutputFileExists, strType, pos

pos = InStr(strFileName,".pdb")

if (pos = 0) then
  strType = " >>> DLL: '"
else
  strType = " >>> PDB: '"
end if

Set objFSO_CF = CreateObject("Scripting.FileSystemObject")
Set objInfile = objFSO_CF.GetFile(strInfile)

' Get stdout NOTE: Host engine MUST be cscript and NOT wscript
Set stdFSO_CF = CreateObject("Scripting.FileSystemObject")
Set stdout_CF = stdFSO_CF.GetStandardStream(1)

' Check if the output DLL exists for this configuration
If objFSO_CF.FileExists(strOutfile) Then
  Set objOutfile = objFSO_CF.GetFile(strOutfile)
  bOutputFileExists = True
End If

Const OverwriteExisting = TRUE

If bOutputFileExists = True Then
  ' If the output DLL already exists only copy if input is newer 
  If objInfile.DateLastModified > objOutfile.DateLastModified Then
     objFSO_CF.CopyFile strInfile, strOutfile, OverwriteExisting
     stdout_CF.WriteLine strType & strFileName & "' copied successfully to directory: " & strOutDir
  Else
    stdout_CF.WriteLine strType & strFileName & "' not copied to directory: " & strOutDir & ", already up to date."
  End If
Else
  ' Output DLL doesn't exist, copy file
  objFSO_CF.CopyFile  strInfile, strOutfile, OverwriteExisting
End If

' Tidy up objects
Set objInfile = Nothing
Set objOutfile = Nothing
Set objFSO_CF = Nothing
Set stdFSO_CF = Nothing

End Sub

Sub EndScript(iCode, strFileName)
' As there is no 'goto' in vbscript, this allows the script to be
' terminated just by calling this subroutine with an exit code

Dim stdFSO, stdout
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
    stdout.WriteLine strType & strFileName & "' copied successfully."
    stdout.WriteLine " "
  Case 1
    strErrorMsg = "'Configuration' variable is not set." & vbCRLF & _
                  "Please close this solution and re-run 'configure.vbs'."
  Case 2
    strErrorMsg = "'Output directory' variable is not set." & vbCRLF & _
                  "Please close this solution and re-run 'configure.vbs'."
  Case 3
    strErrorMsg = "Debug '" & strFileName & "' does not exist." & vbCRLF & _
                  "Please ensure that the Debug build completed without error."
  Case 4
    strErrorMsg = "Release '" & strFileName & "' does not exist." & vbCRLF & _
                  "Please ensure that the Release build completed without error."
  Case 5
    strErrorMsg = "Unknown configuration: '" & strConfig & "'. Unable to continue."
  Case 6
    stdout.WriteLine " "
    stdout.WriteLine strType & strFileName & "' not copied, already up to date."
    stdout.WriteLine " "
    ' Reset error code
    ExitCode = 0
End Select

' Tidy up objects
Set stdFSO = Nothing
Set stdout = Nothing

' Byee if serious error
if (ExitCode <> 0) Then
  MsgBox strErrorMsg, vbCritical, "Error: " & WScript.ScriptName
  WScript.Quit(ExitCode)
End If

End Sub
