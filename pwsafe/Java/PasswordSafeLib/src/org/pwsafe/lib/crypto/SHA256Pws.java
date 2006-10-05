package org.pwsafe.lib.crypto;

import org.bouncycastle.crypto.digests.SHA256Digest;

/**
 * SHA256 implementation. Currently uses BouncyCastle provider underneath.
 * 
 * @author Glen Smith
 */
public class SHA256Pws {


    public static byte[] digest(byte[] incoming) {
    	
    	SHA256Digest digest = new SHA256Digest();
    	byte[] output = new byte[digest.getDigestSize()];
    	
    	digest.update(incoming, 0, incoming.length);
    	digest.doFinal(output, 0);
    	
    	return output;
        
    }

}
