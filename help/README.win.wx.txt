In order to make a Help zip file that can be used by the Windows wxWdigets build do:

1. Using Windows Explorer, navigate to the appropriate Help subdirectory i.e. default, pwsafeDE, etc.
2. Right click on the "html" folder and select "Send to" -> "Compressed (zipped) file".  This will create a file "html.zip".
3. Drag (copy) the 3 files: "pwsafe.hhc", "pwsafeXX.hhk" & "pwsafeXX.hhp" files onto the "htmp.zip".  Note: 'XX' is empty for the default English help otherwise it represents the language i.e. DE.
4. Rename "html.zip" file to "helpXX.zip, where XX is "EN" for the default English help file otherwise the language identifier i.e. "DE".
5. Copy this file into the wxWidgets PasswordSafe execution directory.
