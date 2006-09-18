package org.pwsafe.lib.crypto;

import java.security.Key;
import java.security.Provider;
import java.security.spec.KeySpec;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

/**
 * HMAC implementation. Currently uses BouncyCastle provider underneath.
 * 
 * @author Glen Smith
 */
public class HmacPws {

    private static Provider provider = new BouncyCastleProvider();

    public static byte[] digest(byte[] key, byte[] incoming) {
        try {
            KeySpec ks = new SecretKeySpec(key, "HMACSHA256");
            Mac mac = Mac.getInstance("HMACSHA256", provider);
            mac.init((Key) ks);
            return mac.doFinal(incoming);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

}
