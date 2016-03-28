'
' Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' Simple VBScript to set up the Visual Studio Properties file for PasswordSafe
' This script is for setting up Visual Studio 2005 (MSVS8). For Visual
' Studio 2010 (MSVS10), please use configure.vbs

Dim objFileSystem, objOutputFile
Dim strOutputFile
Dim strFileLocation
Dim str1, str2, str3,CRLF
Dim rc

Dim XML_XPATH, strPgmFiles
Dim strGitDir, strXercesDir, strXerces64Dir, strWXDir
Dim strKeyPath, strValueName, strValue

CRLF = Chr(13) & Chr(10)

' Check if running 64-bit OS
' If running a 64-bit Windows OS, as PasswordSafe is a 32-bit application,
' developers should install the 32-bit version of Xerces XML library.
' Note: the 8.0 in the Xerces directory corresponds to VS2005; 9.0 to VS2008 etc.
' wxWidgets only come in a 32-bit version.
' Default installation of wxWidgets is in a root directory. Changed here to be
' under the 'C:\Program Files' or 'C:\Program Files (x86)' directory.


const HLM = &H80000002
strComputer = "."
strPgmFiles = ""

Set oReg = GetObject("winmgmts:{impersonationLevel=impersonate}!\\" &_
                     strComputer & "\root\default:StdRegProv")

strKeyPath = "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"
strValueName = "PROCESSOR_ARCHITECTURE"
oReg.GetStringValue HLM, strKeyPath, strValueName, strValue

If strValue = "AMD64" Then
  strPgmFiles = " (x86)"
End If

Set oReg = Nothing

' Set defaults
strGitDir = "C:\Program Files (x86)\Git"
strXercesDir = "C:\Program Files" & strPgmFiles & "\xerces-c-3.1.1-x86-windows-vc-8.0"
strXerces64Dir = "C:\Program Files\xerces-c-3.1.1-x86_64-windows-vc-8.0"
strWXDir = "C:\Program Files" & strPgmFiles & "\wxWidgets-2.8.10"

str1 = "Please supply fully qualified location, without quotes, where "
str2 = " was installed." & CRLF & "Leave empty or pressing Cancel for default to:" & CRLF & CRLF
str3 = CRLF & CRLF & "See README.DEVELOPERS.txt for more information."

strOutputFile = "UserVariables.vsprops"
XML_XPATH="VisualStudioPropertySheet/UserMacro"

Set objXMLDoc = CreateObject("Microsoft.XMLDOM")
objXMLDoc.async = False
objXMLDoc.load(strOutputFile)

' If already exists, set the defaults to be current value so user doesn't have to
' remember what they set last time
Set UserMacros = objXMLDoc.getElementsByTagName(XML_XPATH)
If UserMacros.length > 0 Then
  For each CurrentUserMacro in UserMacros
    If CurrentUserMacro.Attributes.getNamedItem ("Name").text = "GitDir" Then
      strGitDir = CurrentUserMacro.Attributes.getNamedItem("Value").text
    End If
    If CurrentUserMacro.Attributes.getNamedItem ("Name").text = "XercesDir" Then
      strXercesDir = CurrentUserMacro.Attributes.getNamedItem("Value").text
    End If
    If CurrentUserMacro.Attributes.getNamedItem ("Name").text = "Xerces64Dir" Then
      strXerces64Dir = CurrentUserMacro.Attributes.getNamedItem("Value").text
    End If
    If CurrentUserMacro.Attributes.getNamedItem ("Name").text = "WXDIR" Then
      strWXDir = CurrentUserMacro.Attributes.getNamedItem("Value").text
    End If
  Next
End If

Set UserMacros = Nothing
Set objXMLDoc = Nothing

Set objFileSystem = CreateObject("Scripting.fileSystemObject")

If (objFileSystem.FileExists(strOutputFile)) Then
  ' vbYesNo | vbQuestion | vbDefaultButton2 = 4 + 32 + 256 = 292
  rc = MsgBox("File """ & strOutputFile & """ already exists! OK to overwrite?", 292)
  ' vbNo = 7
  If (rc = 7) Then
    Set objFileSystem = Nothing
    WScript.Quit(0)
  End If
End If

Set objOutputFile = objFileSystem.CreateTextFile(strOutputFile, TRUE)

objOutputFile.WriteLine("<?xml version=""1.0"" encoding=""Windows-1252""?>")
objOutputFile.WriteLine("<VisualStudioPropertySheet")
objOutputFile.WriteLine("  ProjectType=""Visual C++""")
objOutputFile.WriteLine("	Version=""8.00""")
objOutputFile.WriteLine("	Name=""UserVariables""")
objOutputFile.WriteLine("	>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""ProjectDir""")
objOutputFile.WriteLine("		Value=""&quot;$(ProjectDir)&quot;""")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""ConfigurationName""")
objOutputFile.WriteLine("		Value=""$(ConfigurationName)""")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""OutDir""")
objOutputFile.WriteLine("		Value=""$(OutDir)""")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""PWSBin""")
objOutputFile.WriteLine("		Value=""..\..\build8\bin\pwsafe\$(ConfigurationName)""")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""PWSObj""")
objOutputFile.WriteLine("		Value=""..\..\build8\obj\pwsafe\$(ConfigurationName)""")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""PWSLib""")
objOutputFile.WriteLine("		Value=""..\..\build8\lib\pwsafe\$(ConfigurationName)""")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""GitDir""")
strFileLocation = InputBox(str1 & "Git" & str2 & strGitDir & str3, "Git Location", strGitDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strGitDir
objOutputFile.WriteLine("		Value=""" & strFileLocation & """")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""XercesDir""")
strFileLocation = InputBox(str1 & "Xerces" & str2 & strXercesDir & str3, "Xerces Location", strXercesDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strXercesDir
objOutputFile.WriteLine("		Value=""" & strFileLocation & """")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""Xerces64Dir""")
strFileLocation = InputBox(str1 & "Xerces" & str2 & strXerces64Dir & str3, "Xerces 64-bit Location", strXerces64Dir)
If (Len(strFileLocation) = 0) Then strFileLocation = strXerces64Dir
objOutputFile.WriteLine("		Value=""" & strFileLocation & """")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""WXDIR""")
strFileLocation = InputBox(str1 & "wxWidgets" & str2 & strWXDir & str3, "wxWidgets Location", strWXDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strWXDir
objOutputFile.WriteLine("		Value=""" & strFileLocation & """")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("</VisualStudioPropertySheet>")

objOutputFile.Close
Set objFileSystem = Nothing

Call MsgBox("File UserVariables.vsprops created successfully", 0, "Configure User Variables")

WScript.Quit(0)
