'
' Create language translation DLLs
'
' cscript Create_DLLs.vbs [LL | /h | /help | /?] [x86 | x64]
'   LL - optional single language
'   /h | /help | /? - show help
'   x86 | x64 - architecture choice. If LL is given and architecture 
'     is omitted, defaults to x86.
' If no arguments are supplied, help is shown.
'
' The list of available translations is defined by the Tranlsations.txt file.
'
' See Readme.txt for more information.
'

If Instr(1, WScript.FullName, "cscript.exe", vbTextCompare) = 0 then
    MsgBox "This script must be executed by cscript.exe" & vbCRLF & _
           "e.g. cscript Create_DLLs.vbs", _
           vbCritical, _
           "Error: " & Wscript.ScriptFullName
    ' return error code to caller
    Wscript.Quit(99)
End If

IncludeFile "Translations.vbs"

' Default conditions
DO_ALL = True
ARCH = "x86"
DO_COUNTRY = Empty
DO_QUIET = False

'
' Argument syntax: [-q | -quiet] [LL] [x86 | x64] [/help | /h | /?]
' Basically, if an argument that is not well known is treated as a LL value
' Oddly, this is far simpler and more flexible code than its predecessor!
'
Dim argx
For argx = 0 To Wscript.Arguments.Count - 1
  Dim arg
  arg = UCase(Wscript.Arguments(argx))
  Select Case arg
    Case "X86"
      ARCH = "x86"
    Case "X64"
      ARCH = "x64"
    Case "-Q", "-QUIET"
      DO_QUIET = True
    Case "/HELP", "/H", "/?" 
      Usage
      Wscript.Quit(0)
    Case Else
      ' Currently languages must be 2 characters long
      If Len(Arg1) = 2 Then
        DO_ALL = False
        DO_COUNTRY = Arg1
      Else
        ' If the argument is none of the above, show help and exit
        Usage
        Wscript.Quit(0)
      End If
  End Select
Next

Dim CURPATH, TOOLS, RESTEXT, RESPWSL, BASE_DLL, DEST_DIR, DO_ALL, DO_COUNTRY, ARCH

' Configure tool and output location

If ARCH = "x86" Then
  ' 32-bit tools
  TOOLS = "..\..\..\..\build\bin"
  RESTEXT = TOOLS & "\restext\release\ResText.exe"
  RESPWSL = TOOLS & "\respwsl\release\ResPWSL.exe"
  BASE_DLL = "..\..\..\..\build\bin\pwsafe\release\pwsafe_base.dll"
  DEST_DIR = "..\..\..\..\build\bin\pwsafe\I18N\"
  WScript.Echo "Creating 32-bit language DLLs"
Else
  ' 64-bit tools
  TOOLS = "..\..\..\..\build\bin"
  RESTEXT = TOOLS & "\restext\release64\ResText.exe"
  RESPWSL = TOOLS & "\respwsl\release64\ResPWSL.exe"
  BASE_DLL = "..\..\..\..\build\bin\pwsafe\release64\pwsafe_base.dll"
  DEST_DIR = "..\..\..\..\build\bin\pwsafe\I18N64\"
  WScript.Echo "Creating 64-bit language DLLs"
End If

' Used through out the script
Dim objFSO
Set objFSO = CreateObject("Scripting.FileSystemObject")

' Make absolute addresses
DEST_DIR = objFSO.GetAbsolutePathName(DEST_DIR)
DEST_DIR = DEST_DIR & "\"

CURPATH = objFSO.GetAbsolutePathName(".")
CURPATH = CURPATH & "\"

' Sometimes the delete files do not work for permission failures. So...
' It is not clear what permissions are a problem. Could be that
' the script needs to be run as Administrator, but there appears
' to be nothing special about the permissions of the DLLs.
If Not DO_QUIET Then
  WScript.Echo " "
  WScript.Echo "*** Please ensure that you have deleted ALL DLLs from this script directory:"
  WScript.Echo  "    " & CURPATH
  WScript.Echo  " "
  WScript.Echo "*** Please delete any DLLs you are (re)making from the target directory:"
  WScript.Echo  "    " & DEST_DIR
  WScript.Echo  " "
  Pause("Press any key to continue.")
End if

' Make absolute addresses
DEST_DIR = objFSO.GetAbsolutePathName(DEST_DIR)
DEST_DIR = DEST_DIR & "\"

CURPATH = objFSO.GetAbsolutePathName(".")
CURPATH = CURPATH & "\"

' Check output directory exists and, if not, create it
CreateFolder(DEST_DIR)

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

If (Not objFSO.FileExists(RESPWSL)) Then
  ' Check required program exists
  WScript.Echo "Can't find Tool - ResPWSL.exe"
  WScript.Quit(99)
End If

' Read in the list of all available translations
Dim TransList
TransList = BuildTranslationList()

' Create DLLs based on command line arguments

If DO_ALL = True Then
  ' Do all countries in Translations.txt
  Wscript.Echo "Creating translation DLLs for all countries in Translations.txt", vbCRLF
  Dim t
  For Each t in TransList
    Wscript.Echo "Creating translation DLL for country", UCase(t.LL)
    DoI18N LCase(t.FileName), t.LCID, t.LL_CC, UCase(t.LL)
  Next
