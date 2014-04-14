'
' Copyright (c) 2003-2014 Rony Shapiro <ronys@users.sourceforge.net>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' Simple VBScript to set up the Visual Studio Properties file for PasswordSafe
' This script is for setting up Visual Studio 2013.

Dim objFileSystem, objOutputFile
Dim strOutputFile
Dim strFileLocation
Dim str1, str2, str3, CRLF
Dim rc

Dim Node, XML_XPATH, strPgmFiles
Dim strGitDir, strXercesDir, strXerces64Dir, strWXDir, strWDKDir
Dim strKeyPath, strValueName, strValue

CRLF = Chr(13) & Chr(10)

' Check if running 64-bit OS
' If running a 64-bit Windows OS, as PasswordSafe is a 32-bit application,
' developers should install the 32-bit version of Xerces XML library.
' Note: the 12.0 in the Xerces directory corresponds to VS2013
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
strXercesDir = "C:\Program Files" & strPgmFiles & "\xerces-c-3.1.1-x86-windows-vc-12.0"
strXerces64Dir = "C:\Program Files\xerces-c-3.1.1-x86_64-windows-vc-12.0"
strWXDir = "C:\Program Files" & strPgmFiles & "\wxWidgets-2.8.12"
strWDKDir = "C:\Program Files" & strPgmFiles & "\Windows Kits\8.1"

str1 = "Please supply fully qualified location, without quotes, where "
str2 = " was installed." & CRLF & "Leave empty or pressing Cancel for default to:" & CRLF & CRLF
str3 = CRLF & CRLF & "See README.DEVELOPERS.txt for more information."

strOutputFile = "UserVariables-12.props"

Set objFileSystem = CreateObject("Scripting.fileSystemObject")

' Check if a VS2010 props file already exists
If (objFileSystem.FileExists(strOutputFile)) Then
  Set objXMLDoc = CreateObject("Microsoft.XMLDOM")
  objXMLDoc.async = False
  objXMLDoc.load(strOutputFile)

  ' If already exists, set the defaults to be current value so user doesn't have to
  ' remember what they set last time
  Set Node = objXMLDoc.documentElement.selectSingleNode("PropertyGroup/GitDir")
  If Not Node Is Nothing Then
    strGitDir = Node.text
  End If
  Set Node = objXMLDoc.documentElement.selectSingleNode("PropertyGroup/XercesDir")
  If Not Node Is Nothing Then
    strXercesDir = Node.text
  End If
  Set Node = objXMLDoc.documentElement.selectSingleNode("PropertyGroup/Xerces64Dir")
  If Not Node Is Nothing Then
    strXerces64Dir = Node.text
  End If
  Set Node = objXMLDoc.documentElement.selectSingleNode("PropertyGroup/WXDIR")
  If Not Node Is Nothing Then
    strWXDir = Node.text
  End If
  Set Node = objXMLDoc.documentElement.selectSingleNode("PropertyGroup/WDKDIR")
  If Not Node Is Nothing Then
    strWDKDir = Node.text
  End If

  Set Node = Nothing
  Set objXMLDoc = Nothing
End If

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

objOutputFile.WriteLine("<?xml version=""1.0"" encoding=""utf-8""?>")
objOutputFile.WriteLine("<Project DefaultTargets=""Build"" ToolsVersion=""12.0"" xmlns=""http://schemas.microsoft.com/developer/msbuild/2003"">")
objOutputFile.WriteLine("  <PropertyGroup Label=""UserMacros"">")
objOutputFile.WriteLine("    <ConfigurationName>$(Configuration)</ConfigurationName>")

strFileLocation = InputBox(str1 & "GitDir" & str2 & strGitDir & str3, "Git Location", strGitDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strGitDir

objOutputFile.WriteLine("    <GitDir>" & strFileLocation & "</GitDir>")

strFileLocation = InputBox(str1 & "Xerces" & str2 & strXercesDir & str3, "Xerces Location", strXercesDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strXercesDir

objOutputFile.WriteLine("    <XercesDir>" & strFileLocation & "</XercesDir>")

strFileLocation = InputBox(str1 & "Xerces" & str2 & strXerces64Dir & str3, "Xerces 64-bit Location", strXerces64Dir)
If (Len(strFileLocation) = 0) Then strFileLocation = strXerces64Dir

objOutputFile.WriteLine("    <Xerces64Dir>" & strFileLocation & "</Xerces64Dir>")

strFileLocation = InputBox(str1 & "wxWidgets" & str2 & strWXDir & str3, "wxWidgets Location", strWXDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strWXDir

objOutputFile.WriteLine("    <WXDIR>" & strFileLocation & "</WXDIR>")

strFileLocation = InputBox(str1 & "Windows Driver Kit" & str2 & strWDKDir & str3, "WDK Location", strWDKDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strWDKDir

objOutputFile.WriteLine("    <WDKDIR>" & strFileLocation & "</WDKDIR>")

objOutputFile.WriteLine("    <PWSBin>..\..\build\bin\pwsafe\$(Configuration)</PWSBin>")
objOutputFile.WriteLine("    <PWSLib>..\..\build\lib\pwsafe\$(Configuration)</PWSLib>")
objOutputFile.WriteLine("    <PWSObj>..\..\build\obj\pwsafe\$(Configuration)</PWSObj>")
objOutputFile.WriteLine("  </PropertyGroup>")
objOutputFile.WriteLine("  <ItemGroup>")
objOutputFile.WriteLine("    <BuildMacro Include=""ProjectDir"">")
objOutputFile.WriteLine("      <Value>$(ProjectDir)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("    <BuildMacro Include=""ConfigurationName"">")
objOutputFile.WriteLine("      <Value>$(ConfigurationName)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("    <BuildMacro Include=""PWSBin"">")
objOutputFile.WriteLine("      <Value>$(PWSBin)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("    <BuildMacro Include=""PWSLib"">")
objOutputFile.WriteLine("      <Value>$(PWSLib)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("    <BuildMacro Include=""PWSObj"">")
objOutputFile.WriteLine("      <Value>$(PWSObj)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("    <BuildMacro Include=""OutDir"">")
objOutputFile.WriteLine("      <Value>$(OutDir)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("    <BuildMacro Include=""GitDir"">")
objOutputFile.WriteLine("      <Value>$(GitDir)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("    <BuildMacro Include=""XercesDir"">")
objOutputFile.WriteLine("      <Value>$(XercesDir)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("    <BuildMacro Include=""Xerces64Dir"">")
objOutputFile.WriteLine("      <Value>$(Xerces64Dir)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("    <BuildMacro Include=""WXDIR"">")
objOutputFile.WriteLine("      <Value>$(WXDIR)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("    <BuildMacro Include=""WDKDIR"">")
objOutputFile.WriteLine("      <Value>$(WDKDIR)</Value>")
objOutputFile.WriteLine("      <EnvironmentVariable>true</EnvironmentVariable>")
objOutputFile.WriteLine("    </BuildMacro>")
objOutputFile.WriteLine("  </ItemGroup>")
objOutputFile.WriteLine("</Project>")

objOutputFile.Close
Set objFileSystem = Nothing

Call MsgBox("File UserVariables.props created successfully", 0, "Configure User Variables")

WScript.Quit(0)
