package org.pwsafe.lib.crypto;

import org.pwsafe.lib.Util;

import junit.framework.TestCase;

/**
 * Test SHA256.
 * 
 * @author Glen Smith
 */
public class SHA256PwsTest extends TestCase {

	public void testDigest() {

		String data = "abc";
		byte[] digest = SHA256Pws.digest(data.getBytes());

		String result = Util.bytesToHex(digest);
		assertEquals("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", result);

	}

}
