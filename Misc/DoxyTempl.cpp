/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/** \file
* pseudo-implementation file for the DoxyTempl class
*/

#include "DoxyTempl.h"

/** This method ignores its parameter and returns zero.
* \param a A meaningless value
* \param b Another meaningless value
* \return Always zero
* 
* \note If you write docu for a class member (method or variable, 
* you must also write docu for the containing class. Otherwise, doxygen
* might your text.
*
* For complex methods, it might also make sense to add \\callgraph or 
* \\callergraph for a visualization.
*/
float DoxyTempl::doNothing(float a, double b)
{
  /** \todo Do something more meaningful here. */
  return 0;
}
/** \overload
*/
float DoxyTempl::doNothing()
{
  return 0;
}

/** A detailed description should be written into the cpp file.
* This keeps the header file compact, but understandable (if you 
* use brief /// descriptions), while method docu is close to the code.
*
* \pre This function has no precondition, actually.
* \post This function has no postcondition, to be honest ;-)
* \invariant zero equals zero
*/
int returnZero()
{
  /** \bug Something goes wrong here... */
  return 1;
}