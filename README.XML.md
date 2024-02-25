## Introduction
Password Safe can be built using different XML libraries.
- The Pugi library (see [pugixml project](http://www.pugixml.org), which is available under the [MIT license](http://www.opensource.org/licenses/mit-license.html)) is part of the source code. This library is used for storage of the runtime parameters in pwsafe.cfg

- Filter and XML Import of databases are supported by

  a) Microsoft XML standard library (available on on Windows) or

  b) Xerces XML library or

  c) Pugi library

For compilation with a or b the relevant Cmake variable must be set: XML_MSXML or XML_XERCES, respectively. Selecting neither implies option c.

The wxWidgets build doesn't support XML_MSXML.

## XERCES
In CMAKE environment for Xerces XML processing, check the "Advanced" checkbox, and set the values of XercesC_INCLUDE_DIR, XercesC_LIBRARY_DEBUG and XercesC_LIBRARY_RELEASE to the correct values.

For example:
```
XercesC_INCLUDE_DIR     C:/local/xerces-c-3.1.3-x86_64-windows-vc-14.0/include
XercesC_LIBRARY_DEBUG   C:/local/xerces-c-3.1.3-x86-windows-vc-14.0/lib
XercesC_LIBRARY_RELEASE C:/local/xerces-c-3.1.3-x86-windows-vc-14.0/lib
```

If you are building x64, you need to build Xerces in x64 mode.

## XSD-Files
The two XSD files
- pwsafe.xsd
- pwsafe_filter.xsd

are used by the Microsoft and Xerces XML builds. The Pugi XML support (syntax check and value check) is hardcoded in the files 
```
src/core/XML/Pugi/PFileXMLProcessor.cpp
src/core/XML/Pugi/PFilterXMLProcessor.cpp
```
HINT: When the database or filter schema changes, both the relevant XSD and Pugi XML code must be updated.

## Automatic loaded filters
As documented, user-defined filters are saved to and loaded from
```
autoload_filters.xml
```

Either in the same directory as pwsafe.cfg or (in Windows) the pwsafe.exe file.

## Filter test
For reading in test of XML coded filter a test file is located as
```
src/test/data/filter_for_test.xml
```
