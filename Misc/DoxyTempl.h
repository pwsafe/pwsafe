/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

/** \file
* This is a template header file, intended to demonstrate how
* special comment blocks are used. This enables doxygen 
* (http://www.doxygen.org)to generate source code documentation 
* automatically.
*
* \par How to generate docu files
* Under Windows, you may use Doxywizard to generate docu. Load Doxyfile 
* (located in your pwsafe working copy) as config file and be sure to 
* enter the path to your working copy as doxygen's working directory.
* After processing, the output HTML files will be in the doxygen subdirectory
* of your working copy - simply open index.html in $BROWSER.
*
* \par How to embed documentation into your code
* You can write documentation for files, classes, functions/ methods and 
* their parameters, variables, data types (such as structs, typedefs etc.)
* \par
* Usually, you write special comment blocks right before the documented 
* entity in your code. You will see examples below. One exception to this
* rule is docu for files - you have to write the special command "\file" 
* on a separate line before your text, just like in this block.
*
* \par Special code block delimiters
* Doxygen parses only code blocks with a special format. I prefer to use 
* Javadoc style, i.e. you open a code block with two asterisks after the 
* slash. However, you can also use Qt- or C#-style comments. More information
* can be found in the doxygen user manual.
*
* \version 1.0
* \date 2007/03/30
* \see Doxygen manual, http://www.stack.nl/~dimitri/doxygen/manual.html
* \since Revision 1335
*/

/** This is docu for a typedef.
*
* \note
* If you write docu for a global entity like typedefs, enums or defines, be 
* sure to write a docu block for the containing file as well! 
* Otherwise, doxygen might omit your comment.
*/
typedef myType int;

/** \brief A class for demonstration purposes
* \author Your Name here
*
* There is nothing interesting with this class. But you can see how the \\brief 
* and \\author commands work :-)
*/
class DoxyTempl
{
public:
  /// A brief comment at the declaration. Details can go to definition
  float doNothing(float a, double b);
  float doNothing();

private:
  /** Demo documentation of a member variable
  */
  int foo;
  /// Or here the brief form (instead of /** \\brief bla*/)
  int foo2;
  char *bar; /**< short notation for documenting members (or parameters) */
};

/// You can omit the \\brief command if you write one line after three slashes.
int returnZero();
