package org.pwsafe.lib.crypto;

import org.pwsafe.lib.Util;

import junit.framework.TestCase;

/**
 * Test HMAC operation.
 * 
 * @author Glen Smith
 */
public class HmacPwsTest extends TestCase {

	
    public void testDigest() {
        
        String key = "Jefe";
        String data = "what do ya want for nothing?";
        byte[] hmac = HmacPws.digest(key.getBytes(), data.getBytes());
        
        String result = Util.bytesToHex(hmac);
        assertEquals("5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843", result);
        
    }

}
