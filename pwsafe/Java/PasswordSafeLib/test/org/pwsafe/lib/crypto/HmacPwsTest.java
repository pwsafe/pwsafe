package org.pwsafe.lib.crypto;

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
        StringBuffer sb = new StringBuffer("0x");
        for (int i = 0; i < hmac.length; i++) {
            sb.append(Integer.toHexString(hmac[i]));
        }
        String result = "0x750c783e6ab0b503eaa86e310a5db738";
        assertEquals(result, sb.toString());
        
    }

}
