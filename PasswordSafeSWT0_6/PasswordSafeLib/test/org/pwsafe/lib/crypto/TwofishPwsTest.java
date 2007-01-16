package org.pwsafe.lib.crypto;

import org.pwsafe.lib.Util;

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
    
    // 256bit key examples from pwsafe
    int[] key32 = { 0xD4, 0x3B, 0xB7, 0x55, 0x6E, 0xA3, 0x2E, 0x46,
            0xF2, 0xA2, 0x82, 0xB7, 0xD4, 0x5B, 0x4E, 0x0D,
            0x57, 0xFF, 0x73, 0x9D, 0x4D, 0xC9, 0x2C, 0x1B,
            0xD7, 0xFC, 0x01, 0x70, 0x0C, 0xC8, 0x21, 0x6F };
    int[] plainText32 = { 0x90, 0xAF, 0xE9, 0x1B, 0xB2, 0x88, 0x54, 0x4F,
            0x2C, 0x32, 0xDC, 0x23, 0x9B, 0x26, 0x35, 0xE6 };
    int[] cipherText32 = { 0x6C, 0xB4, 0x56, 0x1C, 0x40, 0xBF, 0x0A, 0x97,
            0x05, 0x93, 0x1C, 0xB6, 0xD4, 0x08, 0xE7, 0xFA };
    
    // http://www.schneier.com/code/ecb_ival.txt
    int[] cp_key = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x0, 0x0, 0x0, 0x0 };

	int[] cp_pt = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x0, 0x0, 0x0 };

	int[] cp_ct = { 0x9F, 0x58, 0x9F, 0x5C, 0xF6, 0x12, 0x2C, 0x32, 0xB6, 0xBF,
			0xEC, 0x2F, 0x2A, 0xE8, 0xC3, 0x5A };

	
	/**
	 * Tests we can encrypted and decrypt and get back to square.
	 * 
	 * @throws Exception if bad things happen
	 */
	public void testRoundTrip() throws Exception {
		
		byte[] encResult = TwofishPws.processECB(Util.unsignedToSigned(key), true, Util.unsignedToSigned(plainText)); 

        byte[] decResult = TwofishPws.processECB(Util.unsignedToSigned(key), false, encResult);

        assertEquals(Util.bytesToHex(Util.unsignedToSigned(plainText)), Util.bytesToHex(decResult));
	}
	

	/**
	 * Tests we can encrypted and decrypt and get back to square using 32 bit key.
	 * 
	 * @throws Exception if bad things happen
	 */
	public void testRoundTrip32() throws Exception {

		byte[] encResult = TwofishPws.processECB(Util.unsignedToSigned(key32), true, Util.unsignedToSigned(plainText32)); 

        byte[] decResult = TwofishPws.processECB(Util.unsignedToSigned(key32), false, encResult);

        assertEquals(Util.bytesToHex(Util.unsignedToSigned(plainText32)), Util.bytesToHex(decResult));

	}


	/**
	 * Tests the Counterpane vectors work here too...
	 * 
	 * @throws Exception if bad things happen
	 */
    public void testVectorCounterPane() throws Exception {

		byte[] encResult = TwofishPws.processECB(Util.unsignedToSigned(cp_key), true, Util.unsignedToSigned(cp_pt)); 

        byte[] decResult = TwofishPws.processECB(Util.unsignedToSigned(cp_key), false, encResult);

        assertEquals(Util.bytesToHex(Util.unsignedToSigned(cp_pt)), Util.bytesToHex(decResult));
        
    }
	
	
	
	/**
	 * Tests the pwsafe vectors work here too...
	 * 
	 * @throws Exception if bad things happen
	 */
    public void testVectorCVersion() throws Exception {
        
    	byte[] encResult = TwofishPws.processECB(Util.unsignedToSigned(key), true, Util.unsignedToSigned(plainText)); 

        String encStr = Util.bytesToHex(encResult);
        String expected = Util.bytesToHex(Util.unsignedToSigned(cipherText));
        
        assertEquals(expected, encStr);
        
    }
    
    

}
