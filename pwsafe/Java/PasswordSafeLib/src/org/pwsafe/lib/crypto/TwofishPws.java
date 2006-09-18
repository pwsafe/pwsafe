package org.pwsafe.lib.crypto;

import java.security.Key;
import java.security.Provider;
import java.security.spec.KeySpec;

import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

/**
 * Twofish implementation wrapper. Current implementation uses
 * BouncyCastle provider.
 * 
 * @author Glen Smith
 */
public class TwofishPws {
    
    private static Provider provider = new BouncyCastleProvider();
    
    public static Cipher getCipher(byte[] key) {

        KeySpec ks = new SecretKeySpec(key, "BLOWFISH");
        try {
            Cipher c = Cipher.getInstance("BLOWFISH",provider);
            c.init(Cipher.ENCRYPT_MODE, (Key) ks);
            return c;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }

    }

}
