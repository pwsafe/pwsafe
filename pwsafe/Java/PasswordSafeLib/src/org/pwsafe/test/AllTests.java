/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.test;

import junit.framework.Test;
import junit.framework.TestSuite;

/**
 *
 */
public class AllTests
{
	/**
	 * Returns a JUnit test suite containing all valid test cases.
	 * 
	 * @return A JUnit test suite containing all valid test cases.
	 */
	public static Test suite()
	{
		TestSuite suite = new TestSuite("Test for org.pwsafe.test");
		//$JUnit-BEGIN$
		suite.addTestSuite(PassphraseUtilsTest.class);
		suite.addTestSuite(UtilTest.class);
		suite.addTestSuite(FileTest.class);
		//$JUnit-END$
		return suite;
	}
}