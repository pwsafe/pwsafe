package org.pwsafe.lib.crypto;

import javax.crypto.Cipher;

import junit.framework.TestCase;

/**
 * Test Twofish operation.
 * 
 * @author Glen Smith
 */
public class TwofishPwsTest extends TestCase {

    int[] key = {0x9F, 0x58, 0x9F, 0x5C, 0xF6, 0x12, 0x2C, 0x32,
            0xB6, 0xBF, 0xEC, 0x2F, 0x2A, 0xE8, 0xC3, 0x5A};
    int[] plainText = { 0xD4, 0x91, 0xDB, 0x16, 0xE7, 0xB1, 0xC3, 0x9E,
              0x86, 0xCB, 0x08, 0x6B, 0x78, 0x9F, 0x54, 0x19 };
    int[] cipherText =  { 0x01, 0x9F, 0x98, 0x09, 0xDE, 0x17, 0x11, 0x85,
              0x8F, 0xAA, 0xC3, 0xA3, 0xBA, 0x20, 0xFB, 0xC3 };
    
    // http://www.schneier.com/code/ecb_ival.txt
    int[] cp_key = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x0, 0x0, 0x0, 0x0 };

	int[] cp_pt = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x0, 0x0, 0x0 };

	int[] cp_ct = { 0x9F, 0x58, 0x9F, 0x5C, 0xF6, 0x12, 0x2C, 0x32, 0xB6, 0xBF,
			0xEC, 0x2F, 0x2A, 0xE8, 0xC3, 0x5A };

    
	
	public byte[] unsignedToSigned(int []ints) {
		byte[] result = new byte[ints.length];
		for (int i=0; i<ints.length; i++) {
			result[i] = (byte) (ints[i] & 0xFF);
		}
		return result;
	}
	/**
	 * Tests we can encrypted and decrypt and get back to square.
	 * 
	 * @throws Exception if bad things happen
	 */
	public void testRoundTrip() throws Exception {
		
        Cipher enc = TwofishPws.getCipher(unsignedToSigned(key), true, true); 
        byte[] encResult = enc.doFinal(unsignedToSigned(plainText));

        Cipher decr = TwofishPws.getCipher(unsignedToSigned(key), false, true); 
        byte[] decResult = decr.doFinal(encResult);

        assertEquals(HmacPwsTest.byteArrayToHex(unsignedToSigned(plainText)), HmacPwsTest.byteArrayToHex(decResult));
	}

	/**
	 * Tests the Counterpane vectors work here too...
	 * 
	 * @throws Exception if bad things happen
	 */
    public void testVectorCounterPane() throws Exception {
        
        Cipher enc = TwofishPws.getCipher(unsignedToSigned(cp_key), true, true); 
        byte[] encResult = enc.doFinal(unsignedToSigned(cp_pt));

        String encStr = HmacPwsTest.byteArrayToHex(encResult);
        String expected = HmacPwsTest.byteArrayToHex(unsignedToSigned(cp_ct));
        
        assertEquals(expected, encStr);
        
    }
	
	
	
	/**
	 * Tests the pwsafe vectors work here too...
	 * 
	 * @throws Exception if bad things happen
	 */
    public void testVectorCVersion() throws Exception {
        
        Cipher enc = TwofishPws.getCipher(unsignedToSigned(key), true, true); 
        byte[] encResult = enc.doFinal(unsignedToSigned(plainText));

        String encStr = HmacPwsTest.byteArrayToHex(encResult);
        String expected = HmacPwsTest.byteArrayToHex(unsignedToSigned(cipherText));
        
        assertEquals(expected, encStr);
        
    }

}
