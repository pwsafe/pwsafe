For Developers using Microsoft's Visual Studio, changes have been made to
allow the product to pick up the required include and library files from
the installation directories that can be overriden by the user.

In order for the solution to open under these products, you MUST run the
"configure.vbs" script file to create the "UserVariables.vsprops" [Visual
Studio Properties file] prior to opening this solution with Visual Studio.
THE SOLUTION WILL NOT OPEN CORRECTLY WITHOUT THIS FILE BEING PRESENT.

The product default directories are already defined.  They are for:

For GUI HTML help support:
	Microsoft's HTML Help Workshop

For updating the revision number within Password Safe's version details:
  Tortoise SVN Client

For XML Support
  Expat's V2.0.1 XML Library
  Microsoft's XML V6.0 SDK
  Xerces V3.0.0 XML Library

In addition, there is a local definition of an environmental variable
"ProjectDir" completed from Visual Studio's internal value.  Do not change
or delete this entry.

As noted above, this project has been updated to support 3 types of XML
processing until a decision is made on which one to use.  Microsoft's XML
library is only available for the Windows Platform.  Expat and Xerces are
available for non-Windows platforms.  See their web sites for more
information.  Note: Xerces V3.0.0 at revision 707374 or greater is needed
to correct a memory leak processing password history entries.

A new preprocessor variable (USE_XML_LIBRARY) has been utilised.  If NOT
defined, Password Safe will not process XML files either as a file or filter
import.  Filters within a database will be treated as 'Unknown Fields' and
will remain unchanged so that other versions of Password Safe that do support
XML can still use them.

The three valid values of the USE_XML_LIBRARY variable are EXPAT, MSXML and
XERCES corresponding to the three different implementations.

Password Safe uses SAX2 processing for both Microsoft's and the Xerces'
XML Parsers, which are full validating parsers and require the presence of
the XSD Schema file to process either file or filter imports.  This ensures
that the data eventually hitting the SAX2 content handlers is correct and just
needs placing in the appropriate fields.

Expat is a non-validating XML Parser and uses its own API rather than SAX2.
Note: SAX2 is only an official standard for Java XML processing although many C
supporting parsers also support this interface.  Because Expat is non-validating,
code has to be added directly into Password Safe to perform this validation, so
that the program and its databases are not corrupted by poorly formed XML or
incorrect data within the XML file.

Appropriate libraries can be obtained from:

http://expat.sourceforge.net/
http://www.microsoft.com/downloads/details.aspx?FamilyID=993C0BCF-3BCF-4009-BE21-27E85E1857B1&displaylang=en
http://xerces.apache.org/xerces-c/

NOTE: as of 2 November 2008, the additional validation code for Expat and
non-Unicode versions of all three implementations are in progress.  Work
is also needed to ensure that error processing is correct and that associated
messages are produced correctly.
