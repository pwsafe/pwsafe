package org.pwsafe.lib.crypto;

import java.security.MessageDigest;
import java.security.Provider;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

/**
 * SHA256 implementation. Currently uses BouncyCastle provider underneath.
 * 
 * @author Glen Smith
 */
public class SHA256Pws {

    private static Provider provider = new BouncyCastleProvider();
    private static MessageDigest md;

    public static byte[] digest(byte[] incoming) {
        try {
            if (md == null) {
            	 md = MessageDigest.getInstance("SHA256", provider);
            }
            return md.digest(incoming);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

}
