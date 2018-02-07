/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PROXY_H
#define __PROXY_H

/**
 * Abstract base classes used by core to interface with the user.
 * UI-specific code should derive concrete classes and pass pointers
 * to objects of derived class for use by core.
 */

// abstract base class for asking user a question
// and getting a yes/no reply
class Asker {
public:
  virtual bool operator()(const stringT &question) = 0;
  virtual bool operator()(const stringT &title, const stringT &question) = 0;
  virtual ~Asker() {} // keep compiler happy
};

// abstract base class for reporting something of
// interest to the user
class Reporter {
public:
  virtual void operator()(const stringT &title, const stringT &message) = 0;
  virtual void operator()(const stringT &message) = 0;
  virtual ~Reporter() {} // keep compiler happy
};

#endif /* __PROXY_H */
