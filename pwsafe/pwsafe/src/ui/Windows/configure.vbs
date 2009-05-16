'
' Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' Simple VBScript to set up the Visual Studio Properties file for PasswordSafe

Dim objFileSystem, objFileSystem_AT, objFileSystem_OSK
Dim objOutputFile, objOutputFile_AT, objOutputFile_OSK
Dim strOutputFile, strOutputFile_AT, strOutputFile_OSK
Dim strFileLocation
Dim str1, str2, str3,CRLF
Dim rc

CRLF = Chr(13) & Chr(10)
const strHTMLWSDir = "C:\Program Files\HTML Help Workshop"
const strTortoiseSVNDir = "C:\Program Files\TortoiseSVN"
const strExpatDir = "C:\Program Files\Expat 2.0.1"
const strMSXML60SDKDir = "C:\Program Files\MSXML 6.0"
const strXercesDir = "C:\Program Files\xerces-c-3.0.0-x86-windows-vc-8.0"

str1 = "Please supply fully qualified location, without quotes, where "
str2 = " was installed." & CRLF & "Leave empty or pressing Cancel for default to:" & CRLF & CRLF
str3 = CRLF & CRLF & "See README.DEVELOPERS.txt for more information."

strOutputFile = "UserVariables.vsprops"

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
objOutputFile.WriteLine("		Name=""HTMLWSDir""")
strFileLocation = InputBox(str1 & "HTML Help Workshop" & str2 & strHTMLWSDir &str3, "HTML Help Workshop Location", strHTMLWSDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strHTMLWSDir
objOutputFile.WriteLine("		Value=""" & strFileLocation & """")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""TortoiseSVNDir""")
strFileLocation = InputBox(str1 & "Tortoise SVN" & str2 & strTortoiseSVNDir & str3, "Tortoise SVN Location", strTortoiseSVNDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strTortoiseSVNDir
objOutputFile.WriteLine("		Value=""" & strFileLocation & """")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""ExpatDir""")
strFileLocation = InputBox(str1 & "Expat" & str2 & strExpatDir &str3 , "Expat Location", strExpatDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strExpatDir
objOutputFile.WriteLine("		Value=""" & strFileLocation & """")
objOutputFile.WriteLine("		PerformEnvironmentSet=""true""")
objOutputFile.WriteLine("	/>")
objOutputFile.WriteLine("	<UserMacro")
objOutputFile.WriteLine("		Name=""MSXML60SDKDir""")
strFileLocation = InputBox(str1 & "MS XML6 SDK" & str2 & strMSXML60SDKDir &str3, "MS XML6 SDK Location", strMSXML60SDKDir)
If (Len(strFileLocation) = 0) Then strFileLocation = strMSXML60SDKDir
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
objOutputFile.WriteLine("</VisualStudioPropertySheet>")

objOutputFile.Close
Set objFileSystem = Nothing

WScript.Quit(0)
