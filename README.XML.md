## Introduction
The implementation of PWSAFE is using different XML libraries.
- Always on board is the Pugi library (see [pugixml project](http://www.pugixml.org), which is available under the [MIT license](http://www.opensource.org/licenses/mit-license.html)). This library is used for storage of the runtime parameters in pwsafe.cfg
Filter and XML Import of data bases are supported by
  a) Microsoft XML standard library,
  b) Xerces XML library or
  c) Pugi library
Microsoft XML library is available for Windows only. The Xerces 
For compilation witrh a) or b) the related environment variable must be set in compilation - XML_MSXML or XML_XECESC. Leaving out both pre-compiler flags lead to ´compilation with variant c).

The wx build doesn't support XML_MSXML.

## XERCES
In CMAKE environment for Xerces XML processing, check the "Advanced" checkbox, and set the values of XercesC_INCLUDE_DIR, XercesC_LIBRARY_DEBUG and XercesC_LIBRARY_RELEASE to the correct values.

Note: Currently only 32-bit compilations are supported.

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
are used for Microsoft and Xerces XML libararies. The Pugi XML support (syntax check and value check) is hard coded in the files located in 
```
src/core/XML/Pugi/PFileXMLProcessor.cpp
src/core/XML/Pugi/PFilterXMLProcessor.cpp
```
HINT: Whenever the structure of the XML filter or XML export of data base is extended or adapted the change of the related XSD file and the change of the Pugi support must be done in parallel.

## Automatic loaded filters
A not documented feature is the loading of standard filters from file
```
autoload_filters.xml
```

This file with default filters, in addition to the filters that are hard coded inside of PWSAFE, can be generated using filter export. The location of the file should be in line with the location of pwsafe.cfg file. In Windows environment the location of the same location as the executable might fit too.

## Filter test
For reading in test of XML coded filter a test file is located as
```
src/test/data/filter_for_test.xml
```
