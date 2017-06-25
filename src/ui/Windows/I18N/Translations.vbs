'
' Common code used by Create_DLLs.vbs and Update_POs.vbs
'

' Represents a record from the Translations.txt file
Class Translation
  Public FileName
  Public LCID
  Public LL_CC
  Public LL
End Class

' Reads Translations.txt and builds a list of PO files
' to be processed as language translations
Function BuildTranslationList()
  ' File system definitions for reading/writing files
  Const ForReading = 1, ForWriting = 2, ForAppending = 8
  Const TristateUseDefault = -2, TristateTrue = -1, TristateFalse = 0

  Dim TranList(), TranListSize
  ReDim TranList(1000)
  TranListSize = 0

  ' Set up regex for parsing out file and locale info from a line
  Dim re
  Set re = New RegExp
  re.Global = True
  re.IgnoreCase = True
  ' A translation record should look something like:
  ' #POFile       lcid   LL_CC LL  
  ' pwsafe_cz.po  0x0405 CS_CZ CS
  re.Pattern = "(^#)|(\w+\.\w+)|(\w+)"

  ' Read through the Translations.txt file
  Dim fso
  Set fso = CreateObject("Scripting.FileSystemObject")
  Dim POFile
  Set POFile = fso.OpenTextFile("Translations.txt", ForReading, False, TristateFalse)
  Dim line, lineCounter
  Dim tokens
  lineCounter = 1
  Do While POFile.AtEndOfStream <> True
      line = POFile.ReadLine
      if line <> "" and mid(line, 1, 1) <> "#" Then
        Set tokens = re.Execute(line)
        ' A valid record must have at least 4 tokens and cannot start with a #
        If tokens.Count >= 4 Then
          Dim tran
          Set tran = New Translation
          tran.FileName = tokens(0)
          tran.LCID = tokens(1)
          tran.LL_CC = tokens(2)
          tran.LL = tokens(3)
          ' The first array element is 0
          Set TranList(TranListSize) = tran
          TranListSize = TranListSize + 1
        Else
          Wscript.Echo "Invalid translation mapping file record at line", lineCounter, "<", line, ">"
        End If
      End If
      lineCounter = lineCounter + 1
  Loop  
  POFile.Close
  
  ' Clean up
  Set re = Nothing
  Set POFile = Nothing
  Set tokens = Nothing
  Set fso = Nothing
  
  ' Re-dimension the array to the exact number of elements used (sets the UBound value)
  ' Elements are indexed as 0 to UBound
  ReDim Preserve TranList(TranListSize - 1)

  ' Note that the first valid array element is TranList(0)
  BuildTranslationList = TranList
  
End Function

' Search a translations array for a matching country.
' This is a brute force search because the list is not very long.
Function FindCountry(trans, cc)
  Dim country
  Set country = Nothing

  Dim t
  For Each t in trans
    If UCase(t.LL) = UCase(cc) Then
      Set country = t
      Exit For
    End If
  Next
  
  Set FindCountry = country
  
End Function
