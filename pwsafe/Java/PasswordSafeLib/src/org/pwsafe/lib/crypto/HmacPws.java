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

	private Mac mac;

	public HmacPws(byte[] key) {
		KeySpec ks = new SecretKeySpec(key, "HMACSHA256");
		try {
			mac = Mac.getInstance("HMACSHA256", provider);
			mac.init((Key) ks);
		} catch (Exception e) {
			throw new RuntimeException(e);
		}
	}

	public void digest(byte[] incoming) {
        try {         
            mac.update(incoming);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

	public byte[] doFinal() {
		try {
			return mac.doFinal();
		} catch (Exception e) {
			throw new RuntimeException(e);
		}
	}

}
