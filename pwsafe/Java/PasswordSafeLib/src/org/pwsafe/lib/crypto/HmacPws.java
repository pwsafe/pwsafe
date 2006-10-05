package org.pwsafe.lib.crypto;

import org.bouncycastle.crypto.digests.SHA256Digest;
import org.bouncycastle.crypto.macs.HMac;
import org.bouncycastle.crypto.params.KeyParameter;

/**
 * HMAC implementation. Currently uses BouncyCastle provider underneath.
 * 
 * @author Glen Smith
 */
public class HmacPws {

	private HMac mac;

	public HmacPws(byte[] key) {
		
		mac = new HMac(new SHA256Digest());
		KeyParameter kp = new KeyParameter(key);
		mac.init(kp);
		
	}

	public void digest(byte[] incoming) {
        mac.update(incoming, 0, incoming.length);
    }

	public byte[] doFinal() {
		byte[] output = new byte[mac.getUnderlyingDigest().getDigestSize()];
		mac.doFinal(output, 0);
		return output;
	}

}
