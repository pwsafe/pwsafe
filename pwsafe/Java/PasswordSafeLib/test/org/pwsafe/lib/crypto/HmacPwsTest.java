package org.pwsafe.lib.crypto;

import junit.framework.TestCase;

/**
 * Test HMAC operation.
 * 
 * @author Glen Smith
 */
public class HmacPwsTest extends TestCase {

	public static String byteArrayToHex(byte[] bs) {
        StringBuffer ret = new StringBuffer(bs.length);
        for (int i = 0; i < bs.length; i++) {
            String hex = Integer.toHexString(0x0100 + (bs[i] & 0x00FF)).substring(1);
            ret.append((hex.length() < 2 ? "0" : "") + hex);
        }
        return ret.toString();
    }

	
    public void testDigest() {
        
        String key = "Jefe";
        String data = "what do ya want for nothing?";
        byte[] hmac = HmacPws.digest(key.getBytes(), data.getBytes());
        
        String result = byteArrayToHex(hmac);
        assertEquals("5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843", result);
        
    }

}