Else
  ' Do one country
  Wscript.Echo "Creating translation DLL for country", DO_COUNTRY
  Dim OneTranslation
  Set OneTranslation = FindCountry(TransList, DO_COUNTRY)
  If Not OneTranslation Is Nothing Then
    DoI18N OneTranslation.FileName, OneTranslation.LCID, OneTranslation.LL_CC, OneTranslation.LL
  Else
    WScript.Echo "Error:", DO_COUNTRY, "is not a valid language"
    WScript.Quit(99)
  End If
End If

Wscript.Echo "Finished"
Wscript.Quit(0)

' End of main
'----------------------------------------------------
' Start of subroutines and functions

Sub DoI18N(POFileName, LCID, LL_CC, LL)
  ' Create a language DLL
  ' Parameters:
  ' 1. Name of file in sub-directory "pos" e.g. "pwsafe_zh.po"
  ' 2. LCID e.g. 0x0804 for Chinese (Simplified)
  ' 3. Generated DLL in form LL_CC e.g. "ZH_CN" for "pwsafeZH_CN.dll"
  '    NOTE: This is determined by the LCID and is not a free choice! See Windows XP version of
  '    http://www.microsoft.com/resources/msdn/goglobal/default.mspx
  '    as this generates the 2-character LL and CC values (later OSes can generate other values).
  ' 4. Final DLL name in form LL e.g. "ZH" for "pwsafeZH.dll"

  If (Not objFSO.FileExists("pos\" & POFileName)) Then
    ' Check required PO file exists
    WScript.Echo "   Can't find requested PO file - pos\" & POFileName
    WScript.StdOut.WriteLine "   Skipped"
    Exit Sub
  End If

  Dim WshShell, oExec
  Set WshShell = CreateObject("WScript.Shell")

  ' Create new intermediate DLL using PO file to replace strings
  Set oExec = WshShell.Exec(RESTEXT & " apply " & BASE_DLL & " foo.dll " & "pos\" & POFileName)

  Do While oExec.Status = 0
    WScript.StdOut.Write ".."
    WScript.Sleep 100
  Loop

  ' Create new DLL with correct name and update version information
  Set oExec = WshShell.Exec(RESPWSL & " apply foo.dll " & LCID)

  Do While oExec.Status = 0
    WScript.StdOut.Write ".."
    WScript.Sleep 100
  Loop

  If (objFSO.FileExists("foo.dll")) Then
    'Delete intermediate DLL if still there
    On Error Resume Next
    objFSO.DeleteFile "foo.dll"
  End If

  If (objFSO.FileExists(DEST_DIR & "pwsafe" & LL & ".dll")) Then
    ' Delete any old version of this language resource-only DLL
    On Error Resume Next
    objFSO.DeleteFile DEST_DIR & "pwsafe" & LL & ".dll"
  End If

  ' Move and rename the new DLL
  objFSO.MoveFile "pwsafe" & LL_CC & ".dll", DEST_DIR & "pwsafe" & LL & ".dll"

  Wscript.StdOut.WriteLine "Done"

  Set oExec = Nothing
  Set WshShell = Nothing

  End Sub

' Recursive folder create, will create directories and Sub
Sub CreateFolder(strPath)
  On Error Resume Next
  If strPath <> "" Then 'Fixes endless recursion in some instances when at lowest directory
    If Not objFSO.FolderExists(objFSO.GetParentFolderName(strPath)) then 
      Call CreateFolder(objFSO.GetParentFolderName(strPath))
    End If
    objFSO.CreateFolder(strPath)
  End If 
End Sub

Sub Pause(strPause)
  Dim z
  WScript.Echo (strPause)
  z = WScript.StdIn.Read(1)
End Sub

Sub Usage
  MsgBox "This script must be executed by cscript.exe in a Command window" & vbCRLF & _
    vbCRLF & _
    "cscript Create_DLLs.vbs [-q | -quiet] [LL] [/h | /help | /?] [x86 | x64]" & vbCRLF & _
    "    -q | -quiet -  do not prompt user" & vbCRLF & _
    "    LL - a two letter language code for a specific language DLL" & vbCRLF & _
    "         e.g cscript Create_DLLs.vbs DE ==> for German/Germany" & vbCRLF & _
    "    /h | /help | /? - show help" & vbCRLF & _
    "    x86 -  produce 32-bit DLLs (default)" & vbCRLF & _ 
    "    x64 -  produce 64-bit DLLs" & vbCRLF & _
    vbCRLF & _
    "Examples" & vbCRLF & _
    "    cscript Create_DLLs.vbs x86 ==> create 32-bit DLLs for all languages" & vbCRLF & _
    "    cscript Create_DLLs.vbs x64 ==> create 64-bit DLLs for all languages" & vbCRLF & _
    "    cscript Create_DLLs.vbs ZH x64 ==> create 64-bit DLL for Chinese" & vbCRLF & _
    "", _
    vbInformation, _
    "Help: Create_DLLs.vbs"

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
