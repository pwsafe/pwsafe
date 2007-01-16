package org.pwsafe.lib.crypto;

import org.bouncycastle.crypto.BufferedBlockCipher;
import org.bouncycastle.crypto.CryptoException;
import org.bouncycastle.crypto.engines.TwofishEngine;
import org.bouncycastle.crypto.modes.CBCBlockCipher;
import org.bouncycastle.crypto.params.KeyParameter;
import org.bouncycastle.crypto.params.ParametersWithIV;

/**
 * Twofish implementation wrapper. Current implementation uses
 * BouncyCastle provider.
 * 
 * @author Glen Smith
 */
public class TwofishPws {
	
	CBCBlockCipher cipher;
    
	public TwofishPws(byte[] key, boolean forEncryption, byte[] IV) {
		
		TwofishEngine tfe = new TwofishEngine();
    	cipher = new CBCBlockCipher(tfe);
    	KeyParameter kp = new KeyParameter(key);
    	ParametersWithIV piv = new ParametersWithIV(kp, IV);
    	cipher.init(forEncryption, piv);
		
	}
	
	public byte[] processCBC(byte[] input) {
		
		
    	byte[]  out = new byte[input.length];

        int len1 = cipher.processBlock(input, 0, out, 0);
        
        return out;
		
	}
	
    public static byte[] processECB(byte[] key, boolean forEncryption, byte[] input) {

    	BufferedBlockCipher cipher = new BufferedBlockCipher(new TwofishEngine());
    	KeyParameter kp = new KeyParameter(key);
    	cipher.init(forEncryption, kp);
    	byte[]  out = new byte[input.length];

        int len1 = cipher.processBytes(input, 0, input.length, out, 0);
        
        try
        {
            int len2 = cipher.doFinal(out, len1);
        }
        catch (CryptoException e)
        {
            throw new RuntimeException(e);
        }
        return out;
    }
    

}
