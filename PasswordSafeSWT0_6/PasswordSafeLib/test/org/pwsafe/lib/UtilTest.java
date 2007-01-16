package org.pwsafe.lib;

import junit.framework.TestCase;

public class UtilTest extends TestCase {

	public void testCopyBytes() {
		byte[] a = new byte[] { 1 };
		byte[] b = new byte[] { 2 };
		Util.copyBytes(a, b);
		assertEquals(a[0],b[0]);
	}

}
