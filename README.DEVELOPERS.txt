1. In order to build the Windows version of PasswordSafe, you need to
install the following freely available components:

1.1 Microsoft's HTML Help Workshop (for online help support)
1.2 Tortoise SVN Client (For updating the revision number within
Password Safe's version details)
1.3 For XML Support
  Microsoft's XML V6.0 SDK (http://www.microsoft.com/downloads/details.aspx?FamilyID=993C0BCF-3BCF-4009-BE21-27E85E1857B1&displaylang=en)
  Expat's V2.0.1 XML Library (http://expat.sourceforge.net/)
  Xerces V3.0.0 XML Library (http://xerces.apache.org/xerces-c/)

See XXX for information on where to find these components.

2. If you're using Microsoft's Visual Studio, the file
UserVariables.vsprops must exist and point to the directories where
the above components have been installed.

THE SOLUTION WILL NOT OPEN CORRECTLY WITHOUT THIS FILE BEING PRESENT.

For your convenience, we've included a simple Visual Basic script,
configure.vbs, that you can run to generate the UserVariables.vsprops
file with the correct values.

3. XML Processing: Originally, PasswordSafe used Microsoft's XML
library for XML parsing and validation. For portability, we're now
working on support for the Expat and Xerces XML libraries, which are
available for non-Windows platforms.  See their web sites for more
information.  Note: Xerces V3.0.0 at revision 707374 or greater is needed
to correct a memory leak processing password history entries.

To determine which XML library to use while building PasswordSafe,
we've defined a new preprocessor variable (USE_XML_LIBRARY). If NOT
defined, the compiled Password Safe will not process XML files
(databases or filter definitions in XML format). Filters within a
database will be treated as 'Unknown Fields' and will remain unchanged
so that other versions of Password Safe that do support XML parsing
can still use them. Note, however, that the application will be able
to write XML files.

The three valid values of the USE_XML_LIBRARY variable are EXPAT, MSXML and
XERCES corresponding to the three different implementations.

Password Safe uses SAX2 processing for both Microsoft's and the Xerces'
XML Parsers, which are full validating parsers and require the presence of
the XSD Schema file to process either file or filter imports.  This ensures
that the data eventually hitting the SAX2 content handlers is correct and just
needs placing in the appropriate fields.

Expat is a non-validating XML Parser and uses its own API rather than SAX2.
Note: SAX2 is only an official standard for Java XML processing although many C
supporting parsers also support this interface.  Because Expat is
non-validating, code has to be added directly into Password Safe to
perform this validation, so  that the program and its databases are
not corrupted by poorly formed XML or incorrect data within the XML file.

NOTE: as of 2 November 2008, the additional validation code for Expat and
non-Unicode versions of all three implementations are in progress.  Work
is also needed to ensure that error processing is correct and that associated
messages are produced correctly.
